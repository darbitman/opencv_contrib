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

#ifndef OPENCV_SEAMCARVER_PIPELINESTAGES_HPP
#define OPENCV_SEAMCARVER_PIPELINESTAGES_HPP

namespace cv
{
enum PipelineStages
{
    /// compute energy
    STAGE_0 = 0,

    /// calculate cumulative path energy
    STAGE_1 = 1,

    /// find seams
    STAGE_2 = 2,

    /// remove seams
    STAGE_3 = 3,

    /// merge channels
    STAGE_4 = 4,

    /**********************/
    // add more stages above
    /**********************/
    NUM_COMPUTE_STAGES,

    LAST_QUEUE_STAGE = NUM_COMPUTE_STAGES,

    /// the number of QUEUES (1 more than number of COMPUTE stages)
    /// EXAMPLE:
    /// 0   1   2   3   4   5   (Queue Stages)
    /// Q C Q C Q C Q C Q C Q
    ///   0   1   2   3   4     (Compute Stages)
    /// there are 6 queues, and 5 compute stages
    NUM_QUEUE_STAGES
};
}  // namespace cv

#endif
