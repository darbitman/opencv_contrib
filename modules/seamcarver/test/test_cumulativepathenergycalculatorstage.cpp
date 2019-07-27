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

#include <cstdint>
#include <iostream>
#include <memory>

#include "opencv2/seamcarver/cumulativepathenergycalculatorstage.hpp"
#include "opencv2/seamcarver/pipelinequeuedata.hpp"
#include "opencv2/seamcarver/pipelinestages.hpp"
#include "opencv2/seamcarver/sharedqueue.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"
#include "test_precomp.hpp"

using namespace cv;
using std::cout;

namespace opencv_test
{
namespace
{
class CumulativePathEnergyCalculatorStageTest : public ::testing::Test
{
public:
    CumulativePathEnergyCalculatorStageTest() {}

    ~CumulativePathEnergyCalculatorStageTest() {}

    virtual void SetUp() override
    {
        initData = makePtr<PipelineQueueData>();

        if (initData != nullptr)
        {
            initData->pipeline_stage = PipelineStages::STAGE_1;
            initData->p_input_queue = makePtr<SharedQueue<VerticalSeamCarverData*>>();
            initData->p_output_queue = makePtr<SharedQueue<VerticalSeamCarverData*>>();

            stage.initialize(initData);
        }
    }

protected:
    Ptr<PipelineQueueData> initData;

    CumulativePathEnergyCalculatorStage stage;
};

TEST_F(CumulativePathEnergyCalculatorStageTest, Initialize)
{
    ASSERT_EQ(stage.isInitialized(), true);
}

TEST_F(CumulativePathEnergyCalculatorStageTest, RunStage)
{
    ASSERT_EQ(stage.isRunning(), false);

    stage.runStage();

    while (!stage.isRunning())
        ;

    ASSERT_EQ(stage.isRunning(), true);

    stage.stopStage();

    while (stage.isRunning())
        ;

    ASSERT_EQ(stage.isRunning(), false);
}

TEST_F(CumulativePathEnergyCalculatorStageTest, TestQueues)
{
    stage.runStage();

    while (!stage.isRunning())
        ;

    VerticalSeamCarverData* data = new VerticalSeamCarverData();

    this->initData->p_input_queue->push(data);

    while (this->initData->p_output_queue->empty())
        ;

    ASSERT_EQ(this->initData->p_output_queue->empty(), false);

    stage.stopStage();

    while (stage.isRunning())
        ;

    ASSERT_EQ(stage.isRunning(), false);
}
}  // namespace
}  // namespace opencv_test
