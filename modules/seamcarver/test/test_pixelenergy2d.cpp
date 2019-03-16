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
#include <vector>
#include "opencv2/seamcarver/debugdisplay.hpp"

namespace opencv_test
{
    namespace
    {
        double initialMarginEnergy = 390150.0;
        size_t initialNumColumns = 1920;
        size_t initialNumRows = 1080;
        size_t initialNumChannels = 3;

        cv::Mat img = cv::imread("eagle.jpg");

        TEST(GradientPixelEnergy2D, CanOpenImage)
        {
            EXPECT_EQ(img.empty(), false);
        }

        TEST(GradientPixelEnergy2D, DefaultCtor)
        {
            cv::GradientPixelEnergy2D pixelEnergyCalculator(initialMarginEnergy);

            EXPECT_EQ(pixelEnergyCalculator.areDimensionsSet(), false);
            EXPECT_EQ(pixelEnergyCalculator.isNumColorChannelsSet(), false);

            pixelEnergyCalculator.setDimensions(initialNumColumns, initialNumRows);
            pixelEnergyCalculator.setNumColorChannels(initialNumChannels);

            EXPECT_EQ(pixelEnergyCalculator.areDimensionsSet(), true);
            EXPECT_EQ(pixelEnergyCalculator.isNumColorChannelsSet(), true);

            cv::ImageDimensionStruct imageDimensions = pixelEnergyCalculator.getDimensions();

            EXPECT_EQ(initialMarginEnergy, pixelEnergyCalculator.getMarginEnergy());
            EXPECT_EQ(initialNumChannels, pixelEnergyCalculator.getNumColorChannels());
            EXPECT_EQ(initialNumColumns, imageDimensions.numColumns_);
            EXPECT_EQ(initialNumRows, imageDimensions.numRows_);
        }

        TEST(GradientPixelEnergy2D, DimsCtor)
        {
            cv::GradientPixelEnergy2D pixelEnergyCalculator(initialNumColumns,
                                                    initialNumRows,
                                                    initialNumChannels,
                                                    initialMarginEnergy);

            EXPECT_EQ(pixelEnergyCalculator.areDimensionsSet(), true);
            EXPECT_EQ(pixelEnergyCalculator.isNumColorChannelsSet(), true);

            cv::ImageDimensionStruct imageDimensions = pixelEnergyCalculator.getDimensions();

            EXPECT_EQ(initialMarginEnergy, pixelEnergyCalculator.getMarginEnergy());
            EXPECT_EQ(initialNumChannels, pixelEnergyCalculator.getNumColorChannels());
            EXPECT_EQ(initialNumColumns, imageDimensions.numColumns_);
            EXPECT_EQ(initialNumRows, imageDimensions.numRows_);
        }

        TEST(GradientPixelEnergy2D, ImgCtor)
        {
            cv::Mat testImage(initialNumRows, initialNumColumns, CV_8U);

            cv::GradientPixelEnergy2D pixelEnergyCalculator(testImage, initialMarginEnergy);

            EXPECT_EQ(pixelEnergyCalculator.areDimensionsSet(), true);
            EXPECT_EQ(pixelEnergyCalculator.isNumColorChannelsSet(), true);

            cv::ImageDimensionStruct imageDimensions = pixelEnergyCalculator.getDimensions();

            EXPECT_EQ(initialMarginEnergy, pixelEnergyCalculator.getMarginEnergy());
            EXPECT_EQ((size_t)testImage.channels(), pixelEnergyCalculator.getNumColorChannels());
            EXPECT_EQ(initialNumColumns, imageDimensions.numColumns_);
            EXPECT_EQ(initialNumRows, imageDimensions.numRows_);
        }

        TEST(GradientPixelEnergy2D, CheckingExceptions)
        {
            // setting negative margin energy
            try
            {
                cv::GradientPixelEnergy2D pixelEnergyCalculator;
                pixelEnergyCalculator.setMarginEnergy(-3.0);
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsInternal);
            }

            // check uninitialized dimensions
            try
            {
                cv::GradientPixelEnergy2D pixelEnergyCalculator;
                pixelEnergyCalculator.getDimensions();
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsInternal);
            }

            // check if number of color channels uninitialized
            try
            {
                cv::GradientPixelEnergy2D pixelEnergyCalculator;
                pixelEnergyCalculator.getNumColorChannels();
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsInternal);
            }

            // setting dimension of 0 size
            try
            {
                cv::GradientPixelEnergy2D pixelEnergyCalculator;
                pixelEnergyCalculator.setDimensions(0, 10);
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsInternal);
            }

            // setting dimension of 0 size
            try
            {
                cv::GradientPixelEnergy2D pixelEnergyCalculator;
                pixelEnergyCalculator.setDimensions(10, 0);
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsInternal);
            }

            // setting invalid number of color channels
            try
            {
                cv::GradientPixelEnergy2D pixelEnergyCalculator;
                pixelEnergyCalculator.setNumColorChannels(2);
            }
            catch (const cv::Exception& e)
            {
                EXPECT_EQ(e.code, cv::Error::Code::StsInternal);
            }

            // check if image is of the wrong dimensions
            {
                cv::Mat testImage(initialNumRows, initialNumColumns, CV_8U);

                cv::GradientPixelEnergy2D pixelEnergyCalculator(initialNumColumns - 1,
                                                        initialNumRows - 1,
                                                        testImage.channels());

                EXPECT_EQ(pixelEnergyCalculator.areDimensionsSet(), true);
                EXPECT_EQ(pixelEnergyCalculator.isNumColorChannelsSet(), true);

                vector<vector<double>> calculatedPixelEnergy;

                try
                {
                    pixelEnergyCalculator.calculatePixelEnergy(testImage, calculatedPixelEnergy);
                }
                catch (const cv::Exception& e)
                {
                    EXPECT_EQ(e.code, cv::Error::Code::StsInternal);
                }
            }
        }

        //TEST(GradientPixelEnergy2D, DisplayCalculatedEnergy)
        //{
        //    cv::GradientPixelEnergy2D pixelEnergyCalculator(img, initialMarginEnergy);

        //    cv::namedWindow("Original Image");
        //    cv::imshow("Original Image", img);

        //    std::vector<std::vector<double>> computedPixelEnergy;
        //    pixelEnergyCalculator.calculatePixelEnergy(img, computedPixelEnergy);

        //    cv::DebugDisplay d;
        //    d.Display2DVector<double>(computedPixelEnergy, initialMarginEnergy);
        //}
    }
}