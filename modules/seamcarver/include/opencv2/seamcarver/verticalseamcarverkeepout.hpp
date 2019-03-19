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

#ifndef OPENCV_SEAMCARVER_VERTICALSEAMCARVERKEEPOUT_HPP
#define OPENCV_SEAMCARVER_VERTICALSEAMCARVERKEEPOUT_HPP

#include <opencv2/core.hpp>
#include "opencv2/seamcarver/verticalseamcarver.hpp"

namespace cv
{
    struct CV_EXPORTS keepoutRegionDimensionsStruct
    {
        size_t row_ = 0;
        size_t column_ = 0;
        size_t height_ = 0;
        size_t width_ = 0;
    };

    class CV_EXPORTS VerticalSeamCarverKeepout : public VerticalSeamCarver
    {
    public:
        /**
         * @brief default ctor
         * @param marginEnergy: defines the edge pixel energy
         * @param pPixelEnergy2D: pointer to a pixel energy calculator
         */
        VerticalSeamCarverKeepout(double marginEnergy = 390150.0,
                                  PixelEnergy2D* pPixelEnergy2D = nullptr);

        /**
         * @brief ctor based on dimensions
         * @param numRows: image height
         * @param numColumns: image width
         * @param startingRow: starting row of keepout region
         * @param startingColumn: starting column of keepout region
         * @param regionWidth: keepout region width
         * @param regionHeight: keepout region height
         * @param marginEnergy: defines the edge pixel energy
         * @param pPixelEnergy2D: pointer to a pixel energy calculator
         */
        VerticalSeamCarverKeepout(size_t numRows,
                                  size_t numColumns,
                                  size_t startingRow,
                                  size_t startingColumn,
                                  size_t regionWidth,
                                  size_t regionHeight,
                                  double marginEnergy = 390150.0,
                                  PixelEnergy2D* pPixelEnergy2D = nullptr);

        /**
         * @brief ctor based on sample image
         * @param img: sample image
         * @param startingRow: starting row of keepout region
         * @param startingColumn: starting column of keepout region
         * @param regionWidth: keepout region width
         * @param regionHeight: keepout region height
         * @param marginEnergy: defines the edge pixel energy
         * @param pPixelEnergy2D: pointer to a pixel energy calculator
         */
        VerticalSeamCarverKeepout(const cv::Mat& img,
                                  size_t startingRow,
                                  size_t startingColumn,
                                  size_t regionWidth,
                                  size_t regionHeight,
                                  double marginEnergy = 390150.0,
                                  PixelEnergy2D* pPixelEnergy2D = nullptr);

        virtual ~VerticalSeamCarverKeepout() {}

        /**
         * @brief run the vertical seam remover algorithm avoiding the keepout region
         * @param numSeams: number of vertical seams to remove
         * @param img: input image
         * @param outImg: output image parameter
         */
        virtual void runSeamRemover(size_t numSeams,
                                    const cv::Mat& img,
                                    cv::Mat& outImg) override;

        /**
         * @brief set the keepout region dimensions
         * @param startingRow: starting row of keepout region
         * @param startingColumn: starting column of keepout region
         * @param regionWidth: keepout region width
         * @param regionHeight: keepout region height
         */
        virtual void setKeepoutRegion(size_t startingRow,
                                      size_t startingColumn,
                                      size_t width,
                                      size_t height);

        /**
         * @brief returns true if a keepout region has been defined
         * @return bool
         */
        virtual bool isKeepoutRegionDefined() const;


        // Deleted/defaulted functions
        VerticalSeamCarverKeepout(const VerticalSeamCarverKeepout& rhs) = delete;
        VerticalSeamCarverKeepout(const VerticalSeamCarverKeepout&& rhs) = delete;
        virtual VerticalSeamCarverKeepout& operator=(const VerticalSeamCarverKeepout& rhs) = delete;
        virtual VerticalSeamCarverKeepout& operator=(const VerticalSeamCarverKeepout&& rhs)= delete;

    protected:
        /**
         * @brief reset vectors to their starting state
         */
        virtual void resetLocalVectors() override;

        bool bKeepoutRegionDefined = false;

        keepoutRegionDimensionsStruct keepoutRegionDimensions_;
    };

}

#endif