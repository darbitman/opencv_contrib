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

#include <stdlib.h>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>

#include "opencv2/seamcarver/constsizepriorityqueue.hpp"
#include "opencv2/seamcarver/sharedconstsizepqadapter.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"
#include "test_precomp.hpp"

using std::make_shared;
using std::priority_queue;
using std::shared_ptr;
using std::vector;

using namespace cv;

namespace opencv_test
{
namespace
{
TEST(PQAdapter, MinOrientedPQ)
{
    class ComparePointers
    {
    public:
        bool operator()(shared_ptr<VerticalSeamCarverData> p1,
                        shared_ptr<VerticalSeamCarverData> p2)
        {
            return p1->getFrameNumber() < p2->getFrameNumber();
        }
    };

    size_t pqCapacity = 150;

    shared_ptr<ConstSizePriorityQueue<shared_ptr<VerticalSeamCarverData>, ComparePointers>> pPq =
        make_shared<ConstSizePriorityQueue<shared_ptr<VerticalSeamCarverData>, ComparePointers>>(
            pqCapacity);

    EXPECT_EQ(pPq->capacity(), pqCapacity);

    SharedConstSizePQAdapter<shared_ptr<VerticalSeamCarverData>, ComparePointers> pqAdapter(pPq);

    EXPECT_EQ(pqAdapter.size(), pPq->size());

    // fill in a general PQ, then place into a vector to use for comparison later
    priority_queue<uint32_t, vector<uint32_t>, std::greater<uint32_t>> randomFrameNumbers;
    vector<uint32_t> sortedFrameNumbers;

    // generate random numbers and place them into the pq under test and the general pq from std
    for (int32_t i = 0; i < 10; ++i)
    {
        shared_ptr<VerticalSeamCarverData> pData = make_shared<VerticalSeamCarverData>();
        uint32_t randomNumber = (uint32_t)(rand() % 50);
        pData->setFrameNumber(randomNumber);
        randomFrameNumbers.push(randomNumber);
        pqAdapter.push(pData);
    }

    // the vector will have the numbers sorted that are from the general pq
    while (!randomFrameNumbers.empty())
    {
        sortedFrameNumbers.push_back(randomFrameNumbers.top());
        randomFrameNumbers.pop();
    }

    // compare the sorted vector to the pq under test
    vector<uint32_t>::iterator iter = sortedFrameNumbers.begin();
    while (!pqAdapter.empty())
    {
        ASSERT_EQ(*iter, pqAdapter.getNext()->getFrameNumber());
        pqAdapter.removeNext();
        ++iter;
    }
}
}  // namespace
}  // namespace opencv_test
