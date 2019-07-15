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

#include "opencv2/seamcarver/computeenergystage.hpp"

#include <thread>

#include "opencv2/seamcarver/seamcarverstagefactory.hpp"
#include "opencv2/seamcarver/seamcarverstagefactoryregistration.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"
#include "opencv2/seamcarver/pipelinequeuedata.hpp"

cv::ComputeEnergyStage::ComputeEnergyStage()
    : bDoRunThread_(false),
      bThreadIsStopped_(true),
      bIsInitialized_(false),
      status_lock_(status_mutex_, std::defer_lock)
{
}

cv::ComputeEnergyStage::~ComputeEnergyStage()
{
    doStopStage();

    // wait for thread to finish
    while (bThreadIsStopped_ == false)
        ;
}

void cv::ComputeEnergyStage::initialize(cv::Ptr<cv::PipelineQueueData> initData)
{
    if (bIsInitialized_ == false)
    {
        cv::PipelineQueueData* data = static_cast<cv::PipelineQueueData*>(initData.get());
        if (data != nullptr)
        {
            pipelineStage_ = data->pipeline_stage;
            p_input_queue_ = data->p_input_queue;
            p_output_queue_ = data->p_output_queue;
            p_input_queue_lock_ = data->p_input_queue_lock;
            p_output_queue_lock_ = data->p_output_queue_lock;
            bIsInitialized_ = true;
        }
    }
}

void cv::ComputeEnergyStage::runStage()
{
    if (bThreadIsStopped_ && bIsInitialized_)
    {
        status_lock_.lock();
        if (bThreadIsStopped_)
        {
            std::thread(&cv::ComputeEnergyStage::runThread, this).detach();
        }
        status_lock_.unlock();
    }
}

void cv::ComputeEnergyStage::runThread()
{
    bDoRunThread_ = true;

    if (bThreadIsStopped_)
    {
        status_lock_.lock();
        if (bThreadIsStopped_)
        {
            bThreadIsStopped_ = false;
        }
        status_lock_.unlock();
    }

    while (bDoRunThread_)
    {
        if (!p_input_queue_->empty())
        {
            // save the pointer for faster access
            VerticalSeamCarverData* data = p_input_queue_->front();

            // TODO run the calculator

            // move data to next queue
            p_input_queue_lock_->lock();
            p_output_queue_lock_->lock();
            p_output_queue_->emplace(data);
            p_input_queue_->pop();
            p_output_queue_lock_->unlock();
            p_input_queue_lock_->unlock();
        }
    }

    bThreadIsStopped_ = true;
}

bool cv::ComputeEnergyStage::isInitialized() const { return bIsInitialized_; }

void cv::ComputeEnergyStage::doStopStage() { bDoRunThread_ = false; }

void cv::ComputeEnergyStage::stopStage() { doStopStage(); }

namespace
{
cv::SeamCarverStageFactoryRegistration registerstage(cv::ComputeEnergyStage::this_shape_id_, []() {
    return static_cast<cv::SeamCarverStage*>(new cv::ComputeEnergyStage());
});
}
