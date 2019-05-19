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

#include "opencv2/seamcarver/verticalseamcarverarbitrarykeepout.hpp"

cv::VerticalSeamCarverArbitraryKeepout::VerticalSeamCarverArbitraryKeepout(
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
    VerticalSeamCarver(marginEnergy, pNewPixelEnergyCalculator)
{}

cv::VerticalSeamCarverArbitraryKeepout::VerticalSeamCarverArbitraryKeepout(
    size_t numRows,
    size_t numColumns,
    const std::vector<std::vector<size_t>>& keepoutRegion,
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
    VerticalSeamCarver(numRows, numColumns, marginEnergy, pNewPixelEnergyCalculator)
{
    setKeepoutRegion(keepoutRegion);
}

cv::VerticalSeamCarverArbitraryKeepout::VerticalSeamCarverArbitraryKeepout(
            const cv::Mat& image,
            const std::vector<std::vector<size_t>>& keepoutRegion,
            double marginEnergy,
            cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
            VerticalSeamCarver(image, marginEnergy, pNewPixelEnergyCalculator)
{
    setKeepoutRegion(keepoutRegion);
}

void cv::VerticalSeamCarverArbitraryKeepout::runSeamRemover(size_t numSeamsToRemove,
                                                            const cv::Mat& image,
                                                            cv::Mat& outImage)
{
    try
    {
        // verify keepout region has been defined
        if (bArbitraryKeepoutRegionDefined)
        {
            VerticalSeamCarver::runSeamRemover(numSeamsToRemove, image, outImage);
        }
        else
        {
            CV_Error(Error::Code::StsInternal, "Keepout region hasn't been defined");
        }
    }
    catch (...)
    {
        throw;
    }
}

void cv::VerticalSeamCarverArbitraryKeepout::setKeepoutRegion(
    const std::vector<std::vector<size_t>>& keepoutRegion)
{
    if (keepoutRegion.size() == 0)
    {
        CV_Error(Error::Code::StsBadArg, "Zero size keepout region");
    }

    // save a local copy
    keepoutRegion_ = keepoutRegion;

    bArbitraryKeepoutRegionDefined = true;
}

void cv::VerticalSeamCarverArbitraryKeepout::resetLocalVectors()
{
    VerticalSeamCarver::resetLocalVectors();

    setKeepoutRegionFromLocalData();
}

bool cv::VerticalSeamCarverArbitraryKeepout::isKeepoutRegionDefined() const
{
    return bArbitraryKeepoutRegionDefined;
}

inline void cv::VerticalSeamCarverArbitraryKeepout::setKeepoutRegionFromLocalData()
{
    size_t numRows = keepoutRegion_.size();

    // number of columns to mark in current row
    size_t numColumnIndices = 0;

    for (size_t row = 0; row < numRows; row++)
    {
        numColumnIndices = keepoutRegion_[row].size();
        for(size_t index = 0; index < numColumnIndices; index++)
        {
            markedPixels[row][ keepoutRegion_[row][index] ] = true;
        }
    }
}
