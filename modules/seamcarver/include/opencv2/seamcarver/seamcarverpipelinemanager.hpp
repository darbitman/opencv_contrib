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

#ifndef OPENCV_SEAMCARVER_SEAMCARVERPIPELINEMANAGER_HPP
#define OPENCV_SEAMCARVER_SEAMCARVERPIPELINEMANAGER_HPP

#include <opencv2/core.hpp>
#include <vector>

#include "opencv2/seamcarver/pipelineconfigurationtype.hpp"
#include "opencv2/seamcarver/pipelinestages.hpp"
#include "opencv2/seamcarver/seamcarverpipelineinterface.hpp"
#include "opencv2/seamcarver/seamcarverqueuemanager.hpp"
#include "opencv2/seamcarver/sharedqueue.hpp"

namespace cv
{
// forward declarations
class VerticalSeamCarverData;
class SeamCarverStage;

/// Client calls the contructor with cv::PipelineConfigurationType
/// Client then makes the following calls to START the pipeline
///     1. Call initialize() to initialize the stages
///
///     2. Call runPipelineStages
///
///     3. Call createPipelineInterface() to get a SeamCarverPipelineInterface which allows the
///     client to insert new frames into the pipeline for processing
///
///     4. Use the SeamCarverPipelineInterface object to add new frames for processing and to get
///     the results
///
/// Client can call addNewPipelineStage() to add an additional stage to speed up processing multiple
/// frames in parallel Can only be called after the pipeline has been initialized
class CV_EXPORTS SeamCarverPipelineManager
{
public:
    SeamCarverPipelineManager(cv::PipelineConfigurationType configurationType);

    ~SeamCarverPipelineManager();

    /// initialize the pipeline
    void initialize();

    /// create the interface to the pipeline
    cv::Ptr<cv::SeamCarverPipelineInterface> createPipelineInterface();

    /// add new pipeline stage to add additional parallel processing
    /// can only be called after initialize() has been called
    bool addNewPipelineStage(cv::PipelineStages stage);

    /// start the pipeline
    void runPipelineStages();

    /// stop the pipeline
    void stopPipelineStages();

    /// is this manager initialized
    bool isInitialized() const;

    /// have the pipeline stages been created
    bool arePipelineStagesCreated() const;

    /// have the pipeline queues been created
    bool arePipelineQueuesCreated() const;

    /// are the pipeline stages initialized
    bool arePipelineStagesInitialized() const;

    /// are the pipeline stages running
    bool arePipelineStagesRunning() const;

    // deleted to prevent misuse
    SeamCarverPipelineManager(const SeamCarverPipelineManager&) = delete;
    SeamCarverPipelineManager(const SeamCarverPipelineManager&&) = delete;
    SeamCarverPipelineManager& operator=(const SeamCarverPipelineManager&) = delete;
    SeamCarverPipelineManager& operator=(const SeamCarverPipelineManager&&) = delete;

private:
    /// have the pipeline stages been created
    bool bPipelineStagesCreated_;

    /// have the pipeline queues been created
    bool bPipelineQueuesCreated_;

    /// have the pipeline stages been started
    bool bPipelineStagesInitialized_;

    /// is the pipeline manager initialized
    bool bIsInitialized_;

    /// are the pipeline stages running
    bool bPipelineStagesRunning_;

    /// create pipeline stages
    void createPipelineStages();

    /// create queues
    void createPipelineQueues();

    /// pass the queues and locks to the stages and start each stage's execution
    void initializePipelineStages();

    SeamCarverQueueManager queue_manager;

    std::vector<cv::Ptr<cv::SharedContainer<VerticalSeamCarverData*>>> queues_;

    std::vector<std::vector<cv::Ptr<SeamCarverStage>>> pipelineStages_;

    cv::PipelineConfigurationType pipelineConfigurationType_;
};
}  // namespace cv

#endif
