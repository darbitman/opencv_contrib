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

#include "opencv2/seamcarver/seamcarver.hpp"
#include "opencv2/seamcarver/gradientpixelenergy2d.hpp"
using std::vector;

cv::SeamCarver::SeamCarver(double marginEnergy, PixelEnergy2D* pPixelEnergy2D) :
    marginEnergy_(marginEnergy)
{
    if (pPixelEnergy2D)
    {
        pixelEnergyCalculator_ = pPixelEnergy2D;
    }
    else
    {
        pixelEnergyCalculator_ = new GradientPixelEnergy2D(marginEnergy);
    }
}

cv::SeamCarver::~SeamCarver()
{
    if (pixelEnergyCalculator_)
    {
        delete pixelEnergyCalculator_;
        pixelEnergyCalculator_ = nullptr;
    }
}

size_t cv::SeamCarver::getNumberOfColumns() const
{
    return numColumns_;
}

size_t cv::SeamCarver::getNumberOfRows() const
{
    return numRows_;
}

void cv::SeamCarver::setDimensions(size_t numRows, size_t numColumns)
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

void cv::SeamCarver::setDimensions(const cv::Mat& img)
{
    if (img.empty())
    {
        CV_Error(Error::Code::StsBadArg, "setDimensions failed due to empty image");
    }

    try
    {
        setDimensions((size_t)img.rows, (size_t)img.cols);
    }
    catch (...)
    {
        throw;
    }
}

void cv::SeamCarver::setPixelEnergyCalculator(PixelEnergy2D* pNewPixelEnergyCalculator)
{
    if (pNewPixelEnergyCalculator == nullptr)
    {
        CV_Error(Error::Code::StsBadArg, "setNewPixelEnergyCalculator failed due to nullptr arg");
    }
    if (pixelEnergyCalculator_)
    {
        delete pixelEnergyCalculator_;
    }
    pixelEnergyCalculator_ = pNewPixelEnergyCalculator;
}

inline bool cv::SeamCarver::areDimensionsInitialized() const
{
    return !needToInitializeLocalData;
}

void cv::SeamCarver::init(const cv::Mat& img, size_t seamLength)
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

void cv::SeamCarver::init(size_t numRows,
                          size_t numColumns,
                          size_t seamLength)
{
    initializeLocalVariables(numRows, numColumns, numRows - 1, numColumns - 1, seamLength);
    initializeLocalVectors();

    needToInitializeLocalData = false;
}

inline void cv::SeamCarver::initializeLocalVariables(size_t numRows,
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

void cv::SeamCarver::initializeLocalVectors()
{
    pixelEnergy.resize(numRows_);
    markedPixels.resize(numRows_);
    totalEnergyTo.resize(numRows_);
    previousLocationTo.resize(numRows_);

    for (size_t row = 0; row < numRows_; row++)
    {
        pixelEnergy[row].resize(numColumns_);
        markedPixels[row].resize(numColumns_);
        totalEnergyTo[row].resize(numColumns_);
        previousLocationTo[row].resize(numColumns_);
    }

    currentSeam.resize(seamLength_);
    discoveredSeams.resize(seamLength_);
}

void cv::SeamCarver::resetLocalVectors(size_t numSeams)
{
    for (size_t row = 0; row < numRows_; row++)
    {
        // set marked pixels to false for new run
        for (size_t column = 0; column < numColumns_; column++)
        {
            markedPixels[row][column] = false;
        }
    }

    for (size_t seamNum = 0; seamNum < seamLength_; seamNum++)
    {
        // ensure priority queue has at least numSeams capacity
        if (numSeams > discoveredSeams[seamNum].capacity())
        {
            discoveredSeams[seamNum].allocate(numSeams);
        }

        // reset priority queue since it could be filled from a previous run
        if (discoveredSeams[seamNum].size() > 0)
        {
            discoveredSeams[seamNum].resetHeap();
        }
    }
}