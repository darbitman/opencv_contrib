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
    const size_t ZERO = 0;


    TEST(ConstSizeMinPriorityQueue, CheckPushThrows)
    {
        // check uninitialized internal memory
        try
        {
            ConstSizeMinPriorityQueue<uint32_t> minHeap;
            minHeap.push(10);
        }
        catch (cv::Exception& ex)
        {
            EXPECT_EQ(ex.code, Error::Code::StsInternal);
        }

        // pushing one more element than the internal capacity allows
        try
        {
            uint32_t capacity = 1;
            ConstSizeMinPriorityQueue<uint32_t> minHeap(capacity);
            minHeap.push(10);
            minHeap.push(10);
        }
        catch (cv::Exception& ex)
        {
            EXPECT_EQ(ex.code, Error::Code::StsInternal);
        }
    }

    TEST(ConstSizeMinPriorityQueue, CheckPopThrows)
    {
        // check uninitialized internal memory
        try
        {
            ConstSizeMinPriorityQueue<uint32_t> minHeap;
            minHeap.pop();
        }
        catch (cv::Exception& ex)
        {
            EXPECT_EQ(ex.code, Error::Code::StsInternal);
        }

        // check popping element when zero internal elements
        try
        {
            size_t capacity = 1;
            ConstSizeMinPriorityQueue<uint32_t> minHeap(capacity);
            minHeap.push(1);
            minHeap.pop();
            minHeap.pop();
        }
        catch (cv::Exception& ex)
        {
            EXPECT_EQ(ex.code, Error::Code::StsInternal);
        }
    }

    TEST(ConstSizeMinPriorityQueue, CheckTopThrows)
    {
        // check uninitialized internal memory has no elements
        try
        {
            ConstSizeMinPriorityQueue<uint32_t> minHeap;
            minHeap.top();
        }
        catch (cv::Exception& ex)
        {
            EXPECT_EQ(ex.code, Error::Code::StsInternal);
        }

        // check returning first element when no elements in PQ
        try
        {
            size_t capacity = 1;
            ConstSizeMinPriorityQueue<uint32_t> minHeap(capacity);
            minHeap.top();
        }
        catch (cv::Exception& ex)
        {
            EXPECT_EQ(ex.code, Error::Code::StsInternal);
        }
    }

    TEST(ConstSizeMinPriorityQueue, DefaultCtor)
    {
        size_t capacity = 10;
        ConstSizeMinPriorityQueue<uint32_t> minHeap;
        EXPECT_EQ(minHeap.capacity(), ZERO);

        minHeap.allocate(capacity);
        EXPECT_EQ(minHeap.capacity(), capacity);

        EXPECT_EQ(minHeap.size(), ZERO);

        vector<uint32_t> greatestToLeast;
        greatestToLeast.push_back(123);
        greatestToLeast.push_back(100);
        greatestToLeast.push_back(92);
        greatestToLeast.push_back(72);
        greatestToLeast.push_back(28);
        greatestToLeast.push_back(15);
        greatestToLeast.push_back(13);
        greatestToLeast.push_back(10);
        greatestToLeast.push_back(9);
        greatestToLeast.push_back(2);

        for (size_t i = 0; i < greatestToLeast.size(); i++)
        {
            minHeap.push(greatestToLeast[i]);
            EXPECT_EQ(minHeap.size(), i + 1);
        }

        // Check whether min is popped first
        while (minHeap.size())
        {
            size_t currentIndex = minHeap.size() - 1;
            EXPECT_EQ(greatestToLeast[currentIndex], minHeap.pop());
        }

        minHeap.allocate(capacity + capacity);
        EXPECT_EQ(minHeap.capacity(), capacity + capacity);
        EXPECT_EQ(minHeap.size(), ZERO);
    }

    TEST(ConstSizeMinPriorityQueue, InitializeCtor)
    {
        size_t capacity = 10;
        ConstSizeMinPriorityQueue<uint32_t> minHeap(capacity);

        EXPECT_EQ(minHeap.capacity(), capacity);

        EXPECT_EQ(minHeap.size(), ZERO);

        vector<uint32_t> greatestToLeast;
        greatestToLeast.push_back(123);
        greatestToLeast.push_back(100);
        greatestToLeast.push_back(92);
        greatestToLeast.push_back(72);
        greatestToLeast.push_back(28);
        greatestToLeast.push_back(15);
        greatestToLeast.push_back(13);
        greatestToLeast.push_back(10);
        greatestToLeast.push_back(9);
        greatestToLeast.push_back(2);

        for (int32_t i = 0; i < (int32_t)greatestToLeast.size(); i++)
        {
            minHeap.push(greatestToLeast[i]);
            EXPECT_EQ((int32_t)minHeap.size(), i + 1);
        }

        // Check whether min is popped first
        while (minHeap.size())
        {
            uint32_t currentIndex = minHeap.size() - 1;
            EXPECT_EQ(greatestToLeast[currentIndex], minHeap.pop());
        }

        minHeap.allocate(capacity + capacity);
        EXPECT_EQ(minHeap.capacity(), capacity + capacity);
        EXPECT_EQ(minHeap.size(), ZERO);
    }

    TEST(ConstSizeMinPriorityQueue, CheckResetHeap)
    {
        size_t capacity = 10;
        ConstSizeMinPriorityQueue<uint32_t> minHeap(capacity);
        EXPECT_EQ(minHeap.capacity(), capacity);

        EXPECT_EQ(minHeap.size(), ZERO);

        vector<uint32_t> greatestToLeast;
        greatestToLeast.push_back(123);
        greatestToLeast.push_back(100);

        for (size_t i = 0; i < greatestToLeast.size(); i++)
        {
            minHeap.push(greatestToLeast[i]);
            EXPECT_EQ(minHeap.size(), i + 1);
        }

        minHeap.resetHeap();
        EXPECT_EQ(minHeap.capacity(), capacity);
        EXPECT_EQ(minHeap.size(), ZERO);
    }
}
