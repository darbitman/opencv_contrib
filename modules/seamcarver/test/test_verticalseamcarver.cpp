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

        TEST(VerticalSeamCarver, CanOpenImage)
        {
            ASSERT_EQ(img.empty(), false);
        }

        TEST(VerticalSeamCarver, DefaultCtor)
        {
            VerticalSeamCarver vSeamCarver(initialMarginEnergy);

            EXPECT_EQ(vSeamCarver.areDimensionsInitialized(), false);

            vSeamCarver.setDimensions(img);

            EXPECT_EQ(vSeamCarver.areDimensionsInitialized(), true);
        }

        TEST(VerticalSeamCarver, DimsCtor)
        {
            VerticalSeamCarver vSeamCarver(
                (size_t)img.rows,
                (size_t)img.cols,
                (size_t)initialMarginEnergy);
            
            EXPECT_EQ(vSeamCarver.areDimensionsInitialized(), true);
        }

        TEST(VerticalSeamCarver, ImgCtor)
        {
            VerticalSeamCarver vSeamCarver(img, initialMarginEnergy);
            
            EXPECT_EQ(vSeamCarver.areDimensionsInitialized(), true);
        }

        TEST(VerticalSeamCarver, CheckExceptions)
        {
            {
                VerticalSeamCarver vSeamCarver(initialMarginEnergy);

                try
                {
                    // try removing more columns than columns available
                    vSeamCarver.runSeamRemover((size_t)(img.cols + 1), img, outImg);
                }
                catch (const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
                }
            }

            {
                VerticalSeamCarver vSeamCarver(initialMarginEnergy);

                try
                {
                    // try setting the number of dimensions to 0
                    vSeamCarver.setDimensions(0, 1);
                }
                catch(const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
                }
            }

            {
                VerticalSeamCarver vSeamCarver(initialMarginEnergy);

                try
                {
                    // try setting the number of dimensions to 0
                    vSeamCarver.setDimensions(1, 0);
                }
                catch(const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
                }
            }

            {
                VerticalSeamCarver vSeamCarver(initialMarginEnergy);

                try
                {
                    // try setting dimensions based on an empty image
                    cv::Mat emptyImage;
                    vSeamCarver.setDimensions(emptyImage);
                }
                catch(const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
                }
            }

            {
                VerticalSeamCarver vSeamCarver(initialMarginEnergy);

                try
                {
                    // try setting a new pixel energy calculator
                    cv::Ptr<PixelEnergy2D>ptr;
                    vSeamCarver.setPixelEnergyCalculator(ptr);
                }
                catch(const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
                }
            }
        }

        TEST(VerticalSeamCarver, RemoveSingleVerticalSeam)
        {
            size_t numSeamsToRemove = 1;

            // initialize internal dimensions/data when calling method to remove seams
            {
                VerticalSeamCarver vSeamCarver(initialMarginEnergy);

                EXPECT_EQ(vSeamCarver.areDimensionsInitialized(), false);

                try
                {
                    // try removing a single vertical seam
                    vSeamCarver.runSeamRemover(numSeamsToRemove, img, outImg);
                    EXPECT_EQ(vSeamCarver.areDimensionsInitialized(), true);
                }
                catch (const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
                }

                EXPECT_EQ((size_t)outImg.rows, (size_t)img.rows);
                EXPECT_EQ((size_t)outImg.cols, (size_t)img.cols - numSeamsToRemove);
            }

            // initialize internal dimensions/data PRIOR to calling method to remove seams
            {
                VerticalSeamCarver vSeamCarver(initialMarginEnergy);

                EXPECT_EQ(vSeamCarver.areDimensionsInitialized(), false);

                try
                {
                    // set dimensions prior to removing vertical seam
                    vSeamCarver.setDimensions(img);

                    EXPECT_EQ(vSeamCarver.areDimensionsInitialized(), true);

                    // try removing a single vertical seam
                    vSeamCarver.runSeamRemover(numSeamsToRemove, img, outImg);
                }
                catch (const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
                }

                EXPECT_EQ((size_t)outImg.rows, (size_t)img.rows);
                EXPECT_EQ((size_t)outImg.cols, (size_t)img.cols - numSeamsToRemove);
            }

            // use the image to initialize internal dimensions/data
            {
                VerticalSeamCarver vSeamCarver(img, initialMarginEnergy);

                EXPECT_EQ(vSeamCarver.areDimensionsInitialized(), true);

                try
                {
                    vSeamCarver.runSeamRemover(numSeamsToRemove, img, outImg);
                }
                catch (const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
                }

                EXPECT_EQ((size_t)outImg.rows, (size_t)img.rows);
                EXPECT_EQ((size_t)outImg.cols, (size_t)img.cols - numSeamsToRemove);
            }

            // use image dimensions to initialize internal dimensions/data
            {
                VerticalSeamCarver vSeamCarver(
                    (size_t)img.rows,
                    (size_t)img.cols,
                    initialMarginEnergy);

                EXPECT_EQ(vSeamCarver.areDimensionsInitialized(), true);

                try
                {
                    vSeamCarver.runSeamRemover(numSeamsToRemove, img, outImg);
                }
                catch (const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
                }

                EXPECT_EQ((size_t)outImg.rows, (size_t)img.rows);
                EXPECT_EQ((size_t)outImg.cols, (size_t)img.cols - numSeamsToRemove);
            }

            // use image to initialize internal dimensions/data
            // then change them to mismatch the image
            // verfiy if the runSeamRemover method adjusts internal dimensions/data to match image
            {
                VerticalSeamCarver vSeamCarver(img, initialMarginEnergy);

                EXPECT_EQ(vSeamCarver.areDimensionsInitialized(), true);

                vSeamCarver.setDimensions((size_t)img.rows + 1, (size_t)img.cols + 1);

                EXPECT_EQ(vSeamCarver.areDimensionsInitialized(), true);

                try
                {
                    vSeamCarver.runSeamRemover(numSeamsToRemove, img, outImg);
                }
                catch (const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsBadArg);
                }

                EXPECT_EQ((size_t)outImg.rows, (size_t)img.rows);
                EXPECT_EQ((size_t)outImg.cols, (size_t)img.cols - numSeamsToRemove);
            }
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

            ///home/dmitriy/opencv_dev/opencvDebugDisplay d;
            //d.displayMatrix(outImg);
        }

        TEST(VerticalSeamCarver, NonDefaultPixelEnergyCalculator)
        {
            cv::Ptr<PixelEnergy2D> pNewGradientPixelEnergy =
                cv::makePtr<GradientPixelEnergy2D>(initialMarginEnergy);

            VerticalSeamCarver vSeamCarver(img, initialMarginEnergy, pNewGradientPixelEnergy);

            EXPECT_EQ(pNewGradientPixelEnergy.use_count(), 2);
        }

        // TODO need tests for custom pixelEnergyCalculator
    }
}
