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
#include <queue>
#include <thread>
#include <mutex>
#include "seamcarver.hpp"

namespace cv
{
    class VerticalSeamCarverData;
    class PixelEnergyCalculator2D;

    class CV_EXPORTS VerticalSeamCarver : public SeamCarver
    {
    public:
        /**
         * @brief default ctor
         * @param marginEnergy: defines the edge pixel energy
         * @param pNewPixelEnergyCalculator: pointer to a pixel energy calculator
         */
        VerticalSeamCarver(double marginEnergy = defaultMarginEnergy,
                cv::Ptr<PixelEnergyCalculator2D> pNewPixelEnergyCalculator = nullptr);

        /**
         * @brief ctor based on dimensions
         * @param numRows: image height
         * @param numColumns: image width
         * @param marginEnergy: defines the edge pixel energy
         * @param pNewPixelEnergyCalculator: pointer to a pixel energy calculator
         */
        VerticalSeamCarver(size_t numRows,
                size_t numColumns,
                double marginEnergy = defaultMarginEnergy,
                cv::Ptr<PixelEnergyCalculator2D> pNewPixelEnergyCalculator = nullptr);

        /**
         * @brief ctor based on a sample image
         * @param image: sample image
         * @param marginEnergy: defines the edge pixel energy
         * @param pNewPixelEnergyCalculator: pointer to a pixel energy calculator
         */
        VerticalSeamCarver(const cv::Mat& image,
                double marginEnergy = defaultMarginEnergy,
                cv::Ptr<PixelEnergyCalculator2D> pNewPixelEnergyCalculator = nullptr);
        /**
         * @brief dtor
         */
        virtual ~VerticalSeamCarver();

        /**
         * @brief run the vertical seam remover algorithm
         * @param numSeamsToRemove: number of vertical seams to remove
         * @param image: input image
         */
        virtual void runSeamRemover(size_t numSeamsToRemove, const cv::Mat& image) override;

        /**
         * @brief attempt to get the next result (non blocking)
         * @return cv::Ptr<cv::Mat>
         */
        virtual cv::Ptr<cv::Mat> tryGetNextFrame();

        /**
         * @brief get the next result (blocking)
         * @return cv::Ptr<cv::Mat>
         */
        virtual cv::Ptr<cv::Mat> getNextFrame();

        /**
         * @brief returns the total number of frames still being processed in the pipeline
         * @returns size_t
         */
        virtual size_t numberFramesBeingProcessed() const;

        /**
         * @brief returns true if new result exists
         * @return bool
         */
        virtual bool newResultExists() const;

        // Deleted/defaulted functions
        VerticalSeamCarver(const VerticalSeamCarver& rhs) = delete;
        VerticalSeamCarver(const VerticalSeamCarver&& rhs) = delete;
        virtual VerticalSeamCarver& operator=(const VerticalSeamCarver& rhs) = delete;
        virtual VerticalSeamCarver& operator=(const VerticalSeamCarver&& rhs) = delete;

    // nonthreaded operations
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
         * @brief reset vectors to their starting state
         */
        virtual void resetLocalVectors();

        /**
         * @brief find then remove remove vertical seams
         */
        virtual void findAndRemoveSeams();

        /**
         * @brief calculates the energy required to reach bottom row (non threaded)
         * @param data: pointer to the data for which to run calculation
         */
        virtual void runCumulativePathEnergyCalculation(VerticalSeamCarverData* data);

        /**
         * @brief check if internal stored dimensions match those of the new image
         * @param image: input image
         * @return bool
         */
        virtual bool areImageDimensionsVerified(const cv::Mat& image) const;

        virtual void constructorInit(double marginEnergy, cv::Ptr<PixelEnergyCalculator2D> pNewPixelEnergyCalculator);

    // threaded operations
    protected:
        /**
         * @brief calculate individual pixel energy
         */
        virtual void calculatePixelEnergy();

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
         * @brief combines the results and adds image to return queue
         */
        virtual void mergeChannels();

    // local variables
    protected:
        std::vector<std::thread> threads;
        
        std::vector<std::queue<VerticalSeamCarverData*>> localDataQueues;        

        bool runThreads = true;

        enum class pipelineStage
        {
            STAGE_0,    // initialize everything
            STAGE_1,    // compute energy
            STAGE_2,    // calculate cumulative path energy
            STAGE_3,    // find seams
            STAGE_4,    // remove seams
            STAGE_5,    // merge channels
            STAGE_6,    // return queue
            NUM_STAGES
        };

        static constexpr size_t pipelineDepth = static_cast<size_t>(pipelineStage::NUM_STAGES);

        std::mutex mutexes[pipelineDepth];
        std::vector<std::unique_lock<std::mutex>> queueLocks;
        
        static constexpr double defaultMarginEnergy = 390150.0;

        cv::Ptr<std::queue<cv::Ptr<cv::Mat>>> pReturnQueue;

        size_t totalFramesInQueues = 0;
    };
}

#endif