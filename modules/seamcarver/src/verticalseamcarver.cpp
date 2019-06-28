/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "opencv2/seamcarver/verticalseamcarver.hpp"
#include "opencv2/seamcarver/gradientpixelenergy2d.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::VerticalSeamCarver::VerticalSeamCarver(
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator)
{
    constructorInit(marginEnergy, pNewPixelEnergyCalculator);
}

cv::VerticalSeamCarver::VerticalSeamCarver(
    size_t numRows,
    size_t numColumns,
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator)
{
    constructorInit(marginEnergy, pNewPixelEnergyCalculator);

    init(numRows, numColumns, numRows);
}

cv::VerticalSeamCarver::VerticalSeamCarver(
    const cv::Mat& image,
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator)
{
    constructorInit(marginEnergy, pNewPixelEnergyCalculator);

    init(image, (size_t)image.rows);
}

cv::VerticalSeamCarver::~VerticalSeamCarver()
{
    // stop threads from checking queues
    runThreads = false;

    // wait (block) for threads to stop execution
    for (size_t threadNum = 0; threadNum < threads.size(); ++threadNum)
    {
        if (threads[threadNum].joinable())
        {
            threads[threadNum].join();
        }
    }
    
    // clean up queue memory
    for (size_t qIndex = 0; qIndex < localDataQueues.size(); ++qIndex)
    {
        std::queue<VerticalSeamCarverData*>& currentQ = localDataQueues[qIndex];
        while (!currentQ.empty())
        {
            delete currentQ.front();
            currentQ.pop();
        }
    }
}

void cv::VerticalSeamCarver::runSeamRemover(size_t numSeamsToRemove, const cv::Mat& image)
{
    std::queue<VerticalSeamCarverData*>& currentQ = localDataQueues[(uint32_t)pipelineStage::STAGE_0];
    std::unique_lock<std::mutex>& currentQLock = queueLocks[(uint32_t)pipelineStage::STAGE_0];

    // STAGE_0 queue is the free store queue to get previously used memory
    // Create more memory if none exist in the free store
    if (currentQ.empty())
    {
        currentQLock.lock();
        currentQ.emplace(new VerticalSeamCarverData(defaultMarginEnergy));
        currentQLock.unlock();
    }

    // pointer to the data for an image in the queue
    VerticalSeamCarverData* currentData = currentQ.front();

    while (true)
    {
        // copy image to internal data store
        currentData->savedImage = cv::makePtr<cv::Mat>(image);
        
        // initialize internal data
        if (currentData->bNeedToInitializeLocalData)
        {
            init(image, (size_t)image.rows);
            break;
        }
        else
        {
            // check if image is of the same dimensions as those used for internal data
            if (areImageDimensionsVerified(image))
            {
                break;
            }
            // if image dimensions are different than those of internal data, reinitialize data
            else
            {
                currentData->bNeedToInitializeLocalData = true;
            }
        }
    }
    
    // check if removing more seams than columns available
    if (numSeamsToRemove > currentData->numColumns_)
    {
        // TODO handle error case
    }

    // set number of seams to remove this pass
    currentData->numSeamsToRemove_ = numSeamsToRemove;

    // reset vectors to their clean state
    resetLocalVectors();

    findAndRemoveSeams();
}

cv::Ptr<cv::Mat> cv::VerticalSeamCarver::tryGetNextFrame()
{
    std::queue<VerticalSeamCarverData*>& currentQ = localDataQueues[(uint32_t)pipelineStage::STAGE_6];
    std::queue<VerticalSeamCarverData*>& nextQ = localDataQueues[(uint32_t)pipelineStage::STAGE_1];
    std::unique_lock<std::mutex>& currentQLock = queueLocks[(uint32_t)pipelineStage::STAGE_6];
    std::unique_lock<std::mutex>& nextQLock = queueLocks[(uint32_t)pipelineStage::STAGE_1];
    
    cv::Ptr<cv::Mat> returnFrame(nullptr);

    if (!currentQ.empty())
    {
        if (currentQLock.try_lock())
        {
            if (nextQLock.try_lock())
            {
                returnFrame = currentQ.front()->savedImage;
                currentQ.front()->savedImage = nullptr;
                nextQ.emplace(currentQ.front());  // put back into free store
                currentQ.pop();
                nextQLock.unlock();
            }
            currentQLock.unlock();
        }
    }
    return returnFrame;
}

cv::Ptr<cv::Mat> cv::VerticalSeamCarver::getNextFrame()
{
    std::queue<VerticalSeamCarverData*>& currentQ = localDataQueues[(uint32_t)pipelineStage::STAGE_6];
    std::queue<VerticalSeamCarverData*>& nextQ = localDataQueues[(uint32_t)pipelineStage::STAGE_1];
    std::unique_lock<std::mutex>& currentQLock = queueLocks[(uint32_t)pipelineStage::STAGE_6];
    std::unique_lock<std::mutex>& nextQLock = queueLocks[(uint32_t)pipelineStage::STAGE_1];

    cv::Ptr<cv::Mat> returnFrame(nullptr);

    if (!currentQ.empty())
    {
        currentQLock.lock();
        nextQLock.lock();
        returnFrame = currentQ.front()->savedImage;
        currentQ.front()->savedImage = nullptr;
        nextQ.emplace(currentQ.front());  // put back into free store
        currentQ.pop();
        nextQLock.unlock();
        currentQLock.unlock();
    }
}

bool cv::VerticalSeamCarver::newResultExists() const
{
    return !localDataQueues[(uint32_t)pipelineStage::STAGE_6].empty();
}

void cv::VerticalSeamCarver::init(const cv::Mat& img, size_t seamLength)
{
    try
    {
        init((size_t)img.rows, (size_t)img.cols, seamLength);
    }
    catch (...)
    {
        throw;
    }
}

void cv::VerticalSeamCarver::init(size_t numRows, size_t numColumns, size_t seamLength)
{
    VerticalSeamCarverData* data = localDataQueues[(uint32_t)pipelineStage::STAGE_0].front();

    // initialize dimension variables
    data->numRows_ = numRows;
    data->numColumns_ = numColumns;
    data->bottomRow_ = numRows - 1;
    data->rightColumn_ = numColumns - 1;
    data->seamLength_ = seamLength;


    // initialize vectors
    data->pixelEnergy.resize(numRows);
    data->markedPixels.resize(numRows);
    data->totalEnergyTo.resize(numRows);
    data->columnTo.resize(numRows);

    for (size_t row = 0; row < numRows; row++)
    {
        data->pixelEnergy[row].resize(numColumns);
        data->markedPixels[row].resize(numColumns);
        data->totalEnergyTo[row].resize(numColumns);
        data->columnTo[row].resize(numColumns);
    }

    data->currentSeam.resize(seamLength);
    data->discoveredSeams.resize(seamLength);

    // data and vectors just set, so no need to do it again
    data->bNeedToInitializeLocalData = false;
}

void cv::VerticalSeamCarver::resetLocalVectors()
{
    VerticalSeamCarverData* data = localDataQueues[(uint32_t)pipelineStage::STAGE_0].front();

    for (size_t row = 0; row < data->numRows_; row++)
    {
        // set marked pixels to false for new run
        for (size_t column = 0; column < data->numColumns_; column++)
        {
            data->markedPixels[row][column] = false;
        }
    }

    for (size_t row = 0; row < data->seamLength_; row++)
    {
        // ensure each row's PQ has enough capacity
        if (data->numSeamsToRemove_ > data->discoveredSeams[row].capacity())
        {
            data->discoveredSeams[row].changeCapacity(data->numSeamsToRemove_);
        }

        // reset priority queue since it could be filled from a previous run
        if (!data->discoveredSeams[row].empty())
        {
            data->discoveredSeams[row].resetPriorityQueue();
        }
    }
}

void cv::VerticalSeamCarver::findAndRemoveSeams()
{
    std::queue<VerticalSeamCarverData*>& currentQ = localDataQueues[(uint32_t)pipelineStage::STAGE_0];
    std::queue<VerticalSeamCarverData*>& nextQ = localDataQueues[(uint32_t)pipelineStage::STAGE_1];
    std::unique_lock<std::mutex>& currentQLock = queueLocks[(uint32_t)pipelineStage::STAGE_0];
    std::unique_lock<std::mutex>& nextQLock = queueLocks[(uint32_t)pipelineStage::STAGE_1];

    // pointer to the data for an image in the queue
    VerticalSeamCarverData* data = currentQ.front();

    cv::Mat& image = *(data->savedImage);

    data->numColorChannels_ = (size_t)image.channels();

    if (data->numColorChannels_ == 3)
    {
        if (data->bgr.size() != 3)
        {
            data->bgr.resize(3);
        }

        cv::split(image, data->bgr);
    }
    else if (data->numColorChannels_ == 1)
    {
        if (data->bgr.size() != 1)
        {
            data->bgr.resize(1);
        }

        cv::extractChannel(image, data->bgr[0], 0);
    }
    else
    {
        // TODO handle error case
    }

    // move data to next queue
    currentQLock.lock();
    nextQLock.lock();
    nextQ.emplace(currentQ.front());
    currentQ.pop();
    nextQLock.unlock();
    currentQLock.unlock();
}

void cv::VerticalSeamCarver::calculatePixelEnergy()
{
    std::queue<VerticalSeamCarverData*>& currentQ = localDataQueues[(uint32_t)pipelineStage::STAGE_1];
    std::queue<VerticalSeamCarverData*>& nextQ = localDataQueues[(uint32_t)pipelineStage::STAGE_2];
    std::unique_lock<std::mutex>& currentQLock = queueLocks[(uint32_t)pipelineStage::STAGE_1];
    std::unique_lock<std::mutex>& nextQLock = queueLocks[(uint32_t)pipelineStage::STAGE_2];

    while (runThreads)
    {
        if (!currentQ.empty())
        {
            // calculate the pixel energy for the image at the front of the current queue
            currentQ.front()->pPixelEnergyCalculator_->calculatePixelEnergy(*currentQ.front()->savedImage, currentQ.front()->pixelEnergy);

            // move data to next queue
            currentQLock.lock();
            nextQLock.lock();
            nextQ.emplace(currentQ.front());
            currentQ.pop();
            nextQLock.unlock();
            currentQLock.unlock();
        }
    }
}

void cv::VerticalSeamCarver::runCumulativePathEnergyCalculation(VerticalSeamCarverData* data)
{
    // initialize top row
    for (size_t column = 0; column < data->numColumns_; column++)
    {
        // if previously markedPixels, set its energy to +INF
        if (data->markedPixels[0][column])
        {
            data->totalEnergyTo[0][column] = data->posInf_;
        }
        else
        {
            data->totalEnergyTo[0][column] = data->marginEnergy_;
        }
        data->columnTo[0][column] = -1;
    }

    // cache the total energy to the pixels up/left, directly above, and up/right
    //   instead of accessing memory for the same pixels
    // shift energy values to the left and access memory only once
    // SHIFT OPERATION:
    //   left/above <== directly above
    //   directly above <== right/above
    //   right/above = access new memory
    double energyUpLeft = data->posInf_;
    double energyUp = data->posInf_;
    double energyUpRight = data->posInf_;

    bool markedUpLeft = false;
    bool markedUp = false;
    bool markedUpRight = false;

    double minEnergy = data->posInf_;
    int32_t minEnergyColumn = -1;

    for (size_t row = 1; row < data->numRows_; row++)
    {
        energyUpLeft = data->posInf_;
        energyUp = data->totalEnergyTo[row - 1][0];
        energyUpRight = data->numColumns_ > 1 ? data->totalEnergyTo[row - 1][1] : data->posInf_;

        markedUpLeft = true;
        markedUp = data->markedPixels[row - 1][0];
        markedUpRight = data->numColumns_ > 1 ? data->markedPixels[row - 1][1] : true;

        // find minimum energy path from previous row to every pixel in the current row
        for (size_t column = 0; column < data->numColumns_; column++)
        {
            // initialize min energy to +INF and initialize the previous column to -1
            //   to set error state
            minEnergy = data->posInf_;
            minEnergyColumn = -1;

            // save some cycles by not doing any comparisons if the current pixel has been
            //      previously marked
            if (!data->markedPixels[row][column])
            {
                // check above
                if (!markedUp && energyUp < minEnergy)
                {
                    minEnergy = energyUp;
                    minEnergyColumn = column;
                }

                // check if right/above is min
                if (column < data->numColumns_ - 1)
                {
                    if (!markedUpRight && energyUpRight < minEnergy)
                    {
                        minEnergy = energyUpRight;
                        minEnergyColumn = column + 1;
                    }
                }

                // check if left/above is min
                if (column > 0)
                {
                    if (!markedUpLeft && energyUpLeft < minEnergy)
                    {
                        minEnergy = energyUpLeft;
                        minEnergyColumn = column - 1;
                    }
                }
            }

            // shift energy to the left
            energyUpLeft = energyUp;
            markedUpLeft = markedUp;
            energyUp = energyUpRight;
            markedUp = markedUpRight;

            // get markedPixels and totalEnergyTo data for pixels right/above
            if (data->numColumns_ > 1 && column < data->numColumns_ - 2)
            {
                energyUpRight = data->totalEnergyTo[row - 1][column + 2];
                markedUpRight = data->markedPixels[row - 1][column + 2];
            }

            // assign cumulative energy to current pixel and save the column of the parent pixel
            if (minEnergyColumn == -1)
            {
                // current pixel is unreachable from parent pixels since they are all markedPixels
                //   OR current pixel already markedPixels
                // set energy to reach current pixel to +INF
                data->totalEnergyTo[row][column] = data->posInf_;
            }
            else
            {
                data->totalEnergyTo[row][column] = minEnergy + data->pixelEnergy[row][column];
            }
            data->columnTo[row][column] = minEnergyColumn;
        }
    }
}

void cv::VerticalSeamCarver::calculateCumulativePathEnergy()
{
    std::queue<VerticalSeamCarverData*>& currentQ = localDataQueues[(uint32_t)pipelineStage::STAGE_2];
    std::queue<VerticalSeamCarverData*>& nextQ = localDataQueues[(uint32_t)pipelineStage::STAGE_3];
    std::unique_lock<std::mutex>& currentQLock = queueLocks[(uint32_t)pipelineStage::STAGE_2];
    std::unique_lock<std::mutex>& nextQLock = queueLocks[(uint32_t)pipelineStage::STAGE_3];

    // pointer to the data for an image in the queue
    VerticalSeamCarverData* data = nullptr;

    while (runThreads)
    {
        if (!currentQ.empty())
        {
            // save the pointer for faster access
            data = currentQ.front();

            runCumulativePathEnergyCalculation(data);

            // move data to next queue
            currentQLock.lock();
            nextQLock.lock();
            nextQ.emplace(data);
            currentQ.pop();
            nextQLock.unlock();
            currentQLock.unlock();
        }
    }
}

void cv::VerticalSeamCarver::findSeams()
{
    std::queue<VerticalSeamCarverData*>& currentQ = localDataQueues[(uint32_t)pipelineStage::STAGE_3];
    std::queue<VerticalSeamCarverData*>& nextQ = localDataQueues[(uint32_t)pipelineStage::STAGE_4];
    std::unique_lock<std::mutex>& currentQLock = queueLocks[(uint32_t)pipelineStage::STAGE_3];
    std::unique_lock<std::mutex>& nextQLock = queueLocks[(uint32_t)pipelineStage::STAGE_4];

    // pointer to the data for an image in the queue
    VerticalSeamCarverData* data = nullptr;

    while (runThreads)
    {
        if (!currentQ.empty())
        {
            // save the pointer for faster access
            data = currentQ.front();

            if (data->pixelEnergy.size() == 0)
            {
                CV_Error(Error::Code::StsInternal,
                        "SeamCarver::findSeams() failed due to zero-size pixelEnergy vector");
            }

            if (data->discoveredSeams.size() != data->pixelEnergy.size())
            {
                CV_Error(Error::Code::StsInternal,
                        "SeamCarver::findSeams() failed due to different sized vectors");
            }

            // initial cumulative energy path has been calculated in the previous step

            // declare/initialize variables used in currentSeam discovery when looking for the least
            //      cumulative energy column in the bottom row
            double minTotalEnergy = data->posInf_;
            int32_t minTotalEnergyColumn = -1;
            bool bRestartSeamDiscovery = false;   // current seam discovery iteration needs to be restarted

            // declare variables to keep track of columns when traversing up the seam
            size_t prevColumn = 0;
            size_t currentColumn = 0;

            /*** RUN SEAM DISCOVERY ***/
            for (int32_t n = 0; n < (int32_t)data->numSeamsToRemove_; n++)
            {
                // initialize total energy to +INF and run linear search for a pixel of least cumulative
                //      energy (if one exists) in the bottom row
                minTotalEnergy = data->posInf_;
                minTotalEnergyColumn = -1;
                for (size_t column = 0; column < data->numColumns_; column++)
                {
                    if (!data->markedPixels[data->bottomRow_][column] &&
                        data->totalEnergyTo[data->bottomRow_][column] < minTotalEnergy)
                    {
                        minTotalEnergy = data->totalEnergyTo[data->bottomRow_][column];
                        minTotalEnergyColumn = column;
                    }
                }

                // all pixels in bottom row are unreachable due to +INF cumulative energy to all of them
                // therefore need to recalculate cumulative energies
                if (minTotalEnergyColumn == -1)
                {
                    // decrement iterator since this seam will need to be restarted after recalculating
                    // the cumulative energy
                    n--;
                    runCumulativePathEnergyCalculation(data);

                    // skip over the seam traversal algorithm below and restart algorithm to rediscover
                    // the seam for this iteration
                    continue;
                }

                // save last column as part of currentSeam that will be checked whether it can fully reach
                //      the top row
                data->currentSeam[data->bottomRow_] = minTotalEnergyColumn;

                // initialize column variables
                prevColumn = minTotalEnergyColumn;
                currentColumn = prevColumn;

                // run seam traversal starting at bottom row to find all the pixels in the seam
                for (int32_t row = data->bottomRow_ - 1; row >= 0; row--)
                {
                    // using the below pixel's row and column, extract the column of the pixel in the
                    //      current row
                    currentColumn = data->columnTo[(size_t)row + 1][prevColumn];

                    // check if the current pixel of the current seam has been used part of another seam
                    if (data->markedPixels[(size_t)row][currentColumn])
                    {
                        // mark the starting pixel in bottom row as having +INF cumulative energy so it
                        //      will not be chosen again
                        data->totalEnergyTo[data->bottomRow_][minTotalEnergyColumn] = data->posInf_;

                        // decrement seam iterator since this seam is invalid and this iteration will
                        // need to be restarted
                        n--;

                        // set to indicate that the outer for loop will need to be restarted for this
                        // seam iteration
                        bRestartSeamDiscovery = true;
                        break;
                    }

                    // save the column of the pixel in the current row
                    data->currentSeam[(size_t)row] = currentColumn;

                    // save current column to be used for the next iteration of the loop
                    prevColumn = currentColumn;
                }

                if (bRestartSeamDiscovery)
                {
                    bRestartSeamDiscovery = false;
                    continue;
                }
                else
                {
                    // copy current seam into the discovered seams and mark appropriate pixels
                    for (size_t row = 0; row < data->numRows_; row++)
                    {
                        currentColumn = data->currentSeam[row];
                        data->discoveredSeams[row].push(currentColumn);
                        data->markedPixels[row][currentColumn] = true;
                    }
                }
            }   // for (int32_t n = 0; n < (int32_t)numSeamsToRemove_; n++)

            // move data to next queue
            currentQLock.lock();
            nextQLock.lock();
            nextQ.emplace(data);
            currentQ.pop();
            nextQLock.unlock();
            currentQLock.unlock();
            
        }   // if (!currentQ.empty())
    }   // while (runThreads)
}

void cv::VerticalSeamCarver::removeSeams()
{
    std::queue<VerticalSeamCarverData*>& currentQ = localDataQueues[(uint32_t)pipelineStage::STAGE_4];
    std::queue<VerticalSeamCarverData*>& nextQ = localDataQueues[(uint32_t)pipelineStage::STAGE_5];
    std::unique_lock<std::mutex>& currentQLock = queueLocks[(uint32_t)pipelineStage::STAGE_4];
    std::unique_lock<std::mutex>& nextQLock = queueLocks[(uint32_t)pipelineStage::STAGE_5];

    // pointer to the data for an image in the queue
    VerticalSeamCarverData* data = nullptr;

    while (runThreads)
    {
        if (!currentQ.empty())
        {
            // save the pointer for faster access
            data = currentQ.front();

            // each row of seams stores an ordered queue of pixels to remove in that row
            //   starting with the min number column
            // each time a new column is encountered, move the pixels to the right of it
            //   (up until the next column number) to the left by the number of pixels already removed

            size_t colToRemove = 0;
            size_t numSeamsRemoved = 0;
            /*** REMOVE PIXELS FOR EVERY ROW ***/
            for (size_t row = 0; row < data->numRows_; row++)
            {
                // seamRecalculationCount the number of seams to the left of the current pixel
                //   to indicate how many spaces to move pixels that aren't being removed to the left
                numSeamsRemoved = 0;
                // loop through all pixels to remove in current row
                while (!data->discoveredSeams[row].empty())
                {
                    numSeamsRemoved++;
                    // column location of pixel to remove in row
                    colToRemove = data->discoveredSeams[row].pop();

                    // mark right endpoint/next pixel column
                    size_t rightColBorder = (data->discoveredSeams[row].empty() ?
                                            data->numColumns_ : data->discoveredSeams[row].top());
                    // starting at the column to the right of the column to remove,
                    //      move the pixel to the left, by the number of seams to the left of the pixel,
                    //      until the right end point which is either the last column or the next column
                    //      to remove whichever comes first
                    for (size_t column = colToRemove + 1; column < rightColBorder; column++)
                    {
                        for (size_t j = 0; j < data->numColorChannels_; j++)
                        {
                            data->bgr[j].at<uchar>(row, column - numSeamsRemoved) = data->bgr[j].at<uchar>(row, column);
                        }
                    }
                }
            }

            /*** SHRINK IMAGE SINCE THE IMPORTANT SEAMS WERE SHIFTED LEFT ***/
            for (size_t channel = 0; channel < data->numColorChannels_; channel++)
            {
                data->bgr[channel] = data->bgr[channel].colRange(0, data->numColumns_ - numSeamsRemoved);
            }

            // move data to next queue
            currentQLock.lock();
            nextQLock.lock();
            nextQ.emplace(data);
            currentQ.pop();
            nextQLock.unlock();
            currentQLock.unlock();
        }
    }
}

void cv::VerticalSeamCarver::mergeChannels()
{
}

bool cv::VerticalSeamCarver::areImageDimensionsVerified(const cv::Mat& image) const
{
    VerticalSeamCarverData* data = localDataQueues[(uint32_t)pipelineStage::STAGE_0].front();
    if ((size_t)image.rows == data->numRows_ && (size_t)image.cols == data->numColumns_)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void cv::VerticalSeamCarver::constructorInit(double marginEnergy, cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator)
{
    for (size_t mIndex = 0; mIndex < pipelineDepth; ++mIndex)
    {
        queueLocks.emplace_back(mutexes[mIndex], std::defer_lock);
    }

    threads.resize(pipelineDepth);
    localDataQueues.resize(pipelineDepth);

    std::queue<VerticalSeamCarverData*>& currentQ = localDataQueues[(uint32_t)pipelineStage::STAGE_0];

    // create initial storage class
    currentQ.emplace(new VerticalSeamCarverData(marginEnergy));

    if (pNewPixelEnergyCalculator != nullptr)
    {
        currentQ.front()->pPixelEnergyCalculator_ = pNewPixelEnergyCalculator;
    }
    else
    {
        currentQ.front()->pPixelEnergyCalculator_ = cv::makePtr<GradientPixelEnergy2D>(marginEnergy);
    }

    // start pipeline threads
    threads[(uint32_t)pipelineStage::STAGE_1] =
        std::thread(&cv::VerticalSeamCarver::calculatePixelEnergy, this);

    threads[(uint32_t)pipelineStage::STAGE_2] =
        std::thread(&cv::VerticalSeamCarver::calculateCumulativePathEnergy, this);

    threads[(uint32_t)pipelineStage::STAGE_3] =
        std::thread(&cv::VerticalSeamCarver::findSeams, this);
    
    threads[(uint32_t)pipelineStage::STAGE_4] =
        std::thread(&cv::VerticalSeamCarver::removeSeams, this);
}
