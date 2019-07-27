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
#include "opencv2/seamcarver/seamcarverqueuemanager.hpp"
#include "opencv2/seamcarver/seamcarverstage.hpp"
#include "opencv2/seamcarver/seamcarverstagefactory.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::SeamCarverPipelineManager::SeamCarverPipelineManager(
    cv::PipelineConfigurationType configurationType)
    : bPipelineStagesCreated_(false),
      bPipelineQueuesCreated_(false),
      bPipelineStagesInitialized_(false),
      bIsInitialized_(false),
      bPipelineStagesRunning_(false),
      pipelineConfigurationType_(configurationType)
{
}

cv::SeamCarverPipelineManager::~SeamCarverPipelineManager() {}

void cv::SeamCarverPipelineManager::initialize()
{
    if (!bIsInitialized_)
    {
        createPipelineStages();
        createPipelineQueues();
        initializePipelineStages();
        bIsInitialized_ = true;
    }
}

cv::Ptr<cv::SeamCarverPipelineInterface> cv::SeamCarverPipelineManager::createPipelineInterface()
{
    cv::Ptr<cv::PipelineQueueData> pNewData = std::make_shared<cv::PipelineQueueData>();
    pNewData->p_input_queue = queues_[PipelineStages::STAGE_0];
    pNewData->p_output_queue = queues_[PipelineStages::NUM_COMPUTE_STAGES];

    cv::Ptr<cv::SeamCarverPipelineInterface> pPipelineInterface =
        std::make_shared<cv::SeamCarverPipelineInterface>(pNewData);

    return pPipelineInterface;
}

bool cv::SeamCarverPipelineManager::addNewPipelineStage(cv::PipelineStages stage)
{
    // can only add a new pipeline stage when this mgr has been initialized
    if (bIsInitialized_)
    {
        cv::Ptr<SeamCarverStage> p_stage = SeamCarverStageFactory::instance().createStage(
            cv::PipelineConfigurationType::VERTICAL_DEFAULT | stage);
        // make sure the pointer returned is not null
        if (p_stage != nullptr)
        {
            cv::Ptr<cv::PipelineQueueData> pNewData = std::make_shared<cv::PipelineQueueData>();
            pNewData->p_input_queue = queue_manager.getQueue(stage);
            pNewData->p_output_queue = queue_manager.getQueue(stage + 1);

            // initialize the stage with the proper queue data
            p_stage->initialize(pNewData);

            // run the stage
            p_stage->runStage();

            // add the stage to the vector
            pipelineStages_[stage].push_back(p_stage);

            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

void cv::SeamCarverPipelineManager::runPipelineStages()
{
    if (!bPipelineStagesRunning_)
    {
        for (int32_t stage = cv::PipelineStages::STAGE_0;
             stage < cv::PipelineStages::NUM_COMPUTE_STAGES; ++stage)
        {
            // start the parallel stages for each 'stage'
            for (size_t p = 0; p < pipelineStages_[stage].size(); ++p)
            {
                if (pipelineStages_[stage][p] != nullptr)
                {
                    pipelineStages_[stage][p]->runStage();
                }
            }
        }

        bPipelineStagesRunning_ = true;
    }
}

void cv::SeamCarverPipelineManager::stopPipelineStages()
{
    if (bPipelineStagesRunning_)
    {
        for (size_t stage = cv::PipelineStages::STAGE_0;
             stage < cv::PipelineStages::NUM_COMPUTE_STAGES; ++stage)
        {
            for (size_t p = 0; p < pipelineStages_[stage].size(); ++p)
            {
                pipelineStages_[stage][p]->stopStage();
            }
        }
        bPipelineStagesRunning_ = false;
    }
}

bool cv::SeamCarverPipelineManager::isInitialized() const { return bIsInitialized_; }

bool cv::SeamCarverPipelineManager::arePipelineStagesCreated() const
{
    return bPipelineStagesCreated_;
}

bool cv::SeamCarverPipelineManager::arePipelineQueuesCreated() const
{
    return bPipelineQueuesCreated_;
}

bool cv::SeamCarverPipelineManager::arePipelineStagesInitialized() const
{
    return bPipelineStagesInitialized_;
}

bool cv::SeamCarverPipelineManager::arePipelineStagesRunning() const
{
    return bPipelineStagesRunning_;
}

void cv::SeamCarverPipelineManager::createPipelineStages()
{
    if (!bPipelineStagesCreated_)
    {
        pipelineStages_.resize(cv::PipelineStages::NUM_COMPUTE_STAGES);

        SeamCarverStageFactory& factory = SeamCarverStageFactory::instance();

        switch (pipelineConfigurationType_)
        {
            case cv::PipelineConfigurationType::VERTICAL_DEFAULT:
                for (int32_t stage = cv::PipelineStages::STAGE_0;
                     stage < cv::PipelineStages::NUM_COMPUTE_STAGES; ++stage)
                {
                    cv::Ptr<SeamCarverStage> p_stage = factory.createStage(
                        cv::PipelineConfigurationType::VERTICAL_DEFAULT | stage);
                    if (p_stage != nullptr)
                    {
                        pipelineStages_[stage].push_back(p_stage);
                    }
                }
                break;
            default:
                break;
        }

        bPipelineStagesCreated_ = true;
    }
}

void cv::SeamCarverPipelineManager::createPipelineQueues()
{
    if (!bPipelineQueuesCreated_)
    {
        std::vector<int32_t> queue_ids;
        for (int32_t stage = cv::PipelineStages::STAGE_0;
             stage < cv::PipelineStages::NUM_QUEUE_STAGES; ++stage)
        {
            queue_ids.push_back(stage);
        }

        queue_manager.initialize(queue_ids);

        bPipelineQueuesCreated_ = true;
    }
}

void cv::SeamCarverPipelineManager::initializePipelineStages()
{
    if (!bPipelineStagesInitialized_ && bPipelineStagesCreated_ && bPipelineQueuesCreated_)
    {
        for (int32_t stage = cv::PipelineStages::STAGE_0;
             stage < cv::PipelineStages::NUM_COMPUTE_STAGES; ++stage)
        {
            // initialize the parallel stages for each 'stage'
            for (size_t p = 0; p < pipelineStages_[stage].size(); ++p)
            {
                if (pipelineStages_[stage][p] != nullptr)
                {
                    cv::Ptr<cv::PipelineQueueData> pNewData =
                        std::make_shared<cv::PipelineQueueData>();

                    pNewData->p_input_queue = queue_manager.getQueue(stage);
                    pNewData->p_output_queue = queue_manager.getQueue(stage + 1);

                    pipelineStages_[stage][p]->initialize(pNewData);
                }
            }
        }
        bPipelineStagesInitialized_ = true;
    }
}
