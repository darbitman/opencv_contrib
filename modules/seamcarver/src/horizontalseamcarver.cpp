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

#include "opencv2/seamcarver/horizontalseamcarver.hpp"
#include "opencv2/seamcarver/gradientpixelenergy2d.hpp"

cv::HorizontalSeamCarver::HorizontalSeamCarver(
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

cv::HorizontalSeamCarver::HorizontalSeamCarver(
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

cv::HorizontalSeamCarver::HorizontalSeamCarver(
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

cv::HorizontalSeamCarver::~HorizontalSeamCarver() {}

void cv::HorizontalSeamCarver::runSeamRemover(size_t numSeamsToRemove,
                                            const cv::Mat& image,
                                            cv::Mat& outImage)
{
    try
    {
        if (bNeedToInitializeLocalData)
        {
            init(image, (size_t)image.cols);
        }

        // check if removing more seams than columns available
        if (numSeamsToRemove > numRows_)
        {
            CV_Error(Error::Code::StsBadArg, "Removing more seams than rows available");
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

void cv::HorizontalSeamCarver::setDimensions(size_t numRows, size_t numColumns)
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

void cv::HorizontalSeamCarver::setDimensions(const cv::Mat& image)
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

inline bool cv::HorizontalSeamCarver::areDimensionsInitialized() const
{
    return !bNeedToInitializeLocalData;
}

void cv::HorizontalSeamCarver::setPixelEnergyCalculator(
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator)
{
    if (pNewPixelEnergyCalculator == nullptr)
    {
        CV_Error(Error::Code::StsBadArg, "setNewPixelEnergyCalculator failed due to nullptr arg");
    }

    pPixelEnergyCalculator_ = pNewPixelEnergyCalculator;
}

void cv::HorizontalSeamCarver::init(const cv::Mat& img, size_t seamLength)
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

void cv::HorizontalSeamCarver::init(size_t numRows, size_t numColumns, size_t seamLength)
{
    initializeLocalVariables(numRows, numColumns, numRows - 1, numColumns - 1, seamLength);
    initializeLocalVectors();
    bNeedToInitializeLocalData = false;
}

inline void cv::HorizontalSeamCarver::initializeLocalVariables(size_t numRows,
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

void cv::HorizontalSeamCarver::initializeLocalVectors()
{
    pixelEnergy.resize(numRows_);
    markedPixels.resize(numRows_);
    totalEnergyTo.resize(numRows_);
    rowTo.resize(numRows_);

    for (size_t row = 0; row < numRows_; row++)
    {
        pixelEnergy[row].resize(numColumns_);
        markedPixels[row].resize(numColumns_);
        totalEnergyTo[row].resize(numColumns_);
        rowTo[row].resize(numColumns_);
    }

    currentSeam.resize(seamLength_);
    discoveredSeams.resize(seamLength_);
}

void cv::HorizontalSeamCarver::resetLocalVectors()
{
    for (size_t row = 0; row < numRows_; row++)
    {
        // set marked pixels to false for new run
        for (size_t column = 0; column < numColumns_; column++)
        {
            markedPixels[row][column] = false;
        }
    }

    for (size_t column = 0; column < seamLength_; column++)
    {
        // ensure each column's PQ has enough capacity
        if (numSeamsToRemove_ > discoveredSeams[column].capacity())
        {
            discoveredSeams[column].changeCapacity(numSeamsToRemove_);
        }

        // reset priority queue since it could be filled from a previous run
        if (!discoveredSeams[column].empty())
        {
            discoveredSeams[column].resetPriorityQueue();
        }
    }
}

void cv::HorizontalSeamCarver::findAndRemoveSeams(const cv::Mat& image, cv::Mat& outImage)
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

void cv::HorizontalSeamCarver::findSeams()
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
    //      cumulative energy row in the right column
    double minTotalEnergy = posInf_;
    int32_t minTotalEnergyRow = -1;
    bool bRestartSeamDiscovery = false;   // current seam discovery iteration needs to be restarted

    // declare variables to keep track of columns when traversing up the seam
    size_t prevRow = 0;
    size_t currentRow = 0;

    /*** RUN SEAM DISCOVERY ***/
    for (int32_t n = 0; n < (int32_t)numSeamsToRemove_; n++)
    {
        // initialize total energy to +INF and run linear search for a pixel of least cumulative
        //      energy (if one exists) in the right column
        minTotalEnergy = posInf_;
        minTotalEnergyRow = -1;
        for (size_t row = 0; row < numColumns_; row++)
        {
            if (!markedPixels[row][rightColumn_] &&
                totalEnergyTo[row][rightColumn_] < minTotalEnergy)
            {
                minTotalEnergy = totalEnergyTo[row][rightColumn_];
                minTotalEnergyRow = row;
            }
        }

        // all pixels in right column are unreachable due to +INF cumulative energy to all of them
        // therefore need to recalculate cumulative energies
        if (minTotalEnergyRow == -1)
        {
            // decrement iterator since this seam will need to be restarted after recalculating
            // the cumulative energy
            n--;
            calculateCumulativePathEnergy();

            // skip over the seam traversal algorithm below and restart algorithm to rediscover
            // the seam for this iteration
            continue;
        }

        // save last row as part of currentSeam that will be checked whether it can fully reach
        //      the top row
        currentSeam[rightColumn_] = minTotalEnergyRow;

        // initialize column variables
        prevRow = minTotalEnergyRow;
        currentRow = prevRow;

        // run seam traversal starting at right column to find all the pixels in the seam
        for (int32_t column = rightColumn_ - 1; column >= 0; column--)
        {
            // using the right pixel's row and column, extract the row of the pixel in the
            //      current column
            currentRow = rowTo[prevRow][(size_t)column + 1];

            // check if the current pixel of the current seam has been used part of another seam
            if (markedPixels[currentRow][(size_t)column])
            {
                // mark the starting pixel in bottom row as having +INF cumulative energy so it
                //      will not be chosen again
                totalEnergyTo[currentRow][(size_t)column] = posInf_;

                // decrement seam iterator since this seam is invalid and this iteration will
                // need to be restarted
                n--;

                // set to indicate that the outer for loop will need to be restarted for this
                // seam iteration
                bRestartSeamDiscovery = true;
                break;
            }

            // save the column of the pixel in the current row
            currentSeam[(size_t)column] = currentRow;

            // save current column to be used for the next iteration of the loop
            prevRow = currentRow;
        }

        if (bRestartSeamDiscovery)
        {
            bRestartSeamDiscovery = false;
            continue;
        }
        else
        {
            // copy current seam into the discovered seams and mark appropriate pixels
            for (size_t column = 0; column < numColumns_; column++)
            {
                currentRow = currentSeam[column];
                discoveredSeams[column].push(currentRow);
                markedPixels[currentRow][column] = true;
            }
        }
    }   // for (int32_t n = 0; n < (int32_t)numSeamsToRemove_; n++)
}

void cv::HorizontalSeamCarver::calculateCumulativePathEnergy()
{
    // initialize left column
    for (size_t row = 0; row < numRows_; row++)
    {
        // if previously markedPixels, set its energy to +INF
        if (markedPixels[row][0])
        {
            totalEnergyTo[row][0] = posInf_;
        }
        else
        {
            totalEnergyTo[row][0] = marginEnergy_;
        }
        rowTo[row][0] = -1;
    }

    // cache the total energy to the pixels up/left, directly left, and down/left
    //   instead of accessing memory for the same pixels
    // shift energy values to the left and access memory only once
    // VERTICAL SHIFT OPERATION:
    //   left/above <== directly left
    //   directly left <== left/below
    //   left/below = access new memory
    double energyUpLeft = posInf_;
    double energyLeft = posInf_;
    double energyDownLeft = posInf_;

    bool markedUpLeft = false;
    bool markedLeft = false;
    bool markedDownLeft = false;

    double minEnergy = posInf_;
    int32_t minEnergyRow = -1;

    for (size_t column = 1; column < numColumns_; column++)
    {
        energyUpLeft = posInf_;
        energyLeft = totalEnergyTo[0][column - 1];
        energyDownLeft = numRows_ > 1 ? totalEnergyTo[1][column - 1] : posInf_;

        markedUpLeft = true;
        markedLeft = markedPixels[0][column - 1];
        markedDownLeft = numRows_ > 1 ? markedPixels[1][column - 1] : true;

        // find minimum energy path from previous row to every pixel in the current row
        for (size_t row = 0; row < numRows_; row++)
        {
            // initialize min energy to +INF and initialize the previous column to -1
            //   to set error state
            minEnergy = posInf_;
            minEnergyRow = -1;

            // save some cycles by not doing any comparisons if the current pixel has been
            //      previously markedPixels
            if (!markedPixels[row][column])
            {
                // check left
                if (!markedLeft && energyLeft < minEnergy)
                {
                    minEnergy = energyLeft;
                    minEnergyRow = row;
                }

                // check if left/down is min
                if (row < numRows_ - 1)
                {
                    if (!markedDownLeft && energyDownLeft < minEnergy)
                    {
                        minEnergy = energyDownLeft;
                        minEnergyRow = row + 1;
                    }
                }

                // check if left/up is min
                if (row > 0)
                {
                    if (!markedUpLeft && energyUpLeft < minEnergy)
                    {
                        minEnergy = energyUpLeft;
                        minEnergyRow = row - 1;
                    }
                }
            }

            // shift energy up
            energyUpLeft = energyLeft;
            markedUpLeft = markedLeft;
            energyLeft = energyDownLeft;
            markedLeft = markedDownLeft;

            // get markedPixels and totalEnergyTo data for pixels right/above
            if (numRows_ > 1 && row < numRows_ - 2)
            {
                energyDownLeft = totalEnergyTo[row + 2][column -1];
                markedDownLeft = markedPixels[row + 2][column - 1];
            }

            // assign cumulative energy to current pixel and save the column of the parent pixel
            if (minEnergyRow == -1)
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
            rowTo[row][column] = minEnergyRow;
        }
    }
}

void cv::HorizontalSeamCarver::removeSeams()
{
    // each row of seams stores an ordered queue of pixels to remove in that row
    //   starting with the min number column
    // each time a new column is encountered, move the pixels to the right of it
    //   (up until the next column number) to the left by the number of pixels already removed

    size_t rowToRemove = 0;
    size_t numSeamsRemoved = 0;
    /*** REMOVE PIXELS FOR EVERY ROW ***/
    for (size_t column = 0; column < numColumns_; column++)
    {
        // keep track of the number of seams above the current seam in order to keep track how much
        //   pixels will need to be moved to account for the removed pixels above
        numSeamsRemoved = 0;
        // loop through all pixels to remove in current row
        while (!discoveredSeams[column].empty())
        {
            numSeamsRemoved++;
            // row location of pixel to remove in column
            rowToRemove = discoveredSeams[column].pop();

            // mark right endpoint/next pixel column
            size_t bottomRowBorder = (discoveredSeams[column].empty() ?
                                     numColumns_ : discoveredSeams[column].top());
            // starting at the row below the row of the pixel to be removed,
            //      move the pixel up, by the number of seams removed above the current row,
            //      until the bottom end point which is either the last row or the next row
            //      to remove whichever comes first
            for (size_t row = rowToRemove + 1; row < bottomRowBorder; row++)
            {
                for (size_t j = 0; j < numColorChannels_; j++)
                {
                    bgr[j].at<uchar>(row - numSeamsRemoved, column) = bgr[j].at<uchar>(row, column);
                }
            }
        }
    }

    /*** SHRINK IMAGE SINCE THE IMPORTANT SINCE WERE SHIFTED UP ***/
    for (size_t channel = 0; channel < numColorChannels_; channel++)
    {
        bgr[channel] = bgr[channel].rowRange(0, numRows_ - numSeamsRemoved);
    }
}