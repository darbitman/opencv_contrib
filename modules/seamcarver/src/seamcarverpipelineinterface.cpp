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

    VerticalSeamCarverData* data = p_freestore_queue_->front();

    while (true)
    {
        // copy image to internal data store
        data->savedImage = cv::makePtr<cv::Mat>(image->clone());

        // initialize internal data
        if (data->bNeedToInitializeLocalData)
        {
            init(image, (size_t)image->rows);
            break;
        }
        else
        {
            // check if image is of the same dimension as those used for internal data
            if (areImageDimensionsVerified())
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

    // check if removing more seams than columsn available
    if (numSeamsToRemove > data->numColumns_)
    {
        // TODO handle error
    }

    // set number of seams to remove this pass
    data->numSeamsToRemove_ = numSeamsToRemove;

    // reset vectors to their clean state
    resetLocalVectors();

    // separate individual color channels
    extractChannels();

    p_freestore_queue_->pop();
    p_input_queue_->push(data);
}

void cv::SeamCarverPipelineInterface::resetLocalVectors()
{
    VerticalSeamCarverData* data = p_freestore_queue_->front();

    // set marked pixels to false for new run
    for (size_t row = 0; row < data->numRows_; row++)
    {
        for (size_t column = 0; column < data->numColumns_; column++)
        {
            data->markedPixels[row][column] = false;
        }
    }

    // ensure each row's PQ has enough capacity
    for (size_t row = 0; row < data->seamLength_; row++)
    {
        if (data->numSeamsToRemove_ > data->discoveredSeams[row].capacity())
        {
            data->discoveredSeams[row].changeCapacity(data->numSeamsToRemove_);
        }

        // reset priority queue since it could be filled from a previous run
        if (!data->discoveredSeams[row].empty())
        {
            data->discoveredSeams[row].resetPriorityQueue();
        }
    }
}

void cv::SeamCarverPipelineInterface::extractChannels()
{
    VerticalSeamCarverData* data = p_freestore_queue_->front();

    data->numColorChannels_ = (size_t)data->savedImage->channels();

    if (data->numColorChannels_ == 3)
    {
        if (data->bgr.size() != 3)
        {
            data->bgr.resize(3);
        }

        cv::split(*(data->savedImage), data->bgr);
    }
    else if (data->numColorChannels_ == 1)
    {
        if (data->bgr.size() != 1)
        {
            data->bgr.resize(1);
        }

        cv::extractChannel(*(data->savedImage), data->bgr[0], 0);
    }
    else
    {
        // TODO handle error case
    }
}

bool cv::SeamCarverPipelineInterface::areImageDimensionsVerified() const
{
    VerticalSeamCarverData* data = p_freestore_queue_->front();

    return ((size_t)data->savedImage->rows == data->numRows_ &&
            (size_t)data->savedImage->cols == data->numColumns_);
}
