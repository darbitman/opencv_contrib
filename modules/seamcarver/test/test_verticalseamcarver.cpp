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

        cv::Mat img = cv::imread("../../../../opencv_contrib/modules/seamcarver/test/eagle.jpg");

        cv::Mat outImg;

        TEST(VerticalSeamCarver, CanOpenImage)
        {
            ASSERT_EQ(img.empty(), false);
        }

        TEST(VerticalSeamCarver, DefaultCtor)
        {
            VerticalSeamCarver vSeamCarver(initialMarginEnergy);
        }

        TEST(VerticalSeamCarver, DimsCtor)
        {
            VerticalSeamCarver vSeamCarver((size_t)img.rows,
                                           (size_t)img.cols,
                                           (size_t)initialMarginEnergy);
        }

        TEST(VerticalSeamCarver, ImgCtor)
        {
            VerticalSeamCarver vSeamCarver(img, initialMarginEnergy);
        }

        TEST(VerticalSeamCarver, CheckRunSeamRemoverThrows)
        {
            VerticalSeamCarver vSeamCarver(initialMarginEnergy);

            try
            {
                vSeamCarver.runSeamRemover((size_t)(img.cols + 1), img, outImg);
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
            }
        }

        TEST(VerticalSeamCarver, RemoveSingleVerticalSeam)
        {
            size_t numSeamsToRemove = 1;
            VerticalSeamCarver vSeamCarver(initialMarginEnergy);

            try
            {
                vSeamCarver.runSeamRemover(numSeamsToRemove, img, outImg);
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
            }
            EXPECT_EQ(outImg.rows, img.rows);
            EXPECT_EQ((size_t)outImg.cols, (size_t)img.cols - numSeamsToRemove);

            //DebugDisplay d;
            //d.displayMatrix(outImg);
        }

        TEST(VerticalSeamCarver, RemoveSingleVerticalSeamImgCtor)
        {
            size_t numSeamsToRemove = 1;
            VerticalSeamCarver vSeamCarver(img, initialMarginEnergy);

            try
            {
                vSeamCarver.runSeamRemover(numSeamsToRemove, img, outImg);
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
            }
            EXPECT_EQ(outImg.rows, img.rows);
            EXPECT_EQ((size_t)outImg.cols, (size_t)img.cols - numSeamsToRemove);
        }

        TEST(VerticalSeamCarver, RemoveMultipleVerticalSeams)
        {
            size_t numSeamsToRemove = 3;
            VerticalSeamCarver vSeamCarver(img, initialMarginEnergy);

            try
            {
                vSeamCarver.runSeamRemover(numSeamsToRemove, img, outImg);
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
            }
            EXPECT_EQ(outImg.rows, img.rows);
            EXPECT_EQ((size_t)outImg.cols, (size_t)img.cols - numSeamsToRemove);

            //DebugDisplay d;
            //d.displayMatrix(outImg);
        }
    }
}