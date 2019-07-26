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

#ifndef OPENCV_SEAMCARVER_SHAREDCONSTSIZEPQADAPTER_HPP
#define OPENCV_SEAMCARVER_SHAREDCONSTSIZEPQADAPTER_HPP

#include <memory>
#include <mutex>
#include <opencv2/core.hpp>
#include <queue>

#include "constsizepriorityqueue.hpp"
#include "sharedcontainer.hpp"

namespace cv
{
template <typename _Tp, typename _ComparatorType = std::less<_Tp>>
class CV_EXPORTS SharedConstSizePQAdapter : public SharedContainer<_Tp>
{
public:
    SharedConstSizePQAdapter(
        std::shared_ptr<ConstSizePriorityQueue<_Tp, _ComparatorType>> pConstSizePQ);

    virtual ~SharedConstSizePQAdapter();

    virtual const _Tp& getNext() override;

    virtual bool empty() const override;

    virtual size_t size() const override;

    virtual void push(const _Tp& value) override;

    virtual void push(_Tp&& value) override;

    virtual void removeNext() override;

private:
    std::shared_ptr<ConstSizePriorityQueue<_Tp, _ComparatorType>> pConstSizeMinPQ_;
};

template <typename _Tp, typename _ComparatorType>
SharedConstSizePQAdapter<_Tp, _ComparatorType>::SharedConstSizePQAdapter(
    std::shared_ptr<ConstSizePriorityQueue<_Tp, _ComparatorType>> pConstSizePQ)
    : pConstSizeMinPQ_(pConstSizePQ)
{
}

template <typename _Tp, typename _ComparatorType>
SharedConstSizePQAdapter<_Tp, _ComparatorType>::~SharedConstSizePQAdapter()
{
}

template <typename _Tp, typename _ComparatorType>
_Tp const& SharedConstSizePQAdapter<_Tp, _ComparatorType>::getNext()
{
    _Tp* toReturn = new _Tp(pConstSizeMinPQ_->top());
    return *toReturn;
}

template <typename _Tp, typename _ComparatorType>
bool SharedConstSizePQAdapter<_Tp, _ComparatorType>::empty() const
{
    return pConstSizeMinPQ_->empty();
}

template <typename _Tp, typename _ComparatorType>
size_t SharedConstSizePQAdapter<_Tp, _ComparatorType>::size() const
{
    return pConstSizeMinPQ_->size();
}

template <typename _Tp, typename _ComparatorType>
void SharedConstSizePQAdapter<_Tp, _ComparatorType>::push(const _Tp& value)
{
    pConstSizeMinPQ_->push(value);
}

template <typename _Tp, typename _ComparatorType>
void SharedConstSizePQAdapter<_Tp, _ComparatorType>::push(_Tp&& value)
{
    pConstSizeMinPQ_->push(value);
}

template <typename _Tp, typename _ComparatorType>
void SharedConstSizePQAdapter<_Tp, _ComparatorType>::removeNext()
{
    pConstSizeMinPQ_->pop();
}

}  // namespace cv

#endif
