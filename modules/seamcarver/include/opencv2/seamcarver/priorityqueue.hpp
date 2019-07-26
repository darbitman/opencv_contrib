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

#ifndef OPENCV_SEAMCARVER_PRIORITYQUEUE_HPP
#define OPENCV_SEAMCARVER_PRIORITYQUEUE_HPP

namespace cv
{
template <typename _Tp>
class PriorityQueue
{
public:
    /**
     * @brief dtor
     */
    virtual ~PriorityQueue() {}

    /**
     * @brief insert new element into priority queue
     * @param element: new element to insert
     */
    virtual void push(const _Tp& element) = 0;

    /**
     * @brief insert new element into priority queue
     * @param element: new element to insert
     */
    virtual void push(_Tp&& element) = 0;

    /**
     * @brief remove top element
     */
    virtual void pop() = 0;

    /**
     * @brief return the top (minimum) element without deleting it
     * @return const _Tp&: const reference to the minimum element
     */
    virtual const _Tp& top() const = 0;

    /**
     * @brief return the number of elements in the queue
     * @return size_t
     */
    virtual size_t size() const = 0;

    /**
     * @brief check if the queue is empty
     * @return bool returns true if queue is empty
     */
    virtual bool empty() const = 0;
};
}  // namespace cv

#endif