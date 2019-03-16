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


cv::VerticalSeamCarver::VerticalSeamCarver(double marginEnergy) : SeamCarver(marginEnergy) {}

cv::VerticalSeamCarver::VerticalSeamCarver(size_t numRows,
                                           size_t numColumns,
                                           size_t numColorChannels,
                                           double marginEnergy) :
    SeamCarver(numRows, numColumns, numColorChannels, marginEnergy)
{
    init(numRows, numColumns, numColorChannels, numRows);
}

cv::VerticalSeamCarver::VerticalSeamCarver(const cv::Mat& img, double marginEnergy) :
    SeamCarver(marginEnergy)
{
    init(img, (size_t)img.rows);
}

void cv::VerticalSeamCarver::runSeamRemover(size_t numSeams,
                                            const cv::Mat& img,
                                            cv::Mat& outImg)
{
    try
    {
        if (needToInitializeLocalData)
        {
            init(img, img.rows);
        }

        // check if removing more seams than columns available
        if (numSeams > numColumns_)
        {
            CV_Error(Error::Code::StsBadArg, "Removing more seams than columns available");
        }

        resetLocalVectors(numSeams);

        findAndRemoveSeams(numSeams, img, outImg);
    }
    catch (...)
    {
        throw;
    }
}

void cv::VerticalSeamCarver::findAndRemoveSeams(const size_t& numSeams,
                                                const cv::Mat& img,
                                                cv::Mat& outImg)
{
    if (pixelEnergyCalculator_->getNumColorChannels() == 3 && img.channels() == 3)
    {
        cv::split(img, bgr);
    }
    else if (pixelEnergyCalculator_->getNumColorChannels() == 1 && img.channels() == 1)
    {
        cv::extractChannel(img, bgr[0], 0);
    }
    else
    {
        CV_Error(Error::Code::StsInternal, "VerticalSeamCarver::findAndRemoveSeams failed due to \
                                            incorrect number of color channels");
    }

    try
    {
        pixelEnergyCalculator_->calculatePixelEnergy(img, pixelEnergy);

        // find all vertical seams
        findSeams(numSeams);

        // remove all found seams, least cumulative energy first
        removeSeams();

        // FIXME merge based on number of color channels
        // combine separate channels into output image
        cv::merge(bgr, outImg);
    }
    catch (const cv::Exception& caughtException)
    {
        throw caughtException;
    }
}


void cv::VerticalSeamCarver::findSeams(size_t numSeams)
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
    int32_t minTotalEnergyCol = -1;
    bool restartSeamDiscovery = false;   // seam discovery needs to be restarted for currentSeam

    /*** RUN SEAM DISCOVERY ***/
    for (size_t n = 0; n < numSeams; n++)
    {
        // initialize total energy to +INF and run linear search for a pixel of least cumulative
        //      energy (if one exists) in the bottom row
        minTotalEnergy = posInf_;
        minTotalEnergyCol = -1;
        for (size_t column = 0; column < numColumns_; column++)
        {
            if (!markedPixels[bottomRow_][column] &&
                totalEnergyTo[bottomRow_][column] < minTotalEnergy)
            {
                minTotalEnergy = totalEnergyTo[bottomRow_][column];
                minTotalEnergyCol = column;
            }
        }

        // all pixels in bottom row are unreachable due to +INF cumulative energy to all of them
        // therefore need to recalculate cumulative energies
        if (minTotalEnergyCol == -1)
        {
            // decrement currentSeam number iterator since this currentSeam was invalid
            // need to recalculate the cumulative energy
            n--;
            calculateCumulativePathEnergy();

            restartSeamDiscovery = true;
        }

        // save last column as part of currentSeam that will be checked whether it can fully reach
        //      the top row
        currentSeam[bottomRow_] = minTotalEnergyCol;

        // initialize variables to keep track of columns
        size_t prevCol = minTotalEnergyCol;
        size_t currentCol = prevCol;

        if (!restartSeamDiscovery)
        {
            for (int32_t row = bottomRow_ - 1; row >= 0; row--)
            {
                // using the below pixel's row and column, extract the column of the pixel in the
                //      current row
                currentCol = previousLocationTo[(size_t)row + 1][prevCol];

                // check if the current pixel of the current seam has been used part of another seam

                if (markedPixels[(size_t)row][currentCol])
                {
                    // mark the starting pixel in bottom row as having +INF cumulative energy so it
                    //      will not be chosen again
                    totalEnergyTo[bottomRow_][minTotalEnergyCol] = posInf_;

                    // FIXME n-- could overflow & cause the outer for loop to fail since n is unsign
                    // decrement currentSeam number iterator since this currentSeam was invalid
                    n--;
                    // restart currentSeam finding loop
                    restartSeamDiscovery = true;
                    break;
                }

                // save the column of the pixel in the current row
                currentSeam[(size_t)row] = currentCol;

                // save current column to be used for the next iteration of the loop
                prevCol = currentCol;
            }
        }

        if (restartSeamDiscovery)
        {
            restartSeamDiscovery = false;
            continue;
        }
        else
        {
            // copy currentSeam and mark appropriate pixels
            for (size_t row = 0; row < numRows_; row++)
            {
                prevCol = currentSeam[row];
                discoveredSeams[row].push(prevCol);
                markedPixels[row][prevCol] = true;
            }
        }
    }   // for (size_t n = 0; n < numSeams; n++)
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
        previousLocationTo[0][column] = -1;
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
    int32_t minEnergyCol = -1;

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
            minEnergyCol = -1;

            // save some cycles by not doing any comparisons if the current pixel has been
            //      previously markedPixels
            if (!markedPixels[row][column])
            {
                // check above
                if (!markedUp && energyUp < minEnergy)
                {
                    minEnergy = energyUp;
                    minEnergyCol = column;
                }

                // check if right/above is min
                if (column < numColumns_ - 1)
                {
                    if (!markedUpRight && energyUpRight < minEnergy)
                    {
                        minEnergy = energyUpRight;
                        minEnergyCol = column + 1;
                    }
                }

                // check if left/above is min
                if (column > 0)
                {
                    if (!markedUpLeft && energyUpLeft < minEnergy)
                    {
                        minEnergy = energyUpLeft;
                        minEnergyCol = column - 1;
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
            if (minEnergyCol == -1)
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
            previousLocationTo[row][column] = minEnergyCol;
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
                for (size_t j = 0; j < pixelEnergyCalculator_->getNumColorChannels(); j++)
                {
                    bgr[j].at<uchar>(row, column - numSeamsRemoved) = bgr[j].at<uchar>(row, column);
                }

            }
        }
    }

    /*** SHRINK IMAGE BY REMOVING SEAMS ***/
    size_t numColorChannels = pixelEnergyCalculator_->getNumColorChannels();
    for (size_t channel = 0; channel < numColorChannels; channel++)
    {
        bgr[channel] = bgr[channel].colRange(0, numColumns_ - numSeamsRemoved);
    }
}
