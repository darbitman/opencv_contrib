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

#include "opencv2/seamcarver.hpp"
#include <thread>

cv::PixelEnergy2D::PixelEnergy2D(double marginEnergy)
{
    marginEnergy_ = marginEnergy;
}

cv::PixelEnergy2D::PixelEnergy2D(int32_t numColumns, int32_t numRows,
                                 int32_t numChannels, double marginEnergy)
{
    imageDimensions_.NumColumns_ = numColumns;
    imageDimensions_.NumRows_ = numRows;
    imageDimensions_.NumColorChannels_ = numChannels;
    marginEnergy_ = marginEnergy;
    bDimensionsInitialized = true;
}

cv::PixelEnergy2D::PixelEnergy2D(const cv::Mat& image, double marginEnergy)
{
    imageDimensions_.NumColumns_ = image.cols;
    imageDimensions_.NumRows_ = image.rows;
    imageDimensions_.NumColorChannels_ = image.channels();
    marginEnergy_ = marginEnergy;
    bDimensionsInitialized = true;
}

cv::PixelEnergy2D::PixelEnergy2D(const PixelEnergy2D& other)
{
    imageDimensions_.NumColumns_ = other.imageDimensions_.NumColumns_;
    imageDimensions_.NumRows_ = other.imageDimensions_.NumRows_;
    imageDimensions_.NumColorChannels_ = other.imageDimensions_.NumColorChannels_;
    marginEnergy_ = other.marginEnergy_;
    bDimensionsInitialized = other.bDimensionsInitialized;
}

cv::PixelEnergy2D::~PixelEnergy2D()
{}

double cv::PixelEnergy2D::getMarginEnergy() const
{
    return marginEnergy_;
}

void cv::PixelEnergy2D::setMarginEnergy(double marginEnergy)
{
    marginEnergy_ = marginEnergy;
}

cv::ImageDimensionStruct cv::PixelEnergy2D::getDimensions() const
{
    if (!bDimensionsInitialized)
    {
        // TODO throw exception
    }
    return imageDimensions_;
}

void cv::PixelEnergy2D::setDimensions(int32_t numColumns, int32_t numRows, int32_t numChannels)
{
    if (numColumns < 0 || numRows < 0)
    {
        // TODO throw exception
    }
    if (!(numChannels == 1 || numChannels == 3))
    {
        // TODO throw exception
        // TODO add enum class?
    }
    imageDimensions_.NumColumns_ = numColumns;
    imageDimensions_.NumRows_ = numRows;
    imageDimensions_.NumColorChannels_ = numChannels;
}

void cv::PixelEnergy2D::calculatePixelEnergy(const cv::Mat& image,
                                             vector<vector<double>>& outPixelEnergy)
{
    // TODO add threads
    // if more columns, split calculation into 2 operations to calculate for every row
    if (imageDimensions_.NumColumns_ >= imageDimensions_.NumRows_)
    {
        std::thread thread1(&cv::PixelEnergy2D::calculatePixelEnergyForEveryRow,
                            this, std::ref(image),
                            std::ref(outPixelEnergy),
                            true);
        std::thread thread2(&cv::PixelEnergy2D::calculatePixelEnergyForEveryRow,
                            this, std::ref(image),
                            std::ref(outPixelEnergy),
                            false);
        while (!thread1.joinable());
        thread1.join();
        while (!thread2.joinable());
        thread2.join();
        //calculatePixelEnergyForEveryRow(image, outPixelEnergy, true);
        //calculatePixelEnergyForEveryRow(image, outPixelEnergy, false);
    }
    // otherwise, if more rows, split calculation into 2 operations to calculate for every column
    else
    {
        std::thread thread1(&cv::PixelEnergy2D::calculatePixelEnergyForEveryColumn,
                            this, std::ref(image),
                            std::ref(outPixelEnergy),
                            true);
        std::thread thread2(&cv::PixelEnergy2D::calculatePixelEnergyForEveryColumn,
                            this, std::ref(image),
                            std::ref(outPixelEnergy),
                            false);
        while (!thread1.joinable());
        thread1.join();
        while (!thread2.joinable());
        thread2.join();
        //calculatePixelEnergyForEveryColumn(image, outPixelEnergy, true);
        //calculatePixelEnergyForEveryColumn(image, outPixelEnergy, false);
    }
}

void cv::PixelEnergy2D::calculatePixelEnergyForEveryRow(const cv::Mat& image,
                                                        vector<vector<double>>& outPixelEnergy,
                                                        bool bDoOddColumns)
{
    // ensure image is of the right size
    if (!(image.cols == imageDimensions_.NumColumns_ &&
          image.rows == imageDimensions_.NumRows_ &&
          image.channels() == imageDimensions_.NumColorChannels_))
    {
        // TODO throw exception
    }

    // ensure image has non-zero dimensions
    if (image.cols == 0 || image.rows == 0 || image.channels() == 0)
    {
        // TODO throw exception
    }

    // ensure outPixelEnergy has the right dimensions
    // if not, then resize locally
    if (!(outPixelEnergy.size() == (uint32_t)imageDimensions_.NumRows_) ||
        !(outPixelEnergy[0].size() == (uint32_t)imageDimensions_.NumColumns_))
    {
        outPixelEnergy.resize(imageDimensions_.NumRows_);
        for (int32_t Row = 0; Row < imageDimensions_.NumRows_; Row++)
        {
            outPixelEnergy[Row].resize(imageDimensions_.NumColumns_);
        }
    }

    int32_t BottomRow = imageDimensions_.NumRows_ - 1;
    int32_t RightColumn = imageDimensions_.NumColumns_ - 1;

    vector<cv::Mat> ImageByChannel;
    ImageByChannel.resize(imageDimensions_.NumColorChannels_);

    // if color channels use cv::split
    // otherwise if grayscale use cv::extractChannel
    if (imageDimensions_.NumColorChannels_ == numChannelsInColorImage_)
    {
        cv::split(image, ImageByChannel);
    }
    else if (imageDimensions_.NumColorChannels_ == 1)
    {
        cv::extractChannel(image, ImageByChannel[0], 0);
    }
    else
    {
        // TODO throw exception
    }

    // Establish vectors whose size is equal to the number of channels
    // Two vectors used to compute X gradient
      // Don't need them for Y since we are only caching the columns
      // We can just access the pixel values above/below directly to compute the delta
    // TODO replace vectors with a multidimensional vector
    vector<double> XDirection2;
    vector<double> XDirection1;

    XDirection2.resize(imageDimensions_.NumColorChannels_);
    XDirection1.resize(imageDimensions_.NumColorChannels_);

    vector<double> DeltaXDirection;
    vector<double> DeltaYDirection;

    DeltaXDirection.resize(imageDimensions_.NumColorChannels_);
    DeltaYDirection.resize(imageDimensions_.NumColorChannels_);

    double DeltaSquareX = 0.0;
    double DeltaSquareY = 0.0;

    int32_t Column = 0;
    // compute energy for every row
    // do odd columns and even columns separately in order to leverage cached values to prevent
        // multiple memory accesses
    for (int32_t Row = 0; Row < imageDimensions_.NumRows_; Row++)
    {
        /***** ODD COLUMNS *****/
        if (bDoOddColumns)
        {
            // initialize starting column
            Column = 1;

            // initialize color values to the left of current pixel
            for (int32_t Channel = 0; Channel < imageDimensions_.NumColorChannels_; Channel++)
            {
                XDirection1[Channel] = ImageByChannel[Channel].at<uchar>(Row, Column - 1);
            }

            // Compute energy of odd columns
            for (/* Column already initialized */;
                 Column < imageDimensions_.NumColumns_; Column += 2)
            {
                if (Row == 0 || Column == 0 || Row == BottomRow || Column == RightColumn)
                {
                    outPixelEnergy[Row][Column] = marginEnergy_;
                }
                else
                {
                    // Reset gradients from previous calculation
                    DeltaSquareX = 0.0;
                    DeltaSquareY = 0.0;

                    // For all channels:
                      // Compute gradients
                      // Compute overall energy by summing both X and Y gradient
                    for (int32_t Channel = 0;
                         Channel < imageDimensions_.NumColorChannels_; Channel++)
                    {
                        // get new values to the right
                        XDirection2[Channel] = ImageByChannel[Channel].at<uchar>(Row, Column + 1);

                        DeltaXDirection[Channel] = XDirection2[Channel] - XDirection1[Channel];

                        DeltaSquareX += DeltaXDirection[Channel] * DeltaXDirection[Channel];

                        DeltaYDirection[Channel] =
                            ImageByChannel[Channel].at<uchar>(Row + 1, Column) -
                            ImageByChannel[Channel].at<uchar>(Row - 1, Column);

                        DeltaSquareY += DeltaYDirection[Channel] * DeltaYDirection[Channel];

                        // shift color values to the left
                        XDirection1[Channel] = XDirection2[Channel];
                    }
                    outPixelEnergy[Row][Column] = DeltaSquareX + DeltaSquareY;
                }
            }
        }
        /***** EVEN COLUMNS *****/
        else
        {
            // initialize starting column
            Column = 0;

            // initialize color values to the right of current pixel
            for (int32_t Channel = 0; Channel < imageDimensions_.NumColorChannels_; Channel++)
            {
                XDirection2[Channel] = ImageByChannel[Channel].at<uchar>(Row, Column + 1);
            }

            // Compute energy of odd columns
            for (/* Column already initialized */;
                 Column < imageDimensions_.NumColumns_; Column += 2)
            {
                if (Row == 0 || Column == 0 || Row == BottomRow || Column == RightColumn)
                {
                    outPixelEnergy[Row][Column] = marginEnergy_;
                }
                else
                {
                    // Reset gradient from previous calculation
                    DeltaSquareX = 0.0;
                    DeltaSquareY = 0.0;

                    // For all channels:
                      // Compute gradients
                      // Compute overall energy by summing both X and Y gradient
                    for (int32_t Channel = 0;
                         Channel < imageDimensions_.NumColorChannels_; Channel++)
                    {
                        // shift color values to the left
                        XDirection1[Channel] = XDirection2[Channel];

                        // get new values to the right
                        XDirection2[Channel] = ImageByChannel[Channel].at<uchar>(Row, Column + 1);
                        DeltaXDirection[Channel] = XDirection2[Channel] - XDirection1[Channel];

                        DeltaSquareX += DeltaXDirection[Channel] * DeltaXDirection[Channel];

                        DeltaYDirection[Channel] =
                            ImageByChannel[Channel].at<uchar>(Row + 1, Column) -
                            ImageByChannel[Channel].at<uchar>(Row - 1, Column);

                        DeltaSquareY += DeltaYDirection[Channel] * DeltaYDirection[Channel];
                    }
                    outPixelEnergy[Row][Column] = DeltaSquareX + DeltaSquareY;
                }
            }
        }
    }
}

void cv::PixelEnergy2D::calculatePixelEnergyForEveryColumn(const cv::Mat& Image,
                                                           vector<vector<double>>& OutPixelEnergy,
                                                           bool bDoOddRows)
{
    // ensure image is of the right size
    if (!(Image.cols == imageDimensions_.NumColumns_ &&
          Image.rows == imageDimensions_.NumRows_ &&
          Image.channels() == imageDimensions_.NumColorChannels_))
    {
        // TODO throw exception
    }

    // ensure image has non-zero dimensions
    if (Image.cols == 0 || Image.rows == 0 || Image.channels() == 0)
    {
        // TODO throw exception
    }

    // ensure outPixelEnergy has the right dimensions
    // if not, then resize locally
    if (!(OutPixelEnergy.size() == (uint32_t)imageDimensions_.NumRows_) ||
        !(OutPixelEnergy[0].size() == (uint32_t)imageDimensions_.NumColumns_))
    {
        OutPixelEnergy.resize(imageDimensions_.NumRows_);
        for (int32_t Row = 0; Row < imageDimensions_.NumRows_; Row++)
        {
            OutPixelEnergy[Row].resize(imageDimensions_.NumColumns_);
        }
    }

    int32_t BottomRow = imageDimensions_.NumRows_ - 1;
    int32_t RightColumn = imageDimensions_.NumColumns_ - 1;

    vector<cv::Mat> ImageByChannel;
    ImageByChannel.resize(imageDimensions_.NumColorChannels_);

    // if color channels use cv::split
    // otherwise if grayscale use cv::extractChannel
    if (imageDimensions_.NumColorChannels_ == numChannelsInColorImage_)
    {
        cv::split(Image, ImageByChannel);
    }
    else if (imageDimensions_.NumColorChannels_ == 1)
    {
        cv::extractChannel(Image, ImageByChannel[0], 0);
    }
    else
    {
        // TODO throw exception
    }

    // Establish vectors whose size is equal to the number of channels
    // Two vectors used to compute X gradient
      // Don't need them for Y since we are only caching the columns
      // We can just access the pixel values above/below directly to compute the delta
    // TODO replace vectors with a multidimensional vector
    vector<double> YDirection2;
    vector<double> YDirection1;

    YDirection2.resize(imageDimensions_.NumColorChannels_);
    YDirection1.resize(imageDimensions_.NumColorChannels_);

    vector<double> DeltaXDirection;
    vector<double> DeltaYDirection;

    DeltaXDirection.resize(imageDimensions_.NumColorChannels_);
    DeltaYDirection.resize(imageDimensions_.NumColorChannels_);

    double DeltaSquareX = 0.0;
    double DeltaSquareY = 0.0;

    int32_t Row = 0;
    // compute energy for every column
    // do odd rows and even rows separately in order to leverage cached values
        // to prevent multiple memory accesses
    for (int32_t Column = 0; Column < imageDimensions_.NumColumns_; Column++)
    {
        /***** ODD ROWS *****/
        if (bDoOddRows)
        {
            // initialize starting row
            Row = 1;

            // initialize color values above the current pixel
            for (int32_t Channel = 0; Channel < imageDimensions_.NumColorChannels_; Channel++)
            {
                YDirection1[Channel] = ImageByChannel[Channel].at<uchar>(Row - 1, Column);
            }

            // Compute energy of odd rows
            for (/* Row was already initialized */; Row < imageDimensions_.NumRows_; Row += 2)
            {
                if (Row == 0 || Column == 0 || Row == BottomRow || Column == RightColumn)
                {
                    OutPixelEnergy[Row][Column] = marginEnergy_;
                }
                else
                {
                    // Reset gradients from previous calculation
                    DeltaSquareX = 0.0;
                    DeltaSquareY = 0.0;

                    // For all channels:
                      // Compute gradients
                      // Compute overall energy by summing both X and Y gradient
                    for (int32_t Channel = 0;
                         Channel < imageDimensions_.NumColorChannels_; Channel++)
                    {
                        // get new values below the current pixel
                        YDirection2[Channel] = ImageByChannel[Channel].at<uchar>(Row + 1, Column);

                        DeltaYDirection[Channel] = YDirection2[Channel] - YDirection1[Channel];

                        DeltaSquareY = DeltaYDirection[Channel] * DeltaYDirection[Channel];

                        DeltaXDirection[Channel] =
                            ImageByChannel[Channel].at<uchar>(Row, Column + 1) -
                            ImageByChannel[Channel].at<uchar>(Row, Column - 1);

                        DeltaSquareX += DeltaXDirection[Channel] * DeltaXDirection[Channel];

                        // shift color values up
                        YDirection1[Channel] = YDirection2[Channel];
                    }
                    OutPixelEnergy[Row][Column] = DeltaSquareX + DeltaSquareY;
                }
            }
        }
        /***** EVEN ROWS *****/
        else
        {
            // initialize starting row
            Row = 0;

            // initialize color values below the current pixel
            for (int32_t Channel = 0; Channel < imageDimensions_.NumColorChannels_; Channel++)
            {
                YDirection2[Channel] = ImageByChannel[Channel].at<uchar>(Row + 1, Column);
            }

            // Compute energy of odd rows
            for (/* Row was already initialized */; Row < imageDimensions_.NumRows_; Row += 2)
            {
                if (Row == 0 || Column == 0 || Row == BottomRow || Column == RightColumn)
                {
                    OutPixelEnergy[Row][Column] = marginEnergy_;
                }
                else
                {
                    // Reset gradient from previous calculation
                    DeltaSquareX = 0.0;
                    DeltaSquareY = 0.0;

                    // For all channels:
                      // Compute gradients
                      // Compute overall energy by summing both X and Y gradient
                    for (int32_t Channel = 0;
                         Channel < imageDimensions_.NumColorChannels_; Channel++)
                    {
                        // shift color values up
                        YDirection1[Channel] = YDirection2[Channel];

                        // get new values below the current pixel
                        YDirection2[Channel] = ImageByChannel[Channel].at<uchar>(Row + 1, Column);
                        DeltaYDirection[Channel] = YDirection2[Channel] - YDirection1[Channel];

                        DeltaSquareY += DeltaYDirection[Channel] * DeltaYDirection[Channel];

                        DeltaXDirection[Channel] =
                            ImageByChannel[Channel].at<uchar>(Row, Column + 1) -
                            ImageByChannel[Channel].at<uchar>(Row, Column - 1);

                        DeltaSquareX += DeltaXDirection[Channel] * DeltaXDirection[Channel];
                    }
                    OutPixelEnergy[Row][Column] = DeltaSquareX + DeltaSquareY;
                }
            }
        }
    }
}
