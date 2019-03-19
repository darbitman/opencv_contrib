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

#ifndef OPENCV_SEAMCARVER_SEAMCARVER_HPP
#define OPENCV_SEAMCARVER_SEAMCARVER_HPP

#include <opencv2/core.hpp>
#include "constsizeminbinaryheap.hpp"

namespace cv
{
    typedef void(*energyFunc)(const cv::Mat& img, std::vector<std::vector<double>>& outPixelEnergy);
    typedef std::vector<cv::ConstSizeMinBinaryHeap<int32_t>> vectorOfMinOrientedPQ;

    // forward declare
    class PixelEnergy2D;

    class CV_EXPORTS SeamCarver
    {
    public:
        virtual ~SeamCarver();

        /**
         * @brief run the seam remover algorithm
         * @param numSeams: number of seams to remove
         * @param img: input image
         * @param outImg: output image parameter
         */
        virtual void runSeamRemover(size_t numSeams,
                                    const cv::Mat& img,
                                    cv::Mat& outImg) = 0;

        /**
         * @brief return the image width expected
         * @return size_t
         */
        virtual size_t getNumberOfColumns() const;

        /**
         * @brief return the image height expected
         * @return size_t
         */
        virtual size_t getNumberOfRows() const;

        /**
         * @brief set the expected image dimensions
         * @param numRows: image height
         * @param numColumns: image width
         */
        virtual void setDimensions(size_t numRows, size_t numColumns);

        /**
         * @brief set the expected image dimensions based on a sample image
         * @param img: sample of the expected image
         */
        virtual void setDimensions(const cv::Mat& img);

        /**
         * @brief sets a new pixel energy calculator
         * @param pNewPixelEnergyCalculator: pointer to a new pixel energy calculator
         */
        virtual void setPixelEnergyCalculator(PixelEnergy2D* pNewPixelEnergyCalculator);

        /**
         * @brief return true if dimensions have been set
         * @param return bool
         */
        virtual bool areDimensionsInitialized() const;

        // Deleted/defaulted functions
        SeamCarver(const SeamCarver& rhs) = delete;
        SeamCarver(const SeamCarver&& rhs) = delete;
        virtual SeamCarver& operator=(const SeamCarver& rhs) = delete;
        virtual SeamCarver& operator=(const SeamCarver&& rhs) = delete;

    protected:
        /**
         * @brief default ctor
         * @param marginEnergy: defines the edge pixel energy
         * @param pPixelEnergy2D: pointer to a pixel energy calculator
         */
        SeamCarver(double marginEnergy = 390150.0, PixelEnergy2D* pPixelEnergy2D = nullptr);

        /**
         * @brief find and remove seams
         * @param numSeams: number of seams to remove
         * @param img: input image
         * @param outImg: output image parameter
         */
        virtual void findAndRemoveSeams(const size_t& numSeams,
                                        const cv::Mat& img,
                                        cv::Mat& outImg) = 0;

        /**
         * @brief calculates the energy required to reach the end
         */
        virtual void calculateCumulativePathEnergy() = 0;

        /**
         * @brief find seams for later removal
         * @param numSeams: number of seams to discover for removal
         */
        virtual void findSeams(size_t numSeams) = 0;

        /**
         * @brief remove seams from img given by column locations stored in seam
         * @param bgr: image separate into 3 channels (BLUE GREEN RED)
         * @param seams: vector of priority queues that hold the locations of the pixels to remove
         */
        virtual void removeSeams() = 0;

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
        virtual void init(size_t numRows,
                          size_t numColumns,
                          size_t seamLength);

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
        virtual void resetLocalVectors(size_t numSeams);

        // vector to store pixels that have been previously markedPixels for removal
        // will ignore these markedPixels pixels when searching for a new seam
        std::vector<std::vector<bool>> markedPixels;

        // individual pixel energy
        std::vector<std::vector<double>> pixelEnergy;

        // vector of min oriented priority queues that store the location of the pixels to remove
        vectorOfMinOrientedPQ discoveredSeams;

        // store cumulative energy to each pixel
        std::vector<std::vector<double>> totalEnergyTo;

        // store the location (column or row) of the previous pixel to get to the current pixel
        std::vector<std::vector<int32_t>> previousLocationTo;

        // store the current seam being discovered
        std::vector<size_t> currentSeam;

        // vector to hold image color channels separately
        std::vector<cv::Mat> bgr;

        bool needToInitializeLocalData = true;

        // default energy at the borders of the image
        const double marginEnergy_;

        // image dimensions
        size_t numRows_  = 0;
        size_t numColumns_ = 0;
        size_t bottomRow_ = 0;
        size_t rightColumn_ = 0;
        size_t numColorChannels_ = 0;

        // number of pixels per seam
        size_t seamLength_ = 0;

        double posInf_ = std::numeric_limits<double>::max();

        cv::PixelEnergy2D* pixelEnergyCalculator_ = nullptr;
    };

}

#endif