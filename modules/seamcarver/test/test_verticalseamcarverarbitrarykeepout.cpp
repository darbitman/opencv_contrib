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

#include "test_precomp.hpp"
#include "opencv2/seamcarver/debugdisplay.hpp"

namespace opencv_test
{
    namespace
    {
        double initialMarginEnergy = 390150.0;

        cv::Mat img = cv::imread(IMG_PATH);
        
        cv::Mat outImg;

        std::vector<std::vector<size_t>> newKeepoutRegion;

        TEST(VerticalSeamCarverArbitraryKeepout, CanOpenImage)
        {
            ASSERT_EQ(img.empty(), false);
        }

        TEST(VerticalSeamCarverArbitraryKeepout, DefaultCtor)
        {
            VerticalSeamCarverArbitraryKeepout vsck(initialMarginEnergy);

            EXPECT_EQ(vsck.areDimensionsInitialized(), false);
            EXPECT_EQ(vsck.isKeepoutRegionDefined(), false);

            vsck.setDimensions(img);

            EXPECT_EQ(vsck.areDimensionsInitialized(), true);

            newKeepoutRegion.resize((size_t)img.rows);
            for(size_t row = 0; row < (size_t)img.rows; ++row)
            {
                for (size_t column = 10; column < 50; ++column)
                {
                    newKeepoutRegion[row].push_back(column);
                }
            }

            vsck.setKeepoutRegion(newKeepoutRegion);

            EXPECT_EQ(vsck.isKeepoutRegionDefined(), true);
        }

        TEST(VerticalSeamCarverArbitraryKeepout, DimsCtor)
        {
            VerticalSeamCarverArbitraryKeepout vsck(
                (size_t)img.rows,
                (size_t)img.cols,
                // keepout region defined in previous test
                newKeepoutRegion,
                initialMarginEnergy
            );

            EXPECT_EQ(vsck.areDimensionsInitialized(), true);
            EXPECT_EQ(vsck.isKeepoutRegionDefined(), true);
        }
    }

    TEST(VerticalSeamCarverArbitraryKeepout, ImgCtor)
    {
        VerticalSeamCarverArbitraryKeepout vsck(
            img,
            // keepout region defined in previous test
            newKeepoutRegion,
            initialMarginEnergy
        );

        EXPECT_EQ(vsck.areDimensionsInitialized(), true);
        EXPECT_EQ(vsck.isKeepoutRegionDefined(), true);
    }

    TEST(VerticalSeamCarverArbitraryKeepout, CheckExceptions)
    {
        VerticalSeamCarverArbitraryKeepout vsck(initialMarginEnergy);

        try
        {
            vsck.runSeamRemover(0, img, outImg);
        }
        catch (const cv::Exception& e)
        {
            EXPECT_EQ(e.code, cv::Error::Code::StsInternal);
        }
        
        try
        {
            // keepout region set in previous test
            vsck.setKeepoutRegion(newKeepoutRegion);

            vsck.runSeamRemover(img.cols + 1, img, outImg);
        }
        catch (const cv::Exception& e)
        {
            EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
        }
        
        try
        {
            std::vector<std::vector<size_t>> emptyVector;
            vsck.setKeepoutRegion(emptyVector);
        }
        catch (const cv::Exception& e)
        {
            EXPECT_EQ(e.code, cv::Error::StsBadArg);
        }
    }
}