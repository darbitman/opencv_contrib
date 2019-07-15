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

#include "opencv2/seamcarver/seamcarverpipelinemanager.hpp"

#include "opencv2/seamcarver/pipelinequeuedata.hpp"
#include "opencv2/seamcarver/seamcarverstage.hpp"
#include "opencv2/seamcarver/seamcarverstagefactory.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::SeamCarverPipelineManager::SeamCarverPipelineManager(
    cv::pipelineconfigurationtype::pipelineconfigurationtype configurationType)
    : bIsInitialized_(false),
      bArePipelineStagesRunning_(false),
      pipelineConfigurationType_(configurationType)
{
}

cv::SeamCarverPipelineManager::~SeamCarverPipelineManager() {}

void cv::SeamCarverPipelineManager::initialize()
{
    if (!bIsInitialized_)
    {
        createPipeline();
        // initializePipelineData();
        // initializeAndRunPipelineStages();
        bIsInitialized_ = true;
    }
}

void cv::SeamCarverPipelineManager::runPipelineStages()
{
    if (!bArePipelineStagesRunning_)
    {
        for (int32_t stage = cv::PipelineStages::STAGE_0; stage < cv::PipelineStages::LAST_STAGE;
             ++stage)
        {
            pipelineStages_[stage]->runStage();
        }
        bArePipelineStagesRunning_ = true;
    }
}

void cv::SeamCarverPipelineManager::stopPipelineStages()
{
    if (bArePipelineStagesRunning_)
    {
        for (int32_t stage = cv::PipelineStages::STAGE_0; stage < cv::PipelineStages::LAST_STAGE;
             ++stage)
        {
            pipelineStages_[stage]->stopStage();
        }
        bArePipelineStagesRunning_ = false;
    }
}

bool cv::SeamCarverPipelineManager::isInitialized() const { return bIsInitialized_; }

bool cv::SeamCarverPipelineManager::arePipelineStagesRunning() const
{
    return bArePipelineStagesRunning_;
}

void cv::SeamCarverPipelineManager::createPipeline()
{
    SeamCarverStageFactory& factory = SeamCarverStageFactory::instance();
    switch (pipelineConfigurationType_)
    {
        case cv::pipelineconfigurationtype::VERTICAL_DEFAULT:
            for (int32_t stage = cv::PipelineStages::STAGE_0;
                 stage < cv::PipelineStages::LAST_STAGE; ++stage)
            {
                pipelineStages_[stage] =
                    factory.createStage(cv::pipelineconfigurationtype::VERTICAL_DEFAULT | stage);
            }
            cv::PipelineQueueData data;
    }
}

void cv::SeamCarverPipelineManager::initializePipelineData(double marginEnergy)
{
    queues_.resize(cv::PipelineStages::NUM_STAGES);
    locks_.resize(cv::PipelineStages::NUM_STAGES);

    // allocate the vector of pointers to queues that hold the data to process for each stage
    // and to locks that own mutexes_
    for (int32_t stage = cv::PipelineStages::STAGE_0; stage < cv::PipelineStages::NUM_STAGES;
         ++stage)
    {
        queues_[stage] = cv::makePtr<std::queue<VerticalSeamCarverData*>>();
        locks_[stage] = cv::Ptr<std::unique_lock<std::mutex>>(
            new std::unique_lock<std::mutex>(mutexes_[stage], std::defer_lock));
    }

    // create initial storage data
    queues_[cv::PipelineStages::STAGE_0]->emplace(new VerticalSeamCarverData(marginEnergy));
}

void cv::SeamCarverPipelineManager::initializePipelineStages()
{
    for (int32_t stage = cv::PipelineStages::STAGE_0; stage < cv::PipelineStages::LAST_STAGE;
         ++stage)
    {
        cv::Ptr<cv::PipelineQueueData> pNewData = std::make_shared<cv::PipelineQueueData>();
        pNewData->p_input_queue = queues_[stage];
        pNewData->p_output_queue = queues_[stage + 1];
        pNewData->p_input_queue_lock = locks_[stage];
        pNewData->p_output_queue_lock = locks_[stage + 1];
        pNewData->pipeline_stage = (cv::PipelineStages)stage;
        pipelineStages_[stage]->initialize(pNewData);
    }
}

void cv::SeamCarverPipelineManager::createPipelineInterface() {}
