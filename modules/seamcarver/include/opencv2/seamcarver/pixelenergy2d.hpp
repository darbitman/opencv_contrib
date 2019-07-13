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

#ifndef OPENCV_SEAMCARVER_PIXELENERGYCALCULATOR2D_HPP
#define OPENCV_SEAMCARVER_PIXELENERGYCALCULATOR2D_HPP

#include <opencv2/core.hpp>
#include <vector>

namespace cv
{
class CV_EXPORTS PixelEnergyCalculator2D
{
public:
    /**
     * @brief dtor
     */
    virtual ~PixelEnergyCalculator2D();

    /**
     * @brief run the pixel energy calculation on image and place the result into a 2D std::vector
     * @param image: 2D matrix representation of the image
     * @param outPixelEnergy: output parameter 2D vector of computed pixel energies
     */
    virtual void calculatePixelEnergy(const cv::Mat& image,
                                      std::vector<std::vector<double>>& outPixelEnergy) = 0;

    /**
     * @brief sets the energy of the border pixels
     * @param newMarginEnergy: energy of the border pixels
     */
    virtual void setMarginEnergy(double newMarginEnergy) = 0;

    /**
     * @brief returns the energy of edge pixels
     * @return double
     */
    virtual double getMarginEnergy() const = 0;
};
}  // namespace cv

#endif