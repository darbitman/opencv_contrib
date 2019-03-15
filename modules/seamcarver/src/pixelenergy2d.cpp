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

#include "opencv2/seamcarver/pixelenergy2d.hpp"
#include <thread>
using std::vector;

cv::PixelEnergy2D::PixelEnergy2D(double marginEnergy)
{
    try
    {
        setMarginEnergy(marginEnergy);
    }
    catch (...)
    {
        throw;
    }
}

cv::PixelEnergy2D::PixelEnergy2D(size_t numColumns,
                                 size_t numRows,
                                 size_t numColorChannels,
                                 double marginEnergy)
{
    try
    {
        setDimensions(numColumns, numRows);
        setMarginEnergy(marginEnergy);
        setNumColorChannels(numColorChannels);
    }
    catch (...)
    {
        throw;
    }
}

cv::PixelEnergy2D::PixelEnergy2D(const cv::Mat& image, double marginEnergy)
{
    try
    {
        setDimensions((size_t)image.cols, (size_t)image.rows);
        setMarginEnergy(marginEnergy);
        setNumColorChannels((size_t)image.channels());
    }
    catch (...)
    {
        throw;
    }
}

cv::PixelEnergy2D::~PixelEnergy2D()
{}

void cv::PixelEnergy2D::setMarginEnergy(double marginEnergy)
{
    if (marginEnergy < 0.0)
    {
        CV_Error(Error::Code::StsInternal, "Can't set negative margin energy");
    }

    marginEnergy_ = marginEnergy;
}

void cv::PixelEnergy2D::setDimensions(size_t numColumns, size_t numRows)
{
    if (numColumns <= 0 || numRows <= 0)
    {
        CV_Error(Error::Code::StsInternal,
                 "PixelEnergy2D::setDimensions() failed due to invalid input dimensions");
    }

    imageDimensions_.numColumns_ = numColumns;
    imageDimensions_.numRows_ = numRows;
    bDimensionsInitialized = true;
}

void cv::PixelEnergy2D::setNumColorChannels(size_t numColorChannels)
{
    if (!(numColorChannels == 1 || numColorChannels == 3))
    {
        CV_Error(Error::Code::StsInternal,
                 "PixelEnergy2D::setDimensions() failed due to incorrect color channels");
    }

    numColorChannels_ = numColorChannels;
    bNumColorChannelsInitialized = true;
}

double cv::PixelEnergy2D::getMarginEnergy() const
{
    return marginEnergy_;
}

cv::ImageDimensionStruct cv::PixelEnergy2D::getDimensions() const
{
    if (!bDimensionsInitialized)
    {
        CV_Error(Error::Code::StsInternal,
                 "Uninitialized internal dimensions");
    }

    return imageDimensions_;
}

size_t cv::PixelEnergy2D::getNumColorChannels() const
{
    if (!bNumColorChannelsInitialized)
    {
        CV_Error(Error::Code::StsInternal,
                 "Uninitialized number of color channels");
    }

    return numColorChannels_;
}

bool cv::PixelEnergy2D::areDimensionsSet() const
{
    return bDimensionsInitialized;
}

bool cv::PixelEnergy2D::isNumColorChannelsSet() const
{
    return bNumColorChannelsInitialized;
}

void cv::PixelEnergy2D::calculatePixelEnergy(const cv::Mat& image,
                                             vector<vector<double>>& outPixelEnergy)
{
    // check for empty image
    if (image.empty())
    {
        CV_Error(Error::Code::StsBadArg,
                 "PixelEnergy2D::calculatePixelEnergyForEveryRow() failed due to empty image");
    }

    if (!bDimensionsInitialized)
    {
        setDimensions((size_t)image.cols, (size_t)image.rows);
    }

    if (!bNumColorChannelsInitialized)
    {
        setNumColorChannels((size_t)image.channels());
    }

    if (imageDimensions_.numColumns_ != (size_t)image.cols)
    {
        imageDimensions_.numColumns_ = (size_t)image.cols;
    }

    if (imageDimensions_.numRows_ != (size_t)image.rows)
    {
        imageDimensions_.numRows_ = (size_t)image.rows;
    }

    // ensure outPixelEnergy has the right dimensions (resize locally if necessary)
    if (!(outPixelEnergy.size() == (size_t)imageDimensions_.numRows_))
    {
        outPixelEnergy.resize(imageDimensions_.numRows_);
    }
    for (size_t row = 0; row < imageDimensions_.numRows_; row++)
    {
        if (!(outPixelEnergy[row].size() == (size_t)imageDimensions_.numColumns_))
        {
            outPixelEnergy[row].resize(imageDimensions_.numColumns_);
        }
    }

    // if more columns, split calculation into 2 operations to calculate for every row
    if (imageDimensions_.numColumns_ >= imageDimensions_.numRows_)
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
    }

    if (threadExceptionPtr)
    {
        std::rethrow_exception(threadExceptionPtr);
    }
}

void cv::PixelEnergy2D::calculatePixelEnergyForEveryRow(const cv::Mat& image,
                                                        vector<vector<double>>& outPixelEnergy,
                                                        bool bDoOddColumns)
{
    try
    {
        size_t bottomRow = imageDimensions_.numRows_ - 1;
        size_t rightColumn = imageDimensions_.numColumns_ - 1;

        vector<cv::Mat> imageByChannel;
        imageByChannel.resize(numColorChannels_);

        // if color channels use cv::split
        // otherwise if grayscale use cv::extractChannel
        if (numColorChannels_ == 3)
        {
            cv::split(image, imageByChannel);
        }
        else if (numColorChannels_ == 1)
        {
            cv::extractChannel(image, imageByChannel[0], 0);
        }
        else
        {
            CV_Error(Error::Code::StsInternal,
                     "PixelEnergy2D::calculatePixelEnergyForEveryRow() failed due to incorrect \
                 number of channels");
        }

        // Establish vectors whose size is equal to the number of channels
        // Two vectors used to compute X gradient
          // Don't need them for Y since we are only caching the columns
          // We can just access the pixel values above/below directly to compute the delta
        // TODO replace vectors with a multidimensional vector
        vector<double> xDirection2;
        vector<double> xDirection1;

        xDirection2.resize(numColorChannels_);
        xDirection1.resize(numColorChannels_);

        vector<double> deltaXDirection;
        vector<double> deltaYDirection;

        deltaXDirection.resize(numColorChannels_);
        deltaYDirection.resize(numColorChannels_);

        double deltaSquareX = 0.0;
        double deltaSquareY = 0.0;

        size_t column = 0;
        // compute energy for every row
        // do odd columns and even columns separately in order to leverage cached values to prevent
            // multiple memory accesses
        for (size_t row = 0; row < imageDimensions_.numRows_; row++)
        {
            /***** ODD COLUMNS *****/
            if (bDoOddColumns)
            {
                // initialize starting column
                column = 1;

                // initialize color values to the left of current pixel
                for (size_t channel = 0; channel < numColorChannels_; channel++)
                {
                    xDirection1[channel] = imageByChannel[channel].at<uchar>(row, column - 1);
                }

                // Compute energy of odd columns
                for (/* column already initialized */;
                     column < imageDimensions_.numColumns_; column += 2)
                {
                    if (row == 0 || column == 0 || row == bottomRow || column == rightColumn)
                    {
                        outPixelEnergy[row][column] = marginEnergy_;
                    }
                    else
                    {
                        // Reset gradients from previous calculation
                        deltaSquareX = 0.0;
                        deltaSquareY = 0.0;

                        // For all channels:
                          // Compute gradients
                          // Compute overall energy by summing both X and Y gradient
                        for (size_t channel = 0;
                             channel < numColorChannels_; channel++)
                        {
                            // get new values to the right
                            xDirection2[channel] = imageByChannel[channel].at<uchar>(row,
                                                                                     column + 1);

                            deltaXDirection[channel] = xDirection2[channel] - xDirection1[channel];

                            deltaSquareX += deltaXDirection[channel] * deltaXDirection[channel];

                            deltaYDirection[channel] =
                                imageByChannel[channel].at<uchar>(row + 1, column) -
                                imageByChannel[channel].at<uchar>(row - 1, column);

                            deltaSquareY += deltaYDirection[channel] * deltaYDirection[channel];

                            // shift color values to the left
                            xDirection1[channel] = xDirection2[channel];
                        }
                        outPixelEnergy[row][column] = deltaSquareX + deltaSquareY;
                    }
                }
            }
            /***** EVEN COLUMNS *****/
            else
            {
                // initialize starting column
                column = 0;

                // initialize color values to the right of current pixel
                for (size_t Channel = 0; Channel < numColorChannels_; Channel++)
                {
                    xDirection2[Channel] = imageByChannel[Channel].at<uchar>(row, column + 1);
                }

                // Compute energy of odd columns
                for (/* column already initialized */;
                     column < imageDimensions_.numColumns_; column += 2)
                {
                    if (row == 0 || column == 0 || row == bottomRow || column == rightColumn)
                    {
                        outPixelEnergy[row][column] = marginEnergy_;
                    }
                    else
                    {
                        // Reset gradient from previous calculation
                        deltaSquareX = 0.0;
                        deltaSquareY = 0.0;

                        // For all channels:
                          // Compute gradients
                          // Compute overall energy by summing both X and Y gradient
                        for (size_t channel = 0;
                             channel < numColorChannels_; channel++)
                        {
                            // shift color values to the left
                            xDirection1[channel] = xDirection2[channel];

                            // get new values to the right
                            xDirection2[channel] = imageByChannel[channel].at<uchar>(row,
                                                                                     column + 1);
                            deltaXDirection[channel] = xDirection2[channel] - xDirection1[channel];

                            deltaSquareX += deltaXDirection[channel] * deltaXDirection[channel];

                            deltaYDirection[channel] =
                                imageByChannel[channel].at<uchar>(row + 1, column) -
                                imageByChannel[channel].at<uchar>(row - 1, column);

                            deltaSquareY += deltaYDirection[channel] * deltaYDirection[channel];
                        }
                        outPixelEnergy[row][column] = deltaSquareX + deltaSquareY;
                    }
                }
            }
        }
    }
    catch (...)
    {
        // save first instance of exception only
        threadExceptionPtrMutex.lock();

        if (threadExceptionPtr == nullptr)
        {
            threadExceptionPtr = std::current_exception();
        }

        threadExceptionPtrMutex.unlock();
    }
}

void cv::PixelEnergy2D::calculatePixelEnergyForEveryColumn(const cv::Mat& image,
                                                           vector<vector<double>>& outPixelEnergy,
                                                           bool bDoOddRows)
{
    try
    {
        size_t bottomRow = imageDimensions_.numRows_ - 1;
        size_t rightColumn = imageDimensions_.numColumns_ - 1;

        vector<cv::Mat> imageByChannel;
        imageByChannel.resize(numColorChannels_);

        // if color channels use cv::split
        // otherwise if grayscale use cv::extractChannel
        if (numColorChannels_ == 3)
        {
            cv::split(image, imageByChannel);
        }
        else if (numColorChannels_ == 1)
        {
            cv::extractChannel(image, imageByChannel[0], 0);
        }
        else
        {
            CV_Error(Error::Code::StsInternal,
                     "PixelEnergy2D::calculatePixelEnergyForEveryColumn() failed due to incorrect \
                      number of channels");
        }

        // Establish vectors whose size is equal to the number of channels
        // Two vectors used to compute X gradient
          // Don't need them for Y since we are only caching the columns
          // We can just access the pixel values above/below directly to compute the delta
        // TODO replace vectors with a multidimensional vector
        vector<double> yDirection2;
        vector<double> yDirection1;

        yDirection2.resize(numColorChannels_);
        yDirection1.resize(numColorChannels_);

        vector<double> deltaXDirection;
        vector<double> deltaYDirection;

        deltaXDirection.resize(numColorChannels_);
        deltaYDirection.resize(numColorChannels_);

        double deltaSquareX = 0.0;
        double deltaSquareY = 0.0;

        size_t row = 0;
        // compute energy for every column
        // do odd rows and even rows separately in order to leverage cached values
            // to prevent multiple memory accesses
        for (size_t column = 0; column < imageDimensions_.numColumns_; column++)
        {
            /***** ODD ROWS *****/
            if (bDoOddRows)
            {
                // initialize starting row
                row = 1;

                // initialize color values above the current pixel
                for (size_t channel = 0; channel < numColorChannels_; channel++)
                {
                    yDirection1[channel] = imageByChannel[channel].at<uchar>(row - 1, column);
                }

                // Compute energy of odd rows
                for (/* row was already initialized */; row < imageDimensions_.numRows_; row += 2)
                {
                    if (row == 0 || column == 0 || row == bottomRow || column == rightColumn)
                    {
                        outPixelEnergy[row][column] = marginEnergy_;
                    }
                    else
                    {
                        // Reset gradients from previous calculation
                        deltaSquareX = 0.0;
                        deltaSquareY = 0.0;

                        // For all channels:
                          // Compute gradients
                          // Compute overall energy by summing both X and Y gradient
                        for (size_t channel = 0;
                             channel < numColorChannels_; channel++)
                        {
                            // get new values below the current pixel
                            yDirection2[channel] = imageByChannel[channel].at<uchar>(row + 1,
                                                                                     column);

                            deltaYDirection[channel] = yDirection2[channel] - yDirection1[channel];

                            deltaSquareY = deltaYDirection[channel] * deltaYDirection[channel];

                            deltaXDirection[channel] =
                                imageByChannel[channel].at<uchar>(row, column + 1) -
                                imageByChannel[channel].at<uchar>(row, column - 1);

                            deltaSquareX += deltaXDirection[channel] * deltaXDirection[channel];

                            // shift color values up
                            yDirection1[channel] = yDirection2[channel];
                        }
                        outPixelEnergy[row][column] = deltaSquareX + deltaSquareY;
                    }
                }
            }
            /***** EVEN ROWS *****/
            else
            {
                // initialize starting row
                row = 0;

                // initialize color values below the current pixel
                for (size_t channel = 0; channel < numColorChannels_; channel++)
                {
                    yDirection2[channel] = imageByChannel[channel].at<uchar>(row + 1, column);
                }

                // Compute energy of odd rows
                for (/* row was already initialized */; row < imageDimensions_.numRows_; row += 2)
                {
                    if (row == 0 || column == 0 || row == bottomRow || column == rightColumn)
                    {
                        outPixelEnergy[row][column] = marginEnergy_;
                    }
                    else
                    {
                        // Reset gradient from previous calculation
                        deltaSquareX = 0.0;
                        deltaSquareY = 0.0;

                        // For all channels:
                          // Compute gradients
                          // Compute overall energy by summing both X and Y gradient
                        for (size_t channel = 0;
                             channel < numColorChannels_; channel++)
                        {
                            // shift color values up
                            yDirection1[channel] = yDirection2[channel];

                            // get new values below the current pixel
                            yDirection2[channel] = imageByChannel[channel].at<uchar>(row + 1,
                                                                                     column);
                            deltaYDirection[channel] = yDirection2[channel] - yDirection1[channel];

                            deltaSquareY += deltaYDirection[channel] * deltaYDirection[channel];

                            deltaXDirection[channel] =
                                imageByChannel[channel].at<uchar>(row, column + 1) -
                                imageByChannel[channel].at<uchar>(row, column - 1);

                            deltaSquareX += deltaXDirection[channel] * deltaXDirection[channel];
                        }
                        outPixelEnergy[row][column] = deltaSquareX + deltaSquareY;
                    }
                }
            }
        }
    }
    catch (...)
    {
        // save first instance of exception
        threadExceptionPtrMutex.lock();

        if (threadExceptionPtr == nullptr)
        {
            threadExceptionPtr = std::current_exception();
        }

        threadExceptionPtrMutex.unlock();
    }
}