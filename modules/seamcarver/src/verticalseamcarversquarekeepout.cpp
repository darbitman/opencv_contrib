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

#include "opencv2/seamcarver/verticalseamcarversquarekeepout.hpp"

cv::VerticalSeamCarverSquareKeepout::VerticalSeamCarverSquareKeepout(
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
    VerticalSeamCarver(marginEnergy, pNewPixelEnergyCalculator)
{}

cv::VerticalSeamCarverSquareKeepout::VerticalSeamCarverSquareKeepout(
    size_t numRows,
    size_t numColumns,
    size_t startingRow,
    size_t startingColumn,
    size_t keepoutWidth,
    size_t keepoutHeight,
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
    VerticalSeamCarver(numRows, numColumns, marginEnergy, pNewPixelEnergyCalculator)
{
    setKeepoutRegion(startingRow, startingColumn, keepoutWidth, keepoutHeight);
}

cv::VerticalSeamCarverSquareKeepout::VerticalSeamCarverSquareKeepout(
    const cv::Mat& image,
    size_t startingRow,
    size_t startingColumn,
    size_t keepoutWidth,
    size_t keepoutHeight,
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
    VerticalSeamCarver(image, marginEnergy, pNewPixelEnergyCalculator)
{
    setKeepoutRegion(startingRow, startingColumn, keepoutWidth, keepoutHeight);
}

void cv::VerticalSeamCarverSquareKeepout::runSeamRemover(size_t numSeamsToRemove,
                                                         const cv::Mat& image,
                                                         cv::Mat& outImage)
{
    try
    {
        // make sure keepout region is defined
        if (!bSquareKeepoutRegionDefined)
        {
            CV_Error(Error::Code::StsInternal, "Keepout region hasn't been defined");
        }
        else
        {
            VerticalSeamCarver::runSeamRemover(numSeamsToRemove, image, outImage);
        }
    }
    catch (...)
    {
        throw;
    }
}

void cv::VerticalSeamCarverSquareKeepout::setKeepoutRegion(size_t startingRow,
                                                           size_t startingColumn,
                                                           size_t width,
                                                           size_t height)
{
    if (areKeepoutDimensionsValid(startingRow, startingColumn, width, height))
    {
        keepoutRegionDimensions_.row_ = startingRow;
        keepoutRegionDimensions_.column_ = startingColumn;
        keepoutRegionDimensions_.height_ = height;
        keepoutRegionDimensions_.width_ = width;
        bSquareKeepoutRegionDefined = true;
    }
    else
    {
        CV_Error(Error::Code::StsBadArg, "Keepout region dimensions invalid");
    }

}

bool cv::VerticalSeamCarverSquareKeepout::isKeepoutRegionDefined() const
{
    return bSquareKeepoutRegionDefined;
}

void cv::VerticalSeamCarverSquareKeepout::resetLocalVectors()
{
    // check keepout dimensions so as not to index into local data vectors past their size
    if (!areKeepoutDimensionsValid(keepoutRegionDimensions_.row_,
                                   keepoutRegionDimensions_.column_,
                                   keepoutRegionDimensions_.width_,
                                   keepoutRegionDimensions_.height_))
    {
        CV_Error(Error::Code::StsBadArg, "Keepout region dimensions invalid");
    }

    VerticalSeamCarver::resetLocalVectors();

    for (size_t row = keepoutRegionDimensions_.row_;
         row < keepoutRegionDimensions_.row_ + keepoutRegionDimensions_.height_; row++)
    {
        for (size_t column = keepoutRegionDimensions_.column_;
             column < keepoutRegionDimensions_.column_ + keepoutRegionDimensions_.width_;
             column++)
        {
            markedPixels[row][column] = true;
        }
    }
}

bool cv::VerticalSeamCarverSquareKeepout::areKeepoutDimensionsValid(size_t startingRow,
                                                                    size_t startingColumn,
                                                                    size_t width,
                                                                    size_t height)
{
    if (!areDimensionsInitialized())
    {
        CV_Error(Error::Code::StsInternal,
                 "internal data/dimensions uninitialized. can't verify keepout region");
    }

    if (height == 0 ||
        width == 0 ||
        startingColumn > rightColumn_ ||
        startingRow > bottomRow_ ||
        (startingColumn + width > rightColumn_ + 1) ||
        (startingRow + height > bottomRow_ + 1)
        )
    {
        return false;
    }
    return true;
}
