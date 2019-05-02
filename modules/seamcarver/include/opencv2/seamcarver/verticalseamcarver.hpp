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
#include "constsizeminpriorityqueue.hpp"

namespace cv
{
    // forward declare class;
    class PixelEnergy2D;

    class CV_EXPORTS VerticalSeamCarver : public SeamCarver
    {
    public:
        /**
         * @brief default ctor
         * @param marginEnergy: defines the edge pixel energy
         * @param pNewPixelEnergyCalculator: pointer to a pixel energy calculator
         */
        VerticalSeamCarver(double marginEnergy = 390150.0,
                           cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator = nullptr);

        /**
         * @brief ctor based on dimensions
         * @param numRows: image height
         * @param numColumns: image width
         * @param marginEnergy: defines the edge pixel energy
         * @param pNewPixelEnergyCalculator: pointer to a pixel energy calculator
         */
        VerticalSeamCarver(size_t numRows,
                           size_t numColumns,
                           double marginEnergy = 390150.0,
                           cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator = nullptr);

        /**
         * @brief ctor based on a sample image
         * @param image: sample image
         * @param marginEnergy: defines the edge pixel energy
         * @param pNewPixelEnergyCalculator: pointer to a pixel energy calculator
         */
        VerticalSeamCarver(const cv::Mat& image,
                           double marginEnergy = 390150.0,
                           cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator = nullptr);
        /**
         * @brief dtor
         */
        virtual ~VerticalSeamCarver();

        /**
         * @brief run the vertical seam remover algorithm
         * @param numSeamsToRemove: number of vertical seams to remove
         * @param image: input image
         * @param outImage: output image parameter
         */
        virtual void runSeamRemover(size_t numSeamsToRemove,
                                    const cv::Mat& image,
                                    cv::Mat& outImage) override;

        /**
         * @brief set the expected image dimensions
         * @param numRows: image height
         * @param numColumns: image width
         */
        virtual void setDimensions(size_t numRows, size_t numColumns);

        /**
         * @brief set the expected image dimensions based on a sample image
         * @param image: sample of the expected image
         */
        virtual void setDimensions(const cv::Mat& image);

        /**
         * @brief return true if dimensions have been set
         * @param return bool
         */
        virtual bool areDimensionsInitialized() const;

        /**
         * @brief sets a new pixel energy calculator
         * @param pNewPixelEnergyCalculator: pointer to a new pixel energy calculator
         */
        virtual void setPixelEnergyCalculator(cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator);

        // Deleted/defaulted functions
        VerticalSeamCarver(const VerticalSeamCarver& rhs) = delete;
        VerticalSeamCarver(const VerticalSeamCarver&& rhs) = delete;
        virtual VerticalSeamCarver& operator=(const VerticalSeamCarver& rhs) = delete;
        virtual VerticalSeamCarver& operator=(const VerticalSeamCarver&& rhs) = delete;

    protected:
        /**
         * @brief initializes member data using image dimensions
         * @param img: a sample frame from which dimensions are extracted
         * @param seamLength: number of pixels per seam
         */
        virtual void init(const cv::Mat& img, size_t seamLength);

        /**
         * @brief initilizes member data using image dimensions
         * @param numRows: number of rows in the image (height)
         * @param numColumns: number of columns in the image (width)
         * @param seamLength: number of pixels per seam
         */
        virtual void init(size_t numRows, size_t numColumns, size_t seamLength);

        /**
         * @brief initializes local member variables
         * @param numRows: number of rows in the image
         * @param numColumns: number of columns in the image
         * @param bottomRow: index of the bottom row
         * @param rightColumn: index of the right column
         * @param seamLength: number of pixels per seam
         */
        virtual void initializeLocalVariables(size_t numRows,
                                              size_t numColumns,
                                              size_t bottomRow,
                                              size_t rightColumn,
                                              size_t seamLength);

        /**
         * @brief initializes memory for all local vectors used part of the algorithm
         */
        virtual void initializeLocalVectors();

        /**
         * @brief reset vectors to their starting state
         */
        virtual void resetLocalVectors();

        /**
         * @brief find then remove remove vertical seams
         * @param image: input image
         * @param outImage: output image parameter
         * @param computeEnergyFunction: pointer to a user-defined energy function.
         *                               If one is not provided, internal one will be used
         */
        virtual void findAndRemoveSeams(const cv::Mat& image, cv::Mat& outImage);

        /**
         * @brief calculates the energy required to reach bottom row
         */
        virtual void calculateCumulativePathEnergy();

        /**
         * @brief find vertical seams for later removal
         */
        virtual void findSeams();

        /**
         * @brief remove vertical seams from img
         */
        virtual void removeSeams();

        /**
         * @brief check if internal stored dimensions match those of the new image
         * @param image: input image
         * @return bool
         */
        virtual bool areImageDimensionsVerified(const cv::Mat& image) const;

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
        cv::Ptr<PixelEnergy2D> pPixelEnergyCalculator_ = nullptr;

        // default energy at the borders of the image
        const double marginEnergy_;
    };
}

#endif