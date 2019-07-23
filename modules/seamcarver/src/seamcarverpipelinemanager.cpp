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

#include "opencv2/seamcarver/pipelineconfigurationtype.hpp"
#include "opencv2/seamcarver/pipelinequeuedata.hpp"
#include "opencv2/seamcarver/seamcarverpipelineinterface.hpp"
#include "opencv2/seamcarver/seamcarverstage.hpp"
#include "opencv2/seamcarver/seamcarverstagefactory.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::SeamCarverPipelineManager::SeamCarverPipelineManager(
    cv::PipelineConfigurationType configurationType)
    : bPipelineStagesCreated_(false),
      bPipelineDataInitialized_(false),
      bPipelineStagesInitialized_(false),
      bIsInitialized_(false),
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
        initializePipelineData();
        initializePipelineStages();
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
            if (pipelineStages_[stage] != nullptr)
            {
                pipelineStages_[stage]->runStage();
            }
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

bool cv::SeamCarverPipelineManager::arePipelineStagesCreated() const
{
    return bPipelineStagesCreated_;
}

bool cv::SeamCarverPipelineManager::isPipelineDataInitialized() const
{
    return bPipelineDataInitialized_;
}

bool cv::SeamCarverPipelineManager::arePipelineStagesInitialized() const
{
    return bPipelineStagesInitialized_;
}

bool cv::SeamCarverPipelineManager::arePipelineStagesRunning() const
{
    return bArePipelineStagesRunning_;
}

void cv::SeamCarverPipelineManager::createPipeline()
{
    if (!bPipelineStagesCreated_)
    {
        pipelineStages_.resize(cv::PipelineStages::LAST_STAGE);
        SeamCarverStageFactory& factory = SeamCarverStageFactory::instance();
        switch (pipelineConfigurationType_)
        {
            case cv::PipelineConfigurationType::VERTICAL_DEFAULT:
                for (int32_t stage = cv::PipelineStages::STAGE_0;
                     stage < cv::PipelineStages::LAST_STAGE; ++stage)
                {
                    pipelineStages_[stage] = factory.createStage(
                        cv::PipelineConfigurationType::VERTICAL_DEFAULT | stage);
                }
        }
        bPipelineStagesCreated_ = true;
    }
}

void cv::SeamCarverPipelineManager::initializePipelineData()
{
    if (!bPipelineDataInitialized_)
    {
        queues_.resize(cv::PipelineStages::NUM_STAGES);

        // allocate the vector of pointers to queues that hold the data to process for each stage
        // and to locks that own mutexes_
        for (int32_t stage = cv::PipelineStages::STAGE_0; stage < cv::PipelineStages::NUM_STAGES;
             ++stage)
        {
            queues_[stage] = cv::makePtr<cv::SharedQueue<VerticalSeamCarverData*>>();
        }

        bPipelineDataInitialized_ = true;
    }
}

void cv::SeamCarverPipelineManager::initializePipelineStages()
{
    if (!bPipelineStagesInitialized_)
    {
        for (int32_t stage = cv::PipelineStages::STAGE_0; stage < cv::PipelineStages::LAST_STAGE;
             ++stage)
        {
            if (pipelineStages_[stage] != nullptr)
            {
                cv::Ptr<cv::PipelineQueueData> pNewData = std::make_shared<cv::PipelineQueueData>();
                pNewData->p_input_queue = queues_[stage];
                pNewData->p_output_queue = queues_[stage + 1];
                pNewData->pipeline_stage = (cv::PipelineStages)stage;

                pipelineStages_[stage]->initialize(pNewData);
            }
        }
        bPipelineStagesInitialized_ = true;
    }
}

cv::Ptr<cv::SeamCarverPipelineInterface> cv::SeamCarverPipelineManager::createPipelineInterface()
{
    cv::Ptr<cv::PipelineQueueData> pNewData = std::make_shared<cv::PipelineQueueData>();
    pNewData->p_input_queue = queues_[PipelineStages::STAGE_0];
    pNewData->p_output_queue = queues_[PipelineStages::LAST_STAGE];

    cv::Ptr<cv::SeamCarverPipelineInterface> pPipelineInterface =
        std::make_shared<cv::SeamCarverPipelineInterface>(pNewData);

    return pPipelineInterface;
}
