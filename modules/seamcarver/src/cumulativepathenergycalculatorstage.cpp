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

#include "opencv2/seamcarver/cumulativepathenergycalculatorstage.hpp"

#include <thread>

#include "opencv2/seamcarver/seamcarverstagefactory.hpp"
#include "opencv2/seamcarver/seamcarverstagefactoryregistration.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::CumulativePathEnergyCalculatorStage::CumulativePathEnergyCalculatorStage()
    : bDoRunThread_(false),
      bThreadIsStopped_(true),
      bIsInitialized_(false),
      status_lock_(status_mutex_, std::defer_lock)
{
}

cv::CumulativePathEnergyCalculatorStage::~CumulativePathEnergyCalculatorStage()
{
    doStopStage();

    // clear the queues
    while (!p_input_queue_->empty())
    {
        delete p_input_queue_->front();
        p_input_queue_->pop();
    }

    while (!p_output_queue_->empty())
    {
        delete p_output_queue_->front();
        p_output_queue_->pop();
    }

    // wait for thread to finish
    while (bThreadIsStopped_ == false)
        ;
}

void cv::CumulativePathEnergyCalculatorStage::initialize(cv::Ptr<PipelineQueueData> initData)
{
    if (bIsInitialized_ == false)
    {
        PipelineQueueData* data = initData.get();
        if (data != nullptr)
        {
            pipelineStage_ = data->pipeline_stage;
            p_input_queue_ = data->p_input_queue;
            p_output_queue_ = data->p_output_queue;
            bIsInitialized_ = true;
        }
    }
}

void cv::CumulativePathEnergyCalculatorStage::runStage()
{
    if (bThreadIsStopped_ && bIsInitialized_)
    {
        status_lock_.lock();
        if (bThreadIsStopped_)
        {
            std::thread(&cv::CumulativePathEnergyCalculatorStage::runThread, this).detach();
            bThreadIsStopped_ = false;
        }
        status_lock_.unlock();
    }
}

void cv::CumulativePathEnergyCalculatorStage::runThread()
{
    bDoRunThread_ = true;

    while (bDoRunThread_)
    {
        if (!p_input_queue_->empty())
        {
            // save the pointer for faster access
            VerticalSeamCarverData* data = p_input_queue_->front();

            calculateCumulativePathEnergy(data);

            // move data to next queue
            p_input_queue_->pop();
            p_output_queue_->push(data);
        }
    }

    bThreadIsStopped_ = true;
}

bool cv::CumulativePathEnergyCalculatorStage::isInitialized() const { return bIsInitialized_; }

void cv::CumulativePathEnergyCalculatorStage::doStopStage() { bDoRunThread_ = false; }

void cv::CumulativePathEnergyCalculatorStage::stopStage() { doStopStage(); }

void cv::CumulativePathEnergyCalculatorStage::calculateCumulativePathEnergy(
    VerticalSeamCarverData* data)
{
    // initialize top row
    for (size_t column = 0; column < data->numColumns_; column++)
    {
        // if previously markedPixels, set its energy to +INF
        if (data->markedPixels[0][column])
        {
            data->totalEnergyTo[0][column] = data->posInf_;
        }
        else
        {
            data->totalEnergyTo[0][column] = data->marginEnergy_;
        }
        data->columnTo[0][column] = -1;
    }

    // cache the total energy to the pixels up/left, directly above, and up/right
    //   instead of accessing memory for the same pixels
    // shift energy values to the left and access memory only once
    // SHIFT OPERATION:
    //   left/above <== directly above
    //   directly above <== right/above
    //   right/above = access new memory
    double energyUpLeft = data->posInf_;
    double energyUp = data->posInf_;
    double energyUpRight = data->posInf_;

    double minEnergy = data->posInf_;
    int32_t minEnergyColumn = -1;

    for (size_t row = 1; row < data->numRows_; row++)
    {
        energyUpLeft = data->posInf_;
        energyUp = data->totalEnergyTo[row - 1][0];
        energyUpRight = data->numColumns_ > 1 ? data->totalEnergyTo[row - 1][1] : data->posInf_;

        bool markedUpLeft = true;
        bool markedUp = data->markedPixels[row - 1][0];
        bool markedUpRight = data->numColumns_ > 1 ? data->markedPixels[row - 1][1] : true;

        // find minimum energy path from previous row to every pixel in the current row
        for (size_t column = 0; column < data->numColumns_; column++)
        {
            // initialize min energy to +INF and initialize the previous column to -1
            //   to set error state
            minEnergy = data->posInf_;
            minEnergyColumn = -1;

            // save some cycles by not doing any comparisons if the current pixel has been
            //      previously marked
            if (!data->markedPixels[row][column])
            {
                // check above
                if (!markedUp && energyUp < minEnergy)
                {
                    minEnergy = energyUp;
                    minEnergyColumn = column;
                }

                // check if right/above is min
                if (column < data->numColumns_ - 1)
                {
                    if (!markedUpRight && energyUpRight < minEnergy)
                    {
                        minEnergy = energyUpRight;
                        minEnergyColumn = column + 1;
                    }
                }

                // check if left/above is min
                if (column > 0)
                {
                    if (!markedUpLeft && energyUpLeft < minEnergy)
                    {
                        minEnergy = energyUpLeft;
                        minEnergyColumn = column - 1;
                    }
                }
            }

            // shift energy to the left
            energyUpLeft = energyUp;
            markedUpLeft = markedUp;
            energyUp = energyUpRight;
            markedUp = markedUpRight;

            // get markedPixels and totalEnergyTo data for pixels right/above
            if (data->numColumns_ > 1 && column < data->numColumns_ - 2)
            {
                energyUpRight = data->totalEnergyTo[row - 1][column + 2];
                markedUpRight = data->markedPixels[row - 1][column + 2];
            }

            // assign cumulative energy to current pixel and save the column of the parent pixel
            if (minEnergyColumn == -1)
            {
                // current pixel is unreachable from parent pixels since they are all markedPixels
                //   OR current pixel already markedPixels
                // set energy to reach current pixel to +INF
                data->totalEnergyTo[row][column] = data->posInf_;
            }
            else
            {
                data->totalEnergyTo[row][column] = minEnergy + data->pixelEnergy[row][column];
            }
            data->columnTo[row][column] = minEnergyColumn;
        }
    }
}

namespace
{
cv::SeamCarverStageFactoryRegistration registerstage(
    cv::CumulativePathEnergyCalculatorStage::this_shape_id_, []() {
        return static_cast<cv::SeamCarverStage*>(new cv::CumulativePathEnergyCalculatorStage());
    });
}