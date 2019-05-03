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

        size_t keepoutRegionStartingRow = 10;
        size_t keepoutRegionStartingColumn = 5;
        size_t keepoutRegionWidth = 100;
        size_t keepoutRegionHeight = 500;

        cv::Mat image = cv::imread(IMG_PATH);

        cv::Mat outImg;

        TEST(VerticalSeamCarverSquareKeepout, CanOpenImage)
        {
            ASSERT_EQ(image.empty(), false);
        }

        TEST(VerticalSeamCarverSquareKeepout, DefaultCtor)
        {
            VerticalSeamCarverSquareKeepout vsck(initialMarginEnergy);

            EXPECT_EQ(vsck.areDimensionsInitialized(), false);
            EXPECT_EQ(vsck.isKeepoutRegionDefined(), false);

            vsck.setDimensions(image);

            EXPECT_EQ(vsck.areDimensionsInitialized(), true);

            vsck.setKeepoutRegion(keepoutRegionStartingRow,
                                  keepoutRegionStartingColumn,
                                  keepoutRegionWidth,
                                  keepoutRegionHeight);

            EXPECT_EQ(vsck.isKeepoutRegionDefined(), true);
        }

        TEST(VerticalSeamCarverSquareKeepout, DimsCtor)
        {
            VerticalSeamCarverSquareKeepout vsck(
                (size_t)image.rows,
                (size_t)image.cols,
                keepoutRegionStartingRow,
                keepoutRegionStartingColumn,
                keepoutRegionWidth,
                keepoutRegionHeight,
                initialMarginEnergy);

            EXPECT_EQ(vsck.areDimensionsInitialized(), true);
            EXPECT_EQ(vsck.isKeepoutRegionDefined(), true);
        }

        TEST(VerticalSeamCarverSquareKeepout, ImgCtor)
        {
            VerticalSeamCarverSquareKeepout vsck(
                image,
                keepoutRegionStartingRow,
                keepoutRegionStartingColumn,
                keepoutRegionWidth,
                keepoutRegionHeight,
                initialMarginEnergy);

            EXPECT_EQ(vsck.areDimensionsInitialized(), true);
            EXPECT_EQ(vsck.isKeepoutRegionDefined(), true);
        }

        TEST(VerticalSeamCarverSquareKeepout, CheckExceptions)
        {
            {
                VerticalSeamCarverSquareKeepout vsck(initialMarginEnergy);

                // expect an exception since keepout region can't be set if 
                // internal data/dimensions is uninitialized
                try
                {
                    vsck.setKeepoutRegion(0, 0, 0, 0);
                }
                catch (const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsInternal);
                }
            }

            {
                VerticalSeamCarverSquareKeepout vsck(initialMarginEnergy);

                // set internal data/dimensions using image
                vsck.setDimensions(image);

                // expect an exception since keepout region height is 0
                try
                {
                    vsck.setKeepoutRegion(0,
                                          0,
                                          1,
                                          0);
                }
                catch(const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
                }
            }

            {
                VerticalSeamCarverSquareKeepout vsck(initialMarginEnergy);

                // set internal data/dimensions using image
                vsck.setDimensions(image);

                // expect an exception since keepout region width is 0
                try
                {
                    vsck.setKeepoutRegion(0,
                                          0,
                                          0,
                                          1);
                }
                catch(const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
                }
            }

            {
                VerticalSeamCarverSquareKeepout vsck(initialMarginEnergy);

                // set internal data/dimensions using image
                vsck.setDimensions(image);

                // expect an exception since keepout region starting column is past the end
                try
                {
                    vsck.setKeepoutRegion(0,
                                          (size_t)image.cols,
                                          1,
                                          1);
                }
                catch(const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
                }
            }
        }

        TEST(VerticalSeamCarverSquareKeepout, CheckRunSeamRemoverThrows)
        {
            VerticalSeamCarverSquareKeepout vsck(initialMarginEnergy);
            size_t numSeamsToRemove = 1;

            try
            {
                vsck.runSeamRemover(numSeamsToRemove, image, outImg);
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsInternal);
            }
        }
    }
}