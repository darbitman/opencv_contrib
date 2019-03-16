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

#ifndef OPENCV_SEAMCARVER_VERTICALSEAMCARVER_HPP
#define OPENCV_SEAMCARVER_VERTICALSEAMCARVER_HPP

#include <opencv2/core.hpp>
#include "seamcarver.hpp"

namespace cv
{
    class CV_EXPORTS VerticalSeamCarver : public SeamCarver
    {
    public:
        VerticalSeamCarver(double marginEnergy = 390150.0);

        VerticalSeamCarver(size_t numRows,
                           size_t numColumns,
                           size_t numColorChannels,
                           double marginEnergy = 390150.0);

        VerticalSeamCarver(const cv::Mat& img, double marginEnergy = 390150.0);

        virtual ~VerticalSeamCarver() {}

        /**
         * @brief run the vertical seam remover algorithm
         * @param numSeams: number of vertical seams to remove
         * @param img: input image
         * @param outImg: output image parameter
         * @param computeEnergyFunction: pointer to a user-defined energy function.
         *                               If one is not provided, internal one will be used
         */
        virtual void runSeamRemover(size_t numSeams,
                                    const cv::Mat& img,
                                    cv::Mat& outImg,
                                    cv::energyFunc computeEnergyFunction = nullptr) override;

        // Deleted/defaulted
        VerticalSeamCarver(const VerticalSeamCarver& rhs) = delete;
        VerticalSeamCarver(const VerticalSeamCarver&& rhs) = delete;
        virtual VerticalSeamCarver& operator=(const VerticalSeamCarver& rhs) = delete;
        virtual VerticalSeamCarver& operator=(const VerticalSeamCarver&& rhs) = delete;

    protected:
        /**
         * @brief find then remove remove vertical seams
         * @param numSeams: number of vertical seams to remove
         * @param img: input image
         * @param outImg: output image parameter
         * @param computeEnergyFunction: pointer to a user-defined energy function.
         *                               If one is not provided, internal one will be used
         */
        virtual void findAndRemoveSeams(const size_t& numSeams,
                                        const cv::Mat& img,
                                        cv::Mat& outImg,
                                        cv::energyFunc computeEnergyFunction) override;

        /**
         * @brief calculates the energy required to reach bottom row
         */
        virtual void calculateCumulativePathEnergy() override;

        /**
         * @brief find vertical seams for later removal
         * @param numSeams: number of seams to find
         */
        virtual void findSeams(size_t numSeams) override;

        /**
         * @brief remove vertical seams from img
         */
        virtual void removeSeams() override;
    };

}

#endif