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

#include "opencv2/seamcarver/baseseamcarverstage.hpp"

#include <thread>

#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::BaseSeamCarverStage::BaseSeamCarverStage() : bThreadIsRunning_(false), bIsInitialized_(false) {}

cv::BaseSeamCarverStage::~BaseSeamCarverStage()
{
    doStopStage();

    // clear the queues
    while (!pInputQueue_->empty())
    {
        delete pInputQueue_->getNext();
        pInputQueue_->removeNext();
    }

    while (!pOutputQueue_->empty())
    {
        delete pOutputQueue_->getNext();
        pOutputQueue_->removeNext();
    }

    // wait for thread to finish
    while (bThreadIsRunning_ == true)
        ;
}

void cv::BaseSeamCarverStage::initialize(cv::Ptr<cv::PipelineQueueData> initData)
{
    if (bIsInitialized_ == false)
    {
        PipelineQueueData* data = initData.get();
        if (data != nullptr)
        {
            pInputQueue_ = data->p_input_queue;
            pOutputQueue_ = data->p_output_queue;

            if (pInputQueue_ == nullptr || pOutputQueue_ == nullptr)
            {
                bIsInitialized_ = false;
            }
            else
            {
                bIsInitialized_ = true;
            }
        }
    }
}

void cv::BaseSeamCarverStage::runStage()
{
    if (bIsInitialized_ && !bThreadIsRunning_)
    {
        std::unique_lock<std::mutex> statusLock(statusMutex_);
        if (!bThreadIsRunning_)
        {
            statusLock.unlock();
            std::thread(&cv::BaseSeamCarverStage::runThread, this).detach();
        }
    }
}

void cv::BaseSeamCarverStage::stopStage() { doStopStage(); }

bool cv::BaseSeamCarverStage::isInitialized() const { return bIsInitialized_; }

bool cv::BaseSeamCarverStage::isRunning() const { return bThreadIsRunning_; }

void cv::BaseSeamCarverStage::runThread()
{
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    bThreadIsRunning_ = true;
    statusLock.unlock();

    while (bThreadIsRunning_)
    {
        if (!pInputQueue_->empty())
        {
            // save the pointer for faster access
            VerticalSeamCarverData* data = pInputQueue_->getNext();

            if (data != nullptr)
            {
                processData(data);

                // move data to next queue
                pInputQueue_->removeNext();
                pOutputQueue_->push(data);
            }
        }
    }

    statusLock.lock();
    bThreadIsRunning_ = false;
}

void cv::BaseSeamCarverStage::doStopStage()
{
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    bThreadIsRunning_ = false;
}

void cv::BaseSeamCarverStage::processData(VerticalSeamCarverData* data)
{
    // define in derived class
}
