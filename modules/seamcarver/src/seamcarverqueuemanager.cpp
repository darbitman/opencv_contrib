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

#include "opencv2/seamcarver/seamcarverqueuemanager.hpp"

#include <map>
#include <opencv2/core.hpp>
#include <vector>

#include "opencv2/seamcarver/sharedqueue.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::SeamCarverQueueManager::SeamCarverQueueManager() : b_mgr_initialized_(false) {}

cv::SeamCarverQueueManager::~SeamCarverQueueManager() {}

void cv::SeamCarverQueueManager::initialize(const std::vector<int32_t>& queue_ids)
{
    if (!b_mgr_initialized_)
    {
        for (size_t i = 0; i < queue_ids.size(); ++i)
        {
            // only map UNIQUE IDs so make sure the id doesn't already exist
            if (id_to_queue_map_.count(queue_ids[i]) == 0)
            {
                id_to_queue_map_[queue_ids[i]] =
                    cv::makePtr<cv::SharedQueue<cv::VerticalSeamCarverData*>>();
            }
        }
        b_mgr_initialized_ = true;
    }
}

bool cv::SeamCarverQueueManager::isInitialized() const { return b_mgr_initialized_; }

cv::Ptr<cv::SharedQueue<cv::VerticalSeamCarverData*>> cv::SeamCarverQueueManager::getQueue(
    int32_t queue_id) const
{
    if (id_to_queue_map_.count(queue_id) > 0)
    {
        return id_to_queue_map_.at(queue_id);
    }
    else
    {
        return nullptr;
    }
}

size_t cv::SeamCarverQueueManager::getNumberOfQueues() const { return id_to_queue_map_.size(); }
