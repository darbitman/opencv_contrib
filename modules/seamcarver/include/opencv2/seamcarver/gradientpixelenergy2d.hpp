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

#include <opencv2/core.hpp>
#include "opencv2/seamcarver/pixelenergy2d.hpp"
#include <vector>

namespace cv
{
    class CV_EXPORTS GradientPixelEnergyCalculator2D : public PixelEnergyCalculator2D
    {
    public:
        /**
         * @brief Default ctor, where the default pixel energy at the edges is 390150.0
         * @param marginEnergy: border pixel energy
         */
        explicit GradientPixelEnergyCalculator2D(double newMarginEnergy = 390150.0);

        /**
         * @brief dtor
         */
        virtual ~GradientPixelEnergyCalculator2D();

        /**
         * @brief run the pixel energy calculation on image and place the result into a 2D std::vector
         * @param image: 2D matrix representation of the image
         * @param outPixelEnergy: output parameter 2D vector of computed pixel energies
         */
        virtual void calculatePixelEnergy(const cv::Mat& image,
                std::vector<std::vector<double>>& outPixelEnergy) override;
        
        /**
         * @brief sets the energy of the border pixels
         * @param newMarginEnergy: energy of the border pixels
         */
        virtual void setMarginEnergy(double newMarginEnergy);

        /**
         * @brief returns the energy of edge pixels
         * @return double
         */
        virtual double getMarginEnergy() const;

        // deleted to prevent misuse
        GradientPixelEnergyCalculator2D(const GradientPixelEnergyCalculator2D&) = delete;
        GradientPixelEnergyCalculator2D(const GradientPixelEnergyCalculator2D&&) = delete;
        virtual GradientPixelEnergyCalculator2D& operator=(const GradientPixelEnergyCalculator2D&) = delete;
        virtual GradientPixelEnergyCalculator2D& operator=(const GradientPixelEnergyCalculator2D&&) = delete;

    protected:
        /**
         * @brief calculate pixel energy for all rows (but either odd or even columns)
         * @param image: 2D matrix representation of the image
         * @param outPixelEnergy: Out parameter, 2D vector of calculated pixel energies
         * @param bDoOddColumns: Indicates whether odd or even columns are done
         * @return bool: indicates if the operation was successful
         */
        virtual void calculatePixelEnergyForEveryRow(
            const cv::Mat& image,
            std::vector<std::vector<double>>& outPixelEnergy,
            bool bDoOddColumns);

        /**
         * @brief calculate pixel energy for all columns (but either odd or even rows)
         * @param image: 2D matrix representation of the image
         * @param outPixelEnergy: output parameter 2D vector of computed pixel energies
         * @param bDoOddRows: indicates whether odd or even rows are done
         * @return bool: indicates if the operation was successful
         */
        virtual void calculatePixelEnergyForEveryColumn(
            const cv::Mat& image,
            std::vector<std::vector<double>>& outPixelEnergy,
            bool bDoOddRows);

        // store an exception if a thread throws it
        std::exception_ptr threadExceptionPtr = nullptr;

        // mutex to protect the exception ptr above
        std::mutex threadExceptionPtrMutex;

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