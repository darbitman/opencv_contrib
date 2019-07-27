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

#ifndef OPENCV_SEAMCARVER_COMPUTEENERGYSTAGE_HPP
#define OPENCV_SEAMCARVER_COMPUTEENERGYSTAGE_HPP

#include <opencv2/core.hpp>

#include "opencv2/seamcarver/pipelineconfigurationtype.hpp"
#include "opencv2/seamcarver/pipelinequeuedata.hpp"
#include "opencv2/seamcarver/seamcarverstage.hpp"
#include "opencv2/seamcarver/sharedqueue.hpp"

namespace cv
{
class VerticalSeamCarverData;

class CV_EXPORTS ComputeEnergyStage : public SeamCarverStage
{
public:
    /// lower 2 bytes are the pipeline stage, upper 2 bytes are the id
    constexpr static uint32_t this_shape_id_ =
        cv::PipelineConfigurationType::VERTICAL_DEFAULT | cv::PipelineStages::STAGE_0;

    ComputeEnergyStage();

    virtual ~ComputeEnergyStage();

    virtual void initialize(cv::Ptr<cv::PipelineQueueData> initData) override;

    virtual void runStage() override;

    virtual void stopStage() override;

    virtual bool isInitialized() const override;

    virtual bool isRunning() const override;

    // deleted to prevent misuse
    ComputeEnergyStage(const ComputeEnergyStage&) = delete;
    ComputeEnergyStage(ComputeEnergyStage&&) = delete;
    ComputeEnergyStage& operator=(const ComputeEnergyStage&) = delete;
    ComputeEnergyStage& operator=(ComputeEnergyStage&&) = delete;

private:
    /// Flag to start and stop the thread and to keep track if it's running
    volatile bool bThreadIsRunning_;

    // Indicates if this stage is initialized
    bool bIsInitialized_;

    /// guards the bThreadIsRunning_ member
    mutable std::mutex statusMutex_;

    /// initialized in the initialize() call
    cv::Ptr<cv::SharedContainer<VerticalSeamCarverData*>> pInputQueue_;
    cv::Ptr<cv::SharedContainer<VerticalSeamCarverData*>> pOutputQueue_;

    // image dimensions
    size_t numRows_;
    size_t numColumns_;
    size_t bottomRow_;
    size_t rightColumn_;
    size_t numColorChannels_;

    void runThread();

    void doStopStage();

    void calculatePixelEnergy(const cv::Ptr<const cv::Mat>& image,
                              std::vector<std::vector<double>>& outPixelEnergy);

    void calculatePixelEnergyForEveryRow(const cv::Ptr<const cv::Mat>& image,
                                         std::vector<std::vector<double>>& outPixelEnergy,
                                         bool bDoOddColumns);

    void calculatePixelEnergyForEveryColumn(const cv::Ptr<const cv::Mat>& image,
                                            std::vector<std::vector<double>>& outPixelEnergy,
                                            bool bDoOddRows);
};
}  // namespace cv

#endif
