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

#include "opencv2/seamcarver/seamcarverpipelineinterface.hpp"
#include "opencv2/seamcarver/sharedqueue.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::SeamCarverPipelineInterface::SeamCarverPipelineInterface(
    cv::Ptr<cv::PipelineQueueData> initData)
    : totalFrameInPipeline_(0), frameNumber_(0)
{
    p_freestore_queue_ = cv::makePtr<cv::SharedQueue<VerticalSeamCarverData*>>();
    p_input_queue_ = initData->p_input_queue;
    p_result_queue_ = initData->p_output_queue;
}

cv::SeamCarverPipelineInterface::~SeamCarverPipelineInterface() {}

void cv::SeamCarverPipelineInterface::addNewFrame(cv::Ptr<cv::Mat> image, size_t numSeamsToRemove)
{
    if (p_freestore_queue_->empty())
    {
        p_freestore_queue_->push(new VerticalSeamCarverData());
    }

    VerticalSeamCarverData* data = p_freestore_queue_->getNext();

    data->setFrameNumber(frameNumber_++);
    
    while (true)
    {
        // copy image to internal data store
        data->savedImage = cv::makePtr<cv::Mat>(image->clone());

        // initialize internal data
        if (data->bNeedToInitializeLocalData)
        {
            data->initialize();
            break;
        }
        else
        {
            // check if image is of the same dimension as those used for internal data
            if (data->areImageDimensionsVerified())
            {
                break;
            }
            // if image dimensions are different than those of internal data, reinitialize data
            else
            {
                data->bNeedToInitializeLocalData = true;
            }
        }
    }

    // check if removing more seams than columns available
    if (numSeamsToRemove > data->numColumns_)
    {
        // TODO handle error
    }

    // set number of seams to remove this pass
    data->numSeamsToRemove_ = numSeamsToRemove;

    // reset vectors to their clean state
    data->resetData();

    // separate individual color channels
    data->separateChannels();

    p_freestore_queue_->removeNext();
    p_input_queue_->push(data);
    ++totalFrameInPipeline_;
}

cv::Ptr<cv::Mat> cv::SeamCarverPipelineInterface::getNextFrame()
{
    cv::Ptr<cv::Mat> frameToReturn(nullptr);

    if (!p_result_queue_->empty())
    {
        // swap the pointers between the result image and the nullptr-initialized frameToReturn
        p_result_queue_->getNext()->savedImage.swap(frameToReturn);

        // result was extracted, so remove the data storage object and put back onto the freestore
        // queue for future use
        p_freestore_queue_->push(p_result_queue_->getNext());

        // removed frame from pipeline and returning to client so decrement counter
        --totalFrameInPipeline_;
    }

    return frameToReturn;
}

bool cv::SeamCarverPipelineInterface::doesNewResultExist() const
{
    return !p_result_queue_.empty();
}

size_t cv::SeamCarverPipelineInterface::getNumberOfFramesInPipeline() const
{
    return totalFrameInPipeline_;
}
