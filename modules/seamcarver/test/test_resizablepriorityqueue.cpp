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

#include "test_precomp.hpp"

namespace opencv_test
{
    //using std::priority_queue;
    //using std::vector;

    //typedef cv::ResizablePriorityQueue<int32_t, std::greater<int32_t>> MinPQ;

    //struct ResizablePriorityQueueTest : testing::Test
    //{
    //    MinPQ* pq;

    //    ResizablePriorityQueueTest()
    //    {
    //        pq = new MinPQ();
    //    }

    //    virtual ~ResizablePriorityQueueTest()
    //    {
    //        delete pq;
    //    }
    //};

    //TEST_F(ResizablePriorityQueueTest, ContainerSize)
    //{
    //    // initial capacity should be 0
    //    EXPECT_EQ(0, pq->GetContainerCapacity());

    //    // reserve memory for NewCapacity elements
    //    uint32_t NewCapacity = 100;
    //    EXPECT_EQ(true, pq->SetCapacity(NewCapacity));
    //    EXPECT_EQ(NewCapacity, pq->GetContainerCapacity());

    //    // number of elements should be NewCapacity
    //    // container capacity should be NewCapacity
    //    for (int32_t n = 0; n < NewCapacity; n++)
    //    {
    //        pq->push(n);
    //    }
    //    EXPECT_EQ(pq->size(), NewCapacity);
    //    EXPECT_EQ(pq->GetContainerCapacity(), NewCapacity);

    //    // number of elements should increase by 1
    //    // container capacity should AT LEAST increase by 1
    //    pq->push(0);
    //    EXPECT_EQ(pq->size(), NewCapacity + 1);
    //    EXPECT_GE(pq->GetContainerCapacity(), NewCapacity + 1);

    //    // remove all elements from the queue, so size should be 0
    //    // capacity shall not change
    //    while (pq->size() > 0)
    //    {
    //        pq->pop();
    //    }
    //    EXPECT_EQ(pq->size(), 0);
    //    EXPECT_GE(pq->GetContainerCapacity(), NewCapacity + 1);
    //}

    //TEST(ResizableMinPriorityQueueTest, InitialNonZeroCapacity)
    //{
    //    // create a new Min Oriented Priority Queue with space for NewCapacity elements
    //    uint32_t NewCapacity = 100;
    //    MinPQ* pq = new MinPQ(NewCapacity);
    //    EXPECT_EQ(pq->size(), 0);
    //    EXPECT_EQ(pq->GetContainerCapacity(), NewCapacity);

    //    // number of elements should be NewCapacity
    //    // container capacity should be NewCapacity
    //    for (int32_t n = 0; n < NewCapacity; n++)
    //    {
    //        pq->push(NewCapacity - n);
    //    }
    //    EXPECT_EQ(pq->size(), NewCapacity);
    //    EXPECT_EQ(pq->GetContainerCapacity(), NewCapacity);

    //    // number of elements should increase by 1
    //    // container capacity should AT LEAST increase by 1
    //    pq->push(0);
    //    EXPECT_EQ(pq->size(), NewCapacity + 1);
    //    EXPECT_GE(pq->GetContainerCapacity(), NewCapacity + 1);

    //    // remove all elements from the queue, so size should be 0
    //    // capacity shall not change
    //    // all elements added were 0 thru 100 in descending order
    //    // elements removed should be 0 thru 100 in ascending order
    //    // the current top should be greater than the previous top
    //    {
    //        int32_t top = pq->top();
    //        while (pq->size() > 0)
    //        {
    //            pq->pop();
    //            if (pq->size())
    //            {
    //                EXPECT_GE(pq->top(), top);
    //                top = pq->top();
    //            }
    //        }
    //    }
    //    EXPECT_EQ(pq->size(), 0);
    //    EXPECT_GE(pq->GetContainerCapacity(), NewCapacity + 1);

    //    delete pq;
    //}
}