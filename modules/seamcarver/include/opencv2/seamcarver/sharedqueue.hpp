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

#ifndef OPENCV_SEAMCARVER_SHAREDQUEUE_HPP
#define OPENCV_SEAMCARVER_SHAREDQUEUE_HPP

#include <mutex>
#include <queue>

namespace cv
{
template <typename _Tp>
class SharedQueue
{
public:
    SharedQueue();

    ~SharedQueue();

    _Tp& front();

    bool empty() const;

    size_t size() const;

    void push(const _Tp& value);

    void push(_Tp&& value);

    template <typename... _Args>
    void emplace(_Args&&... __args);

    void pop();

private:
    mutable std::mutex mtx_;
    std::queue<_Tp> queue_;
};

template <typename _Tp>
SharedQueue<_Tp>::SharedQueue()
{
}

template <typename _Tp>
SharedQueue<_Tp>::~SharedQueue()
{
}

template <typename _Tp>
_Tp& SharedQueue<_Tp>::front()
{
    std::unique_lock<std::mutex> mlock(mtx_);
    return queue_.front();
}

template <typename _Tp>
bool SharedQueue<_Tp>::empty() const
{
    std::unique_lock<std::mutex> mlock(mtx_);
    return queue_.empty();
}

template <typename _Tp>
size_t SharedQueue<_Tp>::size() const
{
    std::unique_lock<std::mutex> mlock(mtx_);
    return queue_.size();
}

template <typename _Tp>
void SharedQueue<_Tp>::push(const _Tp& value)
{
    std::unique_lock<std::mutex> mlock(mtx_);
    queue_.push(value);
}

template <typename _Tp>
void SharedQueue<_Tp>::push(_Tp&& value)
{
    std::unique_lock<std::mutex> mlock(mtx_);
    queue_.push(value);
}

template <typename _Tp>
template <typename... _Args>
void SharedQueue<_Tp>::emplace(_Args&&... __args)
{
    std::unique_lock<std::mutex> mlock(mtx_);
    queue_.emplace(std::forward<_Args>(__args)...);
}

template <typename _Tp>
void SharedQueue<_Tp>::pop()
{
    std::unique_lock<std::mutex> mlock(mtx_);
    queue_.pop();
}

}  // namespace cv
#endif
