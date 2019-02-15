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
#include "pixelenergy2d.hpp"
#include "constsizeminbinaryheap.hpp"

namespace cv
{
    typedef void(*energyFunc)(const cv::Mat& img, vector<vector<double>>& outPixelEnergy);
    typedef std::vector<cv::ConstSizeMinBinaryHeap<int32_t>> vectorOfMinPQ;

    class CV_EXPORTS SeamCarver
    {
    public:
        SeamCarver(double marginEnergy = 390150.0) : marginEnergy(marginEnergy),
            numRows_(0),
            numColumns_(0),
            bottomRow_(0),
            rightColumn_(0),
            posInf_(std::numeric_limits<double>::max()),
            pixelEnergyCalculator_(marginEnergy)
        {}

        virtual ~SeamCarver() {}

        /**
         * @brief find and remove vertical seams
         * @param NumSeams: number of vertical seams to remove
         * @param img: input image
         * @param outImg: output paramter
         * @param computeEnergy: pointer to a user-defined energy function. If one is not provided,
         *      internal one will be used
         * @return bool: indicates whether seam removal was successful or not
         */
        virtual void findAndRemoveVerticalSeams(int32_t numSeams,
                                                const cv::Mat& img,
                                                cv::Mat& outImg,
                                                cv::energyFunc computeEnergyFn = nullptr);

    protected:
        /**
         * @brief find vertical seams for later removal
         * @param numSeams: number of seams to discover for removal
         * @param pixelEnergy: calculated pixel energy of image
         * @param outDiscoveredSeams: output parameter (vector of priority queues)
         * @return bool: indicates success
         */
        virtual void findVerticalSeams(int32_t numSeams, vector<vector<double>>& pixelEnergy,
                                       vectorOfMinPQ& outDiscoveredSeams);

        /**
        * @brief calculates the energy required to reach bottom row
        * @param pixelEnergy: calculated pixel energy of image
        * @param outTotalEnergyTo: cumulative energy to reach pixel
        * @param outColumnTo: columnn of the pixel in the row above to get to every pixel
        */
        virtual void calculateCumulativeVerticalPathEnergy(
            const vector<vector<double>>& pixelEnergy,
            vector<vector<double>>& outTotalEnergyTo,
            vector<vector<int32_t>>& outColumnTo);

        /**
         * @brief remove vertical seam from img given by column locations stored in seam
         * @param bgr image separate into 3 channels (BLUE GREEN RED)
         * @param seams vector of priority queues that hold the columns for the pixels to remove
         *              for each row, where the index into the vector is the row number
         */
        virtual void removeVerticalSeams(vector<cv::Mat>& bgr, vectorOfMinPQ& seams);

        // vector to store pixels that have been previously markedPixels for removal
        // will ignore these markedPixels pixels when searching for a new seam
        vector<vector<bool>> markedPixels;

        // default energy at the borders of the image
        const double marginEnergy;

        int32_t numRows_;
        int32_t numColumns_;
        int32_t bottomRow_;
        int32_t rightColumn_;
        double posInf_;

        cv::PixelEnergy2D pixelEnergyCalculator_;
    };

}

#endif