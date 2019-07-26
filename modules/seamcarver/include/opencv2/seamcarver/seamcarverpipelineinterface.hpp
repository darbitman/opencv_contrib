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

#ifndef OPENCV_SEAMCARVER_SEAMCARVERPIPELINEINTERFACE_HPP
#define OPENCV_SEAMCARVER_SEAMCARVERPIPELINEINTERFACE_HPP

#include "opencv2/core.hpp"
#include "opencv2/seamcarver/pipelinequeuedata.hpp"

namespace cv
{
class SeamCarverPipelineInterface
{
public:
    SeamCarverPipelineInterface(cv::Ptr<cv::PipelineQueueData> initData);

    ~SeamCarverPipelineInterface();

    void addNewFrame(cv::Ptr<cv::Mat> image, size_t numSeamsToRemove);

    cv::Ptr<cv::Mat> getNextFrame();

    bool doesNewResultExist() const;

    size_t getNumberOfFramesInPipeline() const;

    // deleted to prevent misuse
    SeamCarverPipelineInterface(const SeamCarverPipelineInterface&) = delete;
    SeamCarverPipelineInterface(SeamCarverPipelineInterface&&) = delete;
    SeamCarverPipelineInterface& operator=(const SeamCarverPipelineInterface&) = delete;
    SeamCarverPipelineInterface& operator=(SeamCarverPipelineInterface&&) = delete;

private:
    /// this is where the unused data storage objects will reside
    /// the objects are deemed unused after the result has been returned to the client
    cv::Ptr<cv::SharedContainer<VerticalSeamCarverData*>> p_freestore_queue_;

    /// data storage objects are placed in this queue so that processing can beging
    cv::Ptr<cv::SharedContainer<VerticalSeamCarverData*>> p_input_queue_;

    /// when the data processing has been completed, the results are placed on this queue
    cv::Ptr<cv::SharedContainer<VerticalSeamCarverData*>> p_result_queue_;

    size_t totalFrameInPipeline_;

    uint32_t frameNumber_;
};
}  // namespace cv
#endif
