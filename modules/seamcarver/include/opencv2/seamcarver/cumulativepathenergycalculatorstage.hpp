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

#ifndef OPENCV_SEAMCARVER_CUMULATIVEPATHENERGYCALCULATORSTAGE_HPP
#define OPENCV_SEAMCARVER_CUMULATIVEPATHENERGYCALCULATORSTAGE_HPP

#include <opencv2/core.hpp>

#include "opencv2/seamcarver/pipelineconfigurationtype.hpp"
#include "opencv2/seamcarver/seamcarverstage.hpp"

namespace cv
{
class VerticalSeamCarverData;

class CV_EXPORTS CumulativePathEnergyCalculatorStage : public SeamCarverStage
{
public:
    /// lower 2 bytes are the pipeline stage, upper 2 bytes are the id
    constexpr static uint32_t this_shape_id_ =
        cv::PipelineConfigurationType::VERTICAL_DEFAULT | cv::PipelineStages::STAGE_1;

    CumulativePathEnergyCalculatorStage();

    virtual ~CumulativePathEnergyCalculatorStage();

    virtual void initialize(cv::Ptr<PipelineQueueData> initData) override;

    virtual void runStage() override;

    virtual void stopStage() override;

    virtual bool isInitialized() const override;

    // deleted to prevent misuse
    CumulativePathEnergyCalculatorStage(const CumulativePathEnergyCalculatorStage&) = delete;
    CumulativePathEnergyCalculatorStage(const CumulativePathEnergyCalculatorStage&&) = delete;
    CumulativePathEnergyCalculatorStage& operator=(const CumulativePathEnergyCalculatorStage&) =
        delete;
    CumulativePathEnergyCalculatorStage& operator=(const CumulativePathEnergyCalculatorStage&&) =
        delete;

private:
    /// initialized in the constructor
    volatile bool bDoRunThread_;
    volatile bool bThreadIsStopped_;
    bool bIsInitialized_;

    /// guards the bThreadIsStopped_ member
    std::mutex status_mutex_;
    std::unique_lock<std::mutex> status_lock_;

    /// initialized in the initialize() call
    cv::PipelineStages pipelineStage_;
    cv::Ptr<cv::SharedContainer<VerticalSeamCarverData*>> p_input_queue_;
    cv::Ptr<cv::SharedContainer<VerticalSeamCarverData*>> p_output_queue_;

    void runThread();

    void doStopStage();

    void calculateCumulativePathEnergy(VerticalSeamCarverData*);
};
}  // namespace cv

#endif
