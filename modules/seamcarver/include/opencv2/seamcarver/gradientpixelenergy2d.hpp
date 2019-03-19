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

#ifndef OPENCV_SEAMCARVER_GRADIENTPIXELENERGY2D_HPP
#define OPENCV_SEAMCARVER_GRADIENTPIXELENERGY2D_HPP

#include "opencv2/seamcarver/pixelenergy2d.hpp"
#include <opencv2/core.hpp>
#include <vector>

namespace cv
{
    class CV_EXPORTS GradientPixelEnergy2D : public PixelEnergy2D
    {
    public:
        /**
         * @brief Default ctor, where the default pixel energy at the edges is 390150.0
         * @param marginEnergy: energy defined for border pixels
         */
        explicit GradientPixelEnergy2D(double marginEnergy = 390150.0);

        virtual ~GradientPixelEnergy2D();

        /**
         * @brief calculate energy for every pixel in image
         * @param image: 2D matrix representation of the image
         * @param outPixelEnergy: Out parameter, 2D vector of calculated pixel energies
         * @return bool: indicates if the operation was successful
         */
        virtual void calculatePixelEnergy(const cv::Mat& image,
                                         std::vector<std::vector<double>>& outPixelEnergy) override;

        // Deleted/defaulted
        GradientPixelEnergy2D(const GradientPixelEnergy2D&) = delete;
        GradientPixelEnergy2D(const GradientPixelEnergy2D&&) = delete;
        virtual GradientPixelEnergy2D& operator=(const GradientPixelEnergy2D&) = delete;
        virtual GradientPixelEnergy2D& operator=(const GradientPixelEnergy2D&&) = delete;

    protected:
        /**
         * @brief
         * @param Image: 2D matrix representation of the image
         * @param OutPixelEnergy: Out parameter, 2D vector of calculated pixel energies
         * @param bDoOddColumns: Indicates whether odd or even columns are done
         * @return bool: indicates if the operation was successful
         */
        virtual void calculatePixelEnergyForEveryRow(
            const cv::Mat& image,
            std::vector<std::vector<double>>& outPixelEnergy,
            bool bDoOddColumns);

        /**
         * @brief
         * @param Image: 2D matrix representation of the image
         * @param OutPixelEnergy: Out parameter, 2D vector of calculated pixel energies
         * @param bDoOddRows: Indicates whether odd or even rows are done
         * @return bool: indicates if the operation was successful
         */
        virtual void calculatePixelEnergyForEveryColumn(
            const cv::Mat& image,
            std::vector<std::vector<double>>& outPixelEnergy,
            bool bDoOddRows);

        // store an exception if a thread throws it
        std::exception_ptr threadExceptionPtr = nullptr;

        std::mutex threadExceptionPtrMutex;
    };
}

#endif