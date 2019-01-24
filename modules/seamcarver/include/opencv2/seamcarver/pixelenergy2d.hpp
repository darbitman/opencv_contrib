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
#include <stdint.h>
#include <vector>
#include "imagedimensionstruct.hpp"
using std::vector;


namespace cv
{
    class CV_EXPORTS PixelEnergy2D
    {
    public:
        /**
         * @brief Default ctor, where the default pixel energy at the edges is 390150.0
         * @param marginEnergy: energy defined for border pixels
         */
        explicit PixelEnergy2D(double marginEnergy = 390150.0);

        /**
         * @brief ctor that will initialize internal memory and
         * @param numColumns: width of the image in pixels
         * @param numRows: height of the image in pixels
         * @param numChannels: number of color channels in image (1 for grayscale, 3 for BGR color)
         * @param marginEnergy: energy defined for border pixels
         */
        explicit PixelEnergy2D(int32_t numColumns, int32_t numRows,
                               int32_t numChannels, double marginEnergy = 390150.0);

        /**
         * @brief ctor that will initialize image dimensions based on the image itself
         * @param image: the image whose dimensions will be extracted
         * @param marginEnergy: energy defined for border pixels
         */
        explicit PixelEnergy2D(const cv::Mat& image, double marginEnergy = 390150.0);

        /**
         * @brief copy ctor
         * @param other: the object whose parameters will be copied
         */
        explicit PixelEnergy2D(const PixelEnergy2D& other);

        virtual ~PixelEnergy2D();

        /**
         * @brief returns the marginEnergy associated with this calculator
         * @return double
         */
        virtual double getMarginEnergy() const;

        /**
         * @brief sets the marginEnergy
         */
        virtual void setMarginEnergy(double MarginEnergy);

        /**
         * @brief returns the image dimensions
         * @return cv::ImageDimensionStruct
         */
        virtual cv::ImageDimensionStruct getDimensions() const;

        /**
         * @brief sets the image dimensions
         * @param numColumns: width of the image in pixels
         * @param numRows: height of the image in pixels
         * @param numChannels: number of color channels in image (1 for grayscale, 3 for BGR color)
         */
        virtual void setDimensions(int32_t numColumns, int32_t numRows, int32_t numChannels);

        /**
         * @brief
         * @param image: 2D matrix representation of the image
         * @param outPixelEnergy: Out parameter, 2D vector of calculated pixel energies
         * @return bool: indicates if the operation was successful
         */
        virtual void calculatePixelEnergy(const cv::Mat& image,
                                          vector<vector<double>>& outPixelEnergy);

        PixelEnergy2D(const PixelEnergy2D&&) = delete;
        virtual PixelEnergy2D& operator=(const PixelEnergy2D&) = delete;
        virtual PixelEnergy2D& operator=(const PixelEnergy2D&&) = delete;

    protected:
        /**
         * @brief
         * @param Image: 2D matrix representation of the image
         * @param OutPixelEnergy: Out parameter, 2D vector of calculated pixel energies
         * @param bDoOddColumns: Indicates whether odd or even columns are done
         * @return bool: indicates if the operation was successful
         */
        virtual void calculatePixelEnergyForEveryRow(const cv::Mat& image,
                                                     vector<vector<double>>& outPixelEnergy,
                                                     bool bDoOddColumns);

        /**
         * @brief
         * @param Image: 2D matrix representation of the image
         * @param OutPixelEnergy: Out parameter, 2D vector of calculated pixel energies
         * @param bDoOddRows: Indicates whether odd or even rows are done
         * @return bool: indicates if the operation was successful
         */
        virtual void calculatePixelEnergyForEveryColumn(const cv::Mat& image,
                                                        vector<vector<double>>& outPixelEnergy,
                                                        bool bDoOddRows);

    private:
        // stores number of columns, rows, color channels
        cv::ImageDimensionStruct imageDimensions_;

        // energy at the borders of an image
        double marginEnergy_ = 0.0;

        // indicates whether image dimensions and memory has already been allocated
        bool bDimensionsInitialized = false;

        // number of channels used for computing energy of a BGR image
        const int32_t numChannelsInColorImage_ = 3;
    };
}

#endif