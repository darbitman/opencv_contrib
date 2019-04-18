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

#ifndef OPENCV_SEAMCARVER_VERTICALSEAMCARVERARBITRARYKEEPOUT_HPP
#define OPENCV_SEAMCARVER_VERTICALSEAMCARVERARBITRARYKEEPOUT_HPP

#include <opencv2/core.hpp>
#include "opencv2/seamcarver/verticalseamcarver.hpp"

namespace cv
{
    class CV_EXPORTS VerticalSeamCarverArbitraryKeepout : public VerticalSeamCarver
    {
    public:
        virtual ~VerticalSeamCarverArbitraryKeepout() {}

        /**
         * @brief run the vertical seam remover algorithm avoiding the keepout region
         * @param numSeams: number of vertical seams to remove
         * @param image: input image
         * @param outImage: output image parameter
         */
        virtual void runSeamRemover(size_t numSeams,
                                    const cv::Mat& image,
                                    cv::Mat& outImage) override;

        /**
         * @brief set the keepout region dimensions
         * @param keepoutRegion: 2D vector of pixels to avoid. Every row has a vector of the columns to mark
         */
        virtual void setKeepoutRegion(const std::vector<std::vector<size_t>>& keepoutRegion);

        /**
         * @brief returns true if a keepout region has been defined
         * @return bool
         */
        virtual bool isKeepoutRegionDefined() const;


        // Deleted/defaulted functions
        VerticalSeamCarverArbitraryKeepout(const VerticalSeamCarverArbitraryKeepout& rhs) = delete;
        VerticalSeamCarverArbitraryKeepout(const VerticalSeamCarverArbitraryKeepout&& rhs) = delete;
        virtual VerticalSeamCarverArbitraryKeepout& operator=(const VerticalSeamCarverArbitraryKeepout& rhs) = delete;
        virtual VerticalSeamCarverArbitraryKeepout& operator=(const VerticalSeamCarverArbitraryKeepout&& rhs) = delete;

    protected:
        /**
         * @brief reset vectors to their starting state
         */
        virtual void resetLocalVectors() override;

        /**
         * @brief marks pixels based on the local keepout region
         */
        virtual void setKeepoutRegionFromLocalData();

        // flag that indicates whether an arbitrarily-shaped keepout region has been set
        bool bArbitraryKeepoutRegionDefined = false;

        // store locations to avoid in seam carving
        // first dimension is a vector of vectors
        // 2nd dimension stores the actual column indices
        // i.e. to get the column for some row, just iterate over keepoutRegion_[row]
        std::vector<std::vector<size_t>> keepoutRegion_;
    };
}

#endif