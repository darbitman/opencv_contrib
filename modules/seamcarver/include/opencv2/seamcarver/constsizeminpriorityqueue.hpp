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

#ifndef OPENCV_SEAMCARVER_CONSTSIZEMINPRIORITYQUEUE_HPP
#define OPENCV_SEAMCARVER_CONSTSIZEMINPRIORITYQUEUE_HPP

#include "priorityqueue.hpp"

namespace cv
{
    template<typename _Tp>
    class CV_EXPORTS ConstSizeMinPriorityQueue : public PriorityQueue<_Tp>
    {
    public:
        /**
         * @brief initialize min oriented binary heap
         * @param capacity maximum number of elements
         */
        explicit ConstSizeMinPriorityQueue(size_t capacity);

        /**
         * @brief default ctor
         */
        ConstSizeMinPriorityQueue();

        /**
         * @brief deallocate memory
         */
        virtual ~ConstSizeMinPriorityQueue();

        /**
         * @brief change capacity and reset priority queue
         * @param newCapacity: maximum number of elements
         */
        virtual void changeCapacity(size_t newCapacity);

        /**
         * @brief resets priority queue to clean state
         */
        virtual void resetPriorityQueue();

        /**
         * @brief insert new element
         * @param element: element to insert
         */
        virtual void push(_Tp element) override;

        /**
         * @brief remove top (minimum) element and return it
         * @return minimum element
         */
        virtual _Tp pop() override;

        /**
         * @brief return the top (minimum) element without deleting it
         * @return minimum element
         */
        virtual _Tp top() const override;

        /**
         * @brief return the number of elements in the queue
         * @return size_t
         */
        virtual size_t size() const override;

        /**
         * @brief return the maximum number of elements that can be stored in the queue
         * @return uint32_t maximum number of elements that can be stored in the queue
         */
        virtual size_t capacity() const;

        /**
         * @brief check if the queue is empty
         * @return bool
         */
        virtual bool empty() const override;

        // default/deleted
        ConstSizeMinPriorityQueue(ConstSizeMinPriorityQueue<_Tp>& rhs) = delete;
        ConstSizeMinPriorityQueue(ConstSizeMinPriorityQueue<_Tp>&& rhs) = default;
        virtual ConstSizeMinPriorityQueue operator=(const ConstSizeMinPriorityQueue<_Tp>& rhs) = delete;
        virtual ConstSizeMinPriorityQueue operator=(const ConstSizeMinPriorityQueue<_Tp>&& rhs) = delete;

    protected:
        /**
         * @brief promote element k if less than its parent
         * @param k: index of element to promote
         */
        virtual void swim(uint32_t k);

        /**
         * @brief demote element k if greater than its parent
         * @param k: index of element to demote
         */
        virtual void sink(uint32_t k);

        /**
         * @brief swap 2 elements
         * @param j: index of the first element
         * @param k: index of the second element
         */
        virtual void exch(uint32_t j, uint32_t k);

        // max number of elements that can be stored
        size_t capacity_ = 0;

        // position of last element
        size_t N_ = 0;

        // underlying storage container for elements in priority queue
        std::vector<_Tp> heap_;
    };


    template<typename _Tp>
    ConstSizeMinPriorityQueue<_Tp>::ConstSizeMinPriorityQueue(size_t capacity) : capacity_(capacity)
    {
        // make sure allocating a heap of at least 1 element
        if (capacity_ == 0)
        {
            CV_Error(Error::Code::StsBadArg, "MinPQ bad capacity argument");
        }
        else
        {
            // binary heaps do not use element 0
            // so need an extra element in array
            heap_.resize(capacity_ + 1);
        }
    }

    template<typename _Tp>
    ConstSizeMinPriorityQueue<_Tp>::ConstSizeMinPriorityQueue() {}

    template<typename _Tp>
    ConstSizeMinPriorityQueue<_Tp>::~ConstSizeMinPriorityQueue() {}

    template<typename _Tp>
    void ConstSizeMinPriorityQueue<_Tp>::changeCapacity(size_t newCapacity)
    {
        // only need to change capacity if increasing size
        if (newCapacity > capacity_)
        {
            capacity_ = newCapacity;
            heap_.resize(capacity_ + 1);
        }

        resetPriorityQueue();
    }

    template<typename _Tp>
    inline void ConstSizeMinPriorityQueue<_Tp>::resetPriorityQueue()
    {
        N_ = 0;
    }


    template<typename _Tp>
    void ConstSizeMinPriorityQueue<_Tp>::push(_Tp element)
    {
        // verify that capacity is non-zero
        // check if queue full
        if (capacity_ == 0 || heap_.size() == 0)
        {
            CV_Error(Error::Code::StsInternal, "push() failed due to zero capacity");
        }
        else if (N_ >= capacity_)
        {
            CV_Error(Error::Code::StsInternal, "push() failed due to full PQ");
        }
        else
        {
            heap_[++N_] = element;
            swim(N_);
        }
    }


    template<typename _Tp>
    _Tp ConstSizeMinPriorityQueue<_Tp>::pop()
    {
        // verify there's a valid item to return
        if (N_ == 0)
        {
            CV_Error(Error::Code::StsInternal, "pop() failed because PQ is empty");
        }

        // save root element
        _Tp min = heap_[1];
        // swap root element and last element
        exch(1, N_--);
        // demote root to reorder heap
        sink(1);
        return min;
    }


    template<typename _Tp>
    _Tp ConstSizeMinPriorityQueue<_Tp>::top() const
    {
        if (N_ == 0)
        {
            CV_Error(Error::Code::StsInternal, "top() failed because PQ is empty");
        }

        return heap_[1];
    }


    template<typename _Tp>
    size_t ConstSizeMinPriorityQueue<_Tp>::size() const
    {
        return N_;
    }


    template<typename _Tp>
    size_t ConstSizeMinPriorityQueue<_Tp>::capacity() const
    {
        return capacity_;
    }


    template<typename _Tp>
    bool ConstSizeMinPriorityQueue<_Tp>::empty() const
    {
        return N_ == 0;
    }


    template<typename _Tp>
    void ConstSizeMinPriorityQueue<_Tp>::swim(uint32_t k)
    {
        // check if we're not at root node and if child is less than parent
        // if so, swap the elements
        while (k > 1 && (heap_[k] < heap_[k / 2]))
        {
            exch(k, k / 2);
            k = k / 2;
        }
    }


    template<typename _Tp>
    void ConstSizeMinPriorityQueue<_Tp>::sink(uint32_t k)
    {
        while (2 * k <= N_)
        {
            uint32_t j = 2 * k;
            // check if left child is greater than right child
            // if so, increment j to point to right child
            if (j < N_ && (heap_[j] > heap_[j + 1]))
            {
                j++;
            }
            // if parent is less than the smallest child, don't need to do anything
            if (heap_[k] < heap_[j])
            {
                break;
            }
            // swap parent and smaller child
            exch(k, j);
            k = j;
        }
    }


    template<typename _Tp>
    void ConstSizeMinPriorityQueue<_Tp>::exch(uint32_t j, uint32_t k)
    {
        _Tp swap = heap_[j];
        heap_[j] = heap_[k];
        heap_[k] = swap;
    }
}

#endif