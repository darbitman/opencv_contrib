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

#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::VerticalSeamCarverData::VerticalSeamCarverData(double marginEnergy)
    : bNeedToInitializeLocalData(true),
      numRows_(0),
      numColumns_(0),
      bottomRow_(0),
      rightColumn_(0),
      numColorChannels_(0),
      seamLength_(0),
      numSeamsToRemove_(0),
      marginEnergy_(marginEnergy)
{
    posInf_ = std::numeric_limits<double>::max();
    pPixelEnergyCalculator_ = nullptr;
}

cv::VerticalSeamCarverData::~VerticalSeamCarverData() {}

void cv::VerticalSeamCarverData::resetData()
{
    for (size_t row = 0; row < numRows_; ++row)
    {
        for (size_t column = 0; column < numColumns_; ++column)
        {
            markedPixels[row][column] = false;
        }
    }

    // ensure each row's PQ has enough capacity
    for (size_t row = 0; row < seamLength_; ++row)
    {
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
