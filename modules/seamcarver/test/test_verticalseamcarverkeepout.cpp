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

        cv::Mat img = cv::imread("../../../opencv_contrib/modules/seamcarver/test/eagle.jpg");

        cv::Mat outImg;

        TEST(VerticalSeamCarverKeepout, CanOpenImage)
        {
            ASSERT_EQ(img.empty(), false);
        }

        TEST(VerticalSeamCarverKeepout, DefaultCtor)
        {
            VerticalSeamCarverSquareKeepout vsck(initialMarginEnergy);

            EXPECT_EQ(vsck.areDimensionsInitialized(), false);
            EXPECT_EQ(vsck.isKeepoutRegionDefined(), false);

            vsck.setDimensions(img);

            EXPECT_EQ(vsck.areDimensionsInitialized(), true);

            vsck.setKeepoutRegion(keepoutRegionStartingRow,
                                  keepoutRegionStartingColumn,
                                  keepoutRegionWidth,
                                  keepoutRegionHeight);

            EXPECT_EQ(vsck.isKeepoutRegionDefined(), true);
        }

        TEST(VerticalSeamCarverKeepout, DimsCtor)
        {
            VerticalSeamCarverSquareKeepout vsck((size_t)img.rows,
                                           (size_t)img.cols,
                                           keepoutRegionStartingRow,
                                           keepoutRegionStartingColumn,
                                           keepoutRegionWidth,
                                           keepoutRegionHeight,
                                           initialMarginEnergy);

            EXPECT_EQ(vsck.areDimensionsInitialized(), true);
            EXPECT_EQ(vsck.isKeepoutRegionDefined(), true);
        }

        TEST(VerticalSeamCarverKeepout, ImgCtor)
        {
            VerticalSeamCarverSquareKeepout vsck(img,
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
            VerticalSeamCarverSquareKeepout vsck(initialMarginEnergy);

            try
            {
                vsck.setKeepoutRegion((size_t)img.rows - 1,
                    (size_t)img.cols - 1,
                                      0,
                                      0);
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
            }

            vsck.setKeepoutRegion(0, 0, keepoutRegionWidth, keepoutRegionHeight);
            vsck.setDimensions(img);

            try
            {
                vsck.setKeepoutRegion(
                    (size_t)img.rows,
                    (size_t)img.cols,
                    keepoutRegionWidth,
                    eepoutRegionHeight
                );
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
            }
        }

        TEST(VerticalSeamCarverSquareKeepout, CheckRunSeamRemoverThrows)
        {
            VerticalSeamCarverSquareKeepout vsck(initialMarginEnergy);
            size_t numSeamsToRemove = 1;

            try
            {
                vsck.runSeamRemover(numSeamsToRemove, img, outImg);
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsInternal);
            }
        }
    }
}