// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_EXCEPTION_H
#define MUDUO_BASE_EXCEPTION_H

#include "muduo/base/Types.h"
#include <exception>

namespace muduo
{

class Exception : public std::exception
{
 public:
  Exception(string what);
  ~Exception() noexcept override = default;

  // default copy-ctor and operator= are okay.

  const char* what() const noexcept override
  {
    return message_.c_str();
  }

  const char* stackTrace() const noexcept
  {
    return stack_.c_str();
  }

 private:
  string message_;     //异常信息的字符串
  string stack_;      //保存异常发生时的栈信息
};

}  // namespace muduo

#endif  // MUDUO_BASE_EXCEPTION_H

//介绍fillStackTrace函数
//backtrace()   栈回溯，保存各个栈帧的地址 ,可以通过man帮助看使用方法
//backtrace_symbols()  ，根据地址，转换为相应的函数符号。 将地址转换为函数的名称,返回一个指针的指针，可以通过man帮助看使用方法