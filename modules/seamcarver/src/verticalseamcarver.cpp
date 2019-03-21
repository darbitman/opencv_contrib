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

cv::VerticalSeamCarver::VerticalSeamCarver(
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
    marginEnergy_(marginEnergy)
{
    if (pNewPixelEnergyCalculator != nullptr)
    {
        pPixelEnergyCalculator_ = pNewPixelEnergyCalculator;
    }
    else
    {
        pPixelEnergyCalculator_ = cv::makePtr<GradientPixelEnergy2D>(marginEnergy);
    }
}

cv::VerticalSeamCarver::VerticalSeamCarver(
    size_t numRows,
    size_t numColumns,
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
    marginEnergy_(marginEnergy)
{
    if (pNewPixelEnergyCalculator != nullptr)
    {
        pPixelEnergyCalculator_ = pNewPixelEnergyCalculator;
    }
    else
    {
        pPixelEnergyCalculator_ = cv::makePtr<GradientPixelEnergy2D>(marginEnergy);
    }

    init(numRows, numColumns, numRows);
}

cv::VerticalSeamCarver::VerticalSeamCarver(
    const cv::Mat& img,
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
    marginEnergy_(marginEnergy)
{
    if (pNewPixelEnergyCalculator != nullptr)
    {
        pPixelEnergyCalculator_ = pNewPixelEnergyCalculator;
    }
    else
    {
        pPixelEnergyCalculator_ = cv::makePtr<GradientPixelEnergy2D>(marginEnergy);
    }

    init(img, (size_t)img.rows);
}

cv::VerticalSeamCarver::~VerticalSeamCarver() {}

void cv::VerticalSeamCarver::runSeamRemover(size_t numSeamsToRemove,
                                            const cv::Mat& image,
                                            cv::Mat& outImage)
{
    try
    {
        if (bNeedToInitializeLocalData)
        {
            init(image, (size_t)image.rows);
        }

        // check if removing more seams than columns available
        if (numSeamsToRemove > numColumns_)
        {
            CV_Error(Error::Code::StsBadArg, "Removing more seams than columns available");
        }

        // set number of seams to remove this pass
        numSeamsToRemove_ = numSeamsToRemove;

        // reset vectors to their clean state
        resetLocalVectors();

        findAndRemoveSeams(image, outImage);
    }
    catch (...)
    {
        throw;
    }
}

void cv::VerticalSeamCarver::setDimensions(size_t numRows, size_t numColumns)
{
    if (numRows == 0 || numColumns == 0)
    {
        CV_Error(Error::Code::StsBadArg, "setDimensions failed due bad dimensions");
    }

    try
    {
        init(numRows, numColumns, numRows);
    }
    catch (...)
    {
        throw;
    }
}

void cv::VerticalSeamCarver::setDimensions(const cv::Mat& image)
{
    if (image.empty())
    {
        CV_Error(Error::Code::StsBadArg, "setDimensions failed due to empty image");
    }

    try
    {
        setDimensions((size_t)image.rows, (size_t)image.cols);
    }
    catch (...)
    {
        throw;
    }
}

inline bool cv::VerticalSeamCarver::areDimensionsInitialized() const
{
    return !bNeedToInitializeLocalData;
}

void cv::VerticalSeamCarver::setPixelEnergyCalculator(
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator)
{
    if (pNewPixelEnergyCalculator == nullptr)
    {
        CV_Error(Error::Code::StsBadArg, "setNewPixelEnergyCalculator failed due to nullptr arg");
    }

    pPixelEnergyCalculator_ = pNewPixelEnergyCalculator;
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
    initializeLocalVariables(numRows, numColumns, numRows - 1, numColumns - 1, seamLength);
    initializeLocalVectors();
    bNeedToInitializeLocalData = false;
}

inline void cv::VerticalSeamCarver::initializeLocalVariables(size_t numRows,
                                                             size_t numColumns,
                                                             size_t bottomRow,
                                                             size_t rightColumn,
                                                             size_t seamLength)
{
    numRows_ = numRows;
    numColumns_ = numColumns;
    bottomRow_ = bottomRow;
    rightColumn_ = rightColumn;
    seamLength_ = seamLength;
}

void cv::VerticalSeamCarver::initializeLocalVectors()
{
    pixelEnergy.resize(numRows_);
    markedPixels.resize(numRows_);
    totalEnergyTo.resize(numRows_);
    columnTo.resize(numRows_);

    for (size_t row = 0; row < numRows_; row++)
    {
        pixelEnergy[row].resize(numColumns_);
        markedPixels[row].resize(numColumns_);
        totalEnergyTo[row].resize(numColumns_);
        columnTo[row].resize(numColumns_);
    }

    currentSeam.resize(seamLength_);
    discoveredSeams.resize(seamLength_);
}

void cv::VerticalSeamCarver::resetLocalVectors()
{
    for (size_t row = 0; row < numRows_; row++)
    {
        // set marked pixels to false for new run
        for (size_t column = 0; column < numColumns_; column++)
        {
            markedPixels[row][column] = false;
        }
    }

    for (size_t row = 0; row < seamLength_; row++)
    {
        // ensure each row's PQ has enough capacity
        if (numSeamsToRemove_ > discoveredSeams[row].capacity())
        {
            discoveredSeams[row].changeCapacity(numSeamsToRemove_);
        }

        // reset priority queue since it could be filled from a previous run
        if (!discoveredSeams[row].empty())
        {
            discoveredSeams[row].resetPriorityQueue();
        }
    }
}

void cv::VerticalSeamCarver::findAndRemoveSeams(const cv::Mat& image, cv::Mat& outImage)
{
    numColorChannels_ = (size_t)image.channels();

    if (numColorChannels_ == 3)
    {
        if (bgr.size() != 3)
        {
            bgr.resize(3);
        }

        cv::split(image, bgr);
    }
    else if (numColorChannels_ == 1)
    {
        if (bgr.size() != 1)
        {
            bgr.resize(1);
        }

        cv::extractChannel(image, bgr[0], 0);
    }
    else
    {
        CV_Error(Error::Code::StsBadArg,
                 "findAndRemoveSeams failed due to incorrect number of color channels");
    }

    try
    {
        // find pixel energy for this pass
        pPixelEnergyCalculator_->calculatePixelEnergy(image, pixelEnergy);

        // find all vertical seams
        findSeams();

        // remove all found seams, least cumulative energy first
        removeSeams();

        // combine separate channels into output image
        cv::merge(bgr, outImage);
    }
    catch (const cv::Exception& caughtException)
    {
        throw caughtException;
    }
}

void cv::VerticalSeamCarver::findSeams()
{
    if (pixelEnergy.size() == 0)
    {
        CV_Error(Error::Code::StsInternal,
                 "SeamCarver::findSeams() failed due to zero-size pixelEnergy vector");
    }

    if (discoveredSeams.size() != pixelEnergy.size())
    {
        CV_Error(Error::Code::StsInternal,
                 "SeamCarver::findSeams() failed due to different sized vectors");
    }

    // initial cumulative energy path calculation
    calculateCumulativePathEnergy();

    // declare/initialize variables used in currentSeam discovery when looking for the least
    //      cumulative energy column in the bottom row
    double minTotalEnergy = posInf_;
    int32_t minTotalEnergyColumn = -1;
    bool bRestartSeamDiscovery = false;   // current seam discovery iteration needs to be restarted

    // declare variables to keep track of columns when traversing up the seam
    size_t prevColumn = 0;
    size_t currentColumn = 0;

    /*** RUN SEAM DISCOVERY ***/
    for (int32_t n = 0; n < (int32_t)numSeamsToRemove_; n++)
    {
        // initialize total energy to +INF and run linear search for a pixel of least cumulative
        //      energy (if one exists) in the bottom row
        minTotalEnergy = posInf_;
        minTotalEnergyColumn = -1;
        for (size_t column = 0; column < numColumns_; column++)
        {
            if (!markedPixels[bottomRow_][column] &&
                totalEnergyTo[bottomRow_][column] < minTotalEnergy)
            {
                minTotalEnergy = totalEnergyTo[bottomRow_][column];
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
            calculateCumulativePathEnergy();

            // skip over the seam traversal algorithm below and restart algorithm to rediscover
            // the seam for this iteration
            continue;
        }

        // save last column as part of currentSeam that will be checked whether it can fully reach
        //      the top row
        currentSeam[bottomRow_] = minTotalEnergyColumn;

        // initialize column variables
        prevColumn = minTotalEnergyColumn;
        currentColumn = prevColumn;

        // run seam traversal starting at bottom row to find all the pixels in the seam
        for (int32_t row = bottomRow_ - 1; row >= 0; row--)
        {
            // using the below pixel's row and column, extract the column of the pixel in the
            //      current row
            currentColumn = columnTo[(size_t)row + 1][prevColumn];

            // check if the current pixel of the current seam has been used part of another seam
            if (markedPixels[(size_t)row][currentColumn])
            {
                // mark the starting pixel in bottom row as having +INF cumulative energy so it
                //      will not be chosen again
                totalEnergyTo[bottomRow_][minTotalEnergyColumn] = posInf_;

                // decrement seam iterator since this seam is invalid and this iteration will
                // need to be restarted
                n--;

                // set to indicate that the outer for loop will need to be restarted for this
                // seam iteration
                bRestartSeamDiscovery = true;
                break;
            }

            // save the column of the pixel in the current row
            currentSeam[(size_t)row] = currentColumn;

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
            for (size_t row = 0; row < numRows_; row++)
            {
                currentColumn = currentSeam[row];
                discoveredSeams[row].push(currentColumn);
                markedPixels[row][currentColumn] = true;
            }
        }
    }   // for (int32_t n = 0; n < (int32_t)numSeamsToRemove_; n++)
}

void cv::VerticalSeamCarver::calculateCumulativePathEnergy()
{
    // initialize top row
    for (size_t column = 0; column < numColumns_; column++)
    {
        // if previously markedPixels, set its energy to +INF
        if (markedPixels[0][column])
        {
            totalEnergyTo[0][column] = posInf_;
        }
        else
        {
            totalEnergyTo[0][column] = marginEnergy_;
        }
        columnTo[0][column] = -1;
    }

    // cache the total energy to the pixels up/left, directly above, and up/right
    //   instead of accessing memory for the same pixels
    // shift energy values to the left and access memory only once
    // SHIFT OPERATION:
    //   left/above <== directly above
    //   directly above <== right/above
    //   right/above = access new memory
    double energyUpLeft = posInf_;
    double energyUp = posInf_;
    double energyUpRight = posInf_;

    bool markedUpLeft = false;
    bool markedUp = false;
    bool markedUpRight = false;

    double minEnergy = posInf_;
    int32_t minEnergyColumn = -1;

    for (size_t row = 1; row < numRows_; row++)
    {
        energyUpLeft = posInf_;
        energyUp = totalEnergyTo[row - 1][0];
        energyUpRight = numColumns_ > 1 ? totalEnergyTo[row - 1][1] : posInf_;

        markedUpLeft = true;
        markedUp = markedPixels[row - 1][0];
        markedUpRight = numColumns_ > 1 ? markedPixels[row - 1][1] : true;

        // find minimum energy path from previous row to every pixel in the current row
        for (size_t column = 0; column < numColumns_; column++)
        {
            // initialize min energy to +INF and initialize the previous column to -1
            //   to set error state
            minEnergy = posInf_;
            minEnergyColumn = -1;

            // save some cycles by not doing any comparisons if the current pixel has been
            //      previously markedPixels
            if (!markedPixels[row][column])
            {
                // check above
                if (!markedUp && energyUp < minEnergy)
                {
                    minEnergy = energyUp;
                    minEnergyColumn = column;
                }

                // check if right/above is min
                if (column < numColumns_ - 1)
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
            if (numColumns_ > 1 && column < numColumns_ - 2)
            {
                energyUpRight = totalEnergyTo[row - 1][column + 2];
                markedUpRight = markedPixels[row - 1][column + 2];
            }

            // assign cumulative energy to current pixel and save the column of the parent pixel
            if (minEnergyColumn == -1)
            {
                // current pixel is unreachable from parent pixels since they are all markedPixels
                //   OR current pixel already markedPixels
                // set energy to reach current pixel to +INF
                totalEnergyTo[row][column] = posInf_;
            }
            else
            {
                totalEnergyTo[row][column] = minEnergy + pixelEnergy[row][column];
            }
            columnTo[row][column] = minEnergyColumn;
        }
    }
}

void cv::VerticalSeamCarver::removeSeams()
{
    // each row of seams stores an ordered queue of pixels to remove in that row
    //   starting with the min number column
    // each time a new column is encountered, move the pixels to the right of it
    //   (up until the next column number) to the left by the number of pixels already removed

    size_t colToRemove = 0;
    size_t numSeamsRemoved = 0;
    /*** REMOVE PIXELS FOR EVERY ROW ***/
    for (size_t row = 0; row < numRows_; row++)
    {
        // seamRecalculationCount the number of seams to the left of the current pixel
        //   to indicate how many spaces to move pixels that aren't being removed to the left
        numSeamsRemoved = 0;
        // loop through all pixels to remove in current row
        while (!discoveredSeams[row].empty())
        {
            numSeamsRemoved++;
            // column location of pixel to remove in row row
            colToRemove = discoveredSeams[row].pop();
            //seams[row].pop();
            // mark right endpoint/next pixel column
            size_t rightColBorder = (discoveredSeams[row].empty() ?
                                     numColumns_ : discoveredSeams[row].top());
            // starting at the column to the right of the column to remove,
            //      move the pixel to the left, by the number of seams to the left of the pixel,
            //      until the right end point which is either the last column or the next column
            //      to remove whichever comes first
            for (size_t column = colToRemove + 1; column < rightColBorder; column++)
            {
                for (size_t j = 0; j < numColorChannels_; j++)
                {
                    bgr[j].at<uchar>(row, column - numSeamsRemoved) = bgr[j].at<uchar>(row, column);
                }
            }
        }
    }

    /*** SHRINK IMAGE BY REMOVING SEAMS ***/
    for (size_t channel = 0; channel < numColorChannels_; channel++)
    {
        bgr[channel] = bgr[channel].colRange(0, numColumns_ - numSeamsRemoved);
    }
}