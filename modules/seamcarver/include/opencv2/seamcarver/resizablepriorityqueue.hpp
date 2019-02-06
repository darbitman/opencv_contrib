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

#ifndef OPENCV_SEAMCARVER_RESIZABLEPRIORITYQUEUE_HPP
#define OPENCV_SEAMCARVER_RESIZABLEPRIORITYQUEUE_HPP

#include <queue>

namespace cv {
  template <class _Ty,
    class _Pr = std::less<typename std::vector<_Ty>::value_type>>
  class ResizablePriorityQueue : public std::priority_queue<_Ty, std::vector<_Ty>, _Pr>
  {
  public:
    ResizablePriorityQueue(int32_t capacity = 0)
    {
      this->c.reserve(capacity);
    }

    uint32_t GetContainerCapacity() const
    {
      return this->c.capacity();
    }

    bool SetCapacity(uint32_t capacity)
    {
      if (this->c.capacity() > 0)
      {
        return false;
      }
      else
      {
        this->c.reserve(capacity);
        return true;
      }
    }

    /**
     * @brief remove top (minimum) element and return it
     * @return minimum element
     */
    _Ty pop()
    {
      _Ty toReturn = this->top();
      std::priority_queue<_Ty, vector<_Ty>, _Pr>::pop();
      return toReturn;
    }
  };
}

#endif