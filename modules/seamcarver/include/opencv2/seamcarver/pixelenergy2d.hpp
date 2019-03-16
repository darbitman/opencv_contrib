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
    struct CV_EXPORTS ImageDimensionStruct
    {
        size_t numColumns_ = 0;
        size_t numRows_ = 0;
    };

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
         * @brief sets the image dimensions
         * @param numColumns: width of the image in pixels
         * @param numRows: height of the image in pixels
         */
        virtual void setDimensions(size_t numColumns, size_t numRows);

        /**
         * @brief sets the number of color channels in an image
         * @param numColorChannels: number of color channels in an image
         */
        virtual void setNumColorChannels(size_t numColorChannels);

        /**
         * @brief returns the marginEnergy associated with this calculator
         * @return double
         */
        virtual double getMarginEnergy() const;

        /**
         * @brief returns the image dimensions
         * @return cv::ImageDimensionStruct
         */
        virtual cv::ImageDimensionStruct getDimensions() const;

        /**
         * @brief returns the number of color channels
         * @return size_t
         */
        virtual size_t getNumColorChannels() const;

        /**
         * @brief have the dimensions been set?
         * @return bool
         */
        virtual bool areDimensionsSet() const;

        /**
         * @brief have the number of colors channels been set?
         * @return bool
         */
        virtual bool isNumColorChannelsSet() const;

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

        /**
         * @brief ctor that will initialize all image dimensions
         * @param numColumns: width of the image in pixels
         * @param numRows: height of the image in pixels
         * @param numColorChannels: how many color channels an image has
         * @param marginEnergy: energy defined for border pixels
         */
        explicit PixelEnergy2D(size_t numColumns,
                               size_t numRows,
                               size_t numColorChannels,
                               double marginEnergy = 390150.0);

        /**
         * @brief ctor that will initialize image dimensions based on the image itself
         * @param image: the image whose dimensions will be extracted
         * @param marginEnergy: energy defined for border pixels
         */
        explicit PixelEnergy2D(const cv::Mat& image, double marginEnergy = 390150.0);

        // stores number of columns, rows
        cv::ImageDimensionStruct imageDimensions_;

        size_t numColorChannels_ = 0;

        // energy at the borders of an image
        double marginEnergy_ = 0.0;

        // indicates whether image dimensions and memory has already been allocated
        bool bDimensionsInitialized = false;

        // color channels initialized
        bool bNumColorChannelsInitialized = false;

        // store an exception if a thread throws it
        //std::exception_ptr threadExceptionPtr = nullptr;

        //std::mutex threadExceptionPtrMutex;
    };
}

#endif