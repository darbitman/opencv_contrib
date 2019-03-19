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

#ifndef OPENCV_SEAMCARVER_PIXELENERGY2D_HPP
#define OPENCV_SEAMCARVER_PIXELENERGY2D_HPP

#include <opencv2/core.hpp>
#include <vector>

namespace cv
{
    class CV_EXPORTS PixelEnergy2D
    {
    public:
        virtual ~PixelEnergy2D();

        /**
         * @brief sets the marginEnergy (positive only)
         * @param marginEnergy: energy of the edge pixels
         */
        virtual void setMarginEnergy(double marginEnergy);

        /**
         * @brief returns the marginEnergy associated with this calculator
         * @return double
         */
        virtual double getMarginEnergy() const;

        /**
         * @brief
         * @param image: 2D matrix representation of the image
         * @param outPixelEnergy: Out parameter, 2D vector of calculated pixel energies
         * @return bool: indicates if the operation was successful
         */
        virtual void calculatePixelEnergy(const cv::Mat& image,
                                          std::vector<std::vector<double>>& outPixelEnergy) = 0;

        // Deleted/defaulted
        PixelEnergy2D(const PixelEnergy2D&) = delete;
        PixelEnergy2D(const PixelEnergy2D&&) = delete;
        virtual PixelEnergy2D& operator=(const PixelEnergy2D&) = delete;
        virtual PixelEnergy2D& operator=(const PixelEnergy2D&&) = delete;

    protected:
        /**
         * @brief Default ctor, where the default pixel energy at the edges is 390150.0
         * @param marginEnergy: energy defined for border pixels
         */
        explicit PixelEnergy2D(double marginEnergy = 390150.0);

        // energy at the borders of an image
        double marginEnergy_ = 0.0;

        // image dimensions
        size_t numRows_ = 0;
        size_t numColumns_ = 0;
        size_t bottomRow_ = 0;
        size_t rightColumn_ = 0;
        size_t numColorChannels_ = 0;
    };
}

#endif