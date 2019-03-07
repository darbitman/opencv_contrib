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

#ifndef OPENCV_SEAMCARVER_SEAMCARVERKEEPOUT_HPP
#define OPENCV_SEAMCARVER_SEAMCARVERKEEPOUT_HPP

#include <opencv2/core.hpp>
#include <opencv2/seamcarver/seamcarver.hpp>

namespace cv
{
    struct keepoutRegionDimensionsStruct
    {
        size_t row_ = 0;
        size_t column_ = 0;
        size_t height_ = 0;
        size_t width_ = 0;
    };

    class CV_EXPORTS SeamCarverKeepout : public SeamCarver
    {
    public:
        SeamCarverKeepout(double marginEnergy = 390150.0);

        SeamCarverKeepout(size_t startingRow,
                          size_t startingColumn,
                          size_t regionWidth,
                          size_t regionHeight,
                          double marginEnergy = 390150.0);

        virtual ~SeamCarverKeepout() {}

        virtual void runVerticalSeamRemover(size_t numSeams,
                                            const cv::Mat& img,
                                            cv::Mat& outImg,
                                           cv::energyFunc computeEnergyFunction = nullptr) override;

        virtual void setKeepoutRegion(size_t startingRow,
                                      size_t startingColumn,
                                      size_t height,
                                      size_t width);


        // Delete/defaulted go here
        SeamCarverKeepout(const SeamCarverKeepout& rhs) = delete;
        SeamCarverKeepout(const SeamCarverKeepout&& rhs) = delete;
        virtual SeamCarverKeepout& operator=(const SeamCarverKeepout& rhs) = delete;
        virtual SeamCarverKeepout& operator=(const SeamCarverKeepout&& rhs) = delete;

    protected:
        virtual void resetLocalVectors(size_t numSeams) override;

        bool keepoutRegionDefined = false;

        keepoutRegionDimensionsStruct keepoutRegionDimensions_;
    };

}

#endif