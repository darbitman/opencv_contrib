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

#include "opencv2/seamcarver/pixelenergy2d.hpp"
#include <thread>
using std::vector;

cv::PixelEnergy2D::PixelEnergy2D(double marginEnergy)
{
    try
    {
        setMarginEnergy(marginEnergy);
    }
    catch (...)
    {
        throw;
    }
}

cv::PixelEnergy2D::PixelEnergy2D(size_t numColumns,
                                 size_t numRows,
                                 size_t numColorChannels,
                                 double marginEnergy)
{
    try
    {
        setDimensions(numColumns, numRows);
        setMarginEnergy(marginEnergy);
        setNumColorChannels(numColorChannels);
    }
    catch (...)
    {
        throw;
    }
}

cv::PixelEnergy2D::PixelEnergy2D(const cv::Mat& image, double marginEnergy)
{
    try
    {
        setDimensions((size_t)image.cols, (size_t)image.rows);
        setMarginEnergy(marginEnergy);
        setNumColorChannels((size_t)image.channels());
    }
    catch (...)
    {
        throw;
    }
}

cv::PixelEnergy2D::~PixelEnergy2D() {}

void cv::PixelEnergy2D::setMarginEnergy(double marginEnergy)
{
    if (marginEnergy < 0.0)
    {
        CV_Error(Error::Code::StsInternal, "Can't set negative margin energy");
    }

    marginEnergy_ = marginEnergy;
}

void cv::PixelEnergy2D::setDimensions(size_t numColumns, size_t numRows)
{
    if (numColumns <= 0 || numRows <= 0)
    {
        CV_Error(Error::Code::StsInternal,
                 "PixelEnergy2D::setDimensions() failed due to invalid input dimensions");
    }

    imageDimensions_.numColumns_ = numColumns;
    imageDimensions_.numRows_ = numRows;
    bDimensionsInitialized = true;
}

void cv::PixelEnergy2D::setNumColorChannels(size_t numColorChannels)
{
    if (!(numColorChannels == 1 || numColorChannels == 3))
    {
        CV_Error(Error::Code::StsInternal,
                 "PixelEnergy2D::setDimensions() failed due to incorrect color channels");
    }

    numColorChannels_ = numColorChannels;
    bNumColorChannelsInitialized = true;
}

double cv::PixelEnergy2D::getMarginEnergy() const
{
    return marginEnergy_;
}

cv::ImageDimensionStruct cv::PixelEnergy2D::getDimensions() const
{
    if (!bDimensionsInitialized)
    {
        CV_Error(Error::Code::StsInternal,
                 "Uninitialized internal dimensions");
    }

    return imageDimensions_;
}

size_t cv::PixelEnergy2D::getNumColorChannels() const
{
    if (!bNumColorChannelsInitialized)
    {
        CV_Error(Error::Code::StsInternal,
                 "Uninitialized number of color channels");
    }

    return numColorChannels_;
}

bool cv::PixelEnergy2D::areDimensionsSet() const
{
    return bDimensionsInitialized;
}

bool cv::PixelEnergy2D::isNumColorChannelsSet() const
{
    return bNumColorChannelsInitialized;
}