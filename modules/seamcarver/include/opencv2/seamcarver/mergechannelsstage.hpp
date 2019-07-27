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

#ifndef OPENCV_SEAMCARVER_MERGECHANNELSSTAGE_HPP
#define OPENCV_SEAMCARVER_MERGECHANNELSSTAGE_HPP

#include <opencv2/core.hpp>

#include "opencv2/seamcarver/baseseamcarverstage.hpp"
#include "opencv2/seamcarver/pipelineconfigurationtype.hpp"
#include "opencv2/seamcarver/pipelinequeuedata.hpp"

namespace cv
{
class CV_EXPORTS MergeChannelsStage : public BaseSeamCarverStage
{
public:
    /// lower 2 bytes are the pipeline stage, upper 2 bytes are the id
    constexpr static uint32_t this_shape_id_ =
        PipelineConfigurationType::VERTICAL_DEFAULT | PipelineStages::STAGE_4;

    MergeChannelsStage();

    virtual ~MergeChannelsStage();

    virtual void initialize(Ptr<PipelineQueueData> initData);

    virtual void runStage();

    virtual void stopStage();

    virtual bool isInitialized() const;

    virtual bool isRunning() const;

    // deleted to prevent misuse
    MergeChannelsStage(const MergeChannelsStage&) = delete;
    MergeChannelsStage(MergeChannelsStage&&) = delete;
    MergeChannelsStage& operator=(const MergeChannelsStage&) = delete;
    MergeChannelsStage& operator=(MergeChannelsStage&&) = delete;

protected:
    /**
     * @brief method that does the actual data processing
     * calls calculateCumulativePathEnergy
     */
    virtual void processData(VerticalSeamCarverData* data);

private:
    void mergeChannels(VerticalSeamCarverData* data);
};
}  // namespace cv

#endif
