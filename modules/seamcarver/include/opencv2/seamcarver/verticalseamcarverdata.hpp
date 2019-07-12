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

#ifndef OPENCV_SEAMCARVER_VERTICALSEAMCARVERDATA_HPP
#define OPENCV_SEAMCARVER_VERTICALSEAMCARVERDATA_HPP

#include <opencv2/core.hpp>
#include "constsizeminpriorityqueue.hpp"

namespace cv
{
    // forward declare class;
    class PixelEnergyCalculator2D;

    class CV_EXPORTS VerticalSeamCarverData
    {
    public:
        VerticalSeamCarverData(double marginEnergy) : marginEnergy_(marginEnergy) {}

        // flag if internal data structures need their memory and values initialized
        bool bNeedToInitializeLocalData = true;

        // vector to store pixels that have been previously markedPixels for removal
        // will ignore these markedPixels pixels when searching for a new seam
        std::vector<std::vector<bool>> markedPixels;

        // individual pixel energy
        std::vector<std::vector<double>> pixelEnergy;

        // vector of min oriented priority queues that store the location of the pixels to remove
        // the index to the vector corresponds to the rows
        // the column index is stored in the PQ
        std::vector<cv::ConstSizeMinPriorityQueue<int32_t>> discoveredSeams;

        // store cumulative energy to each pixel
        std::vector<std::vector<double>> totalEnergyTo;

        // store the column of the pixel in the "previous" row to get to the current pixel
        std::vector<std::vector<int32_t>> columnTo;

        // store the current seam being discovered
        std::vector<size_t> currentSeam;

        // vector to hold image color channels separately
        std::vector<cv::Mat> bgr;

        // image dimensions
        size_t numRows_ = 0;
        size_t numColumns_ = 0;
        size_t bottomRow_ = 0;
        size_t rightColumn_ = 0;
        size_t numColorChannels_ = 0;

        // number of pixels per seam
        size_t seamLength_ = 0;

        // value of positive infinity
        double posInf_ = std::numeric_limits<double>::max();

        // number of seams to remove (updated every run)
        size_t numSeamsToRemove_ = 0;

        // pointer to an object that calculates pixel energy
        cv::Ptr<PixelEnergyCalculator2D> pPixelEnergyCalculator_ = nullptr;

        // default energy at the borders of the image
        const double marginEnergy_;

        // stores the image and output result
        cv::Ptr<cv::Mat> savedImage;
    };
}

#endif
