/* sys/exception.hpp
 *
 * Copyright (C) 2007 Antonio Di Monaco
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once
#ifndef __SYS_EXCEPTION_HPP__
#define __SYS_EXCEPTION_HPP__

#include <system_error>

namespace LFS
{
  class Exception : public std::system_error
  {
    static std::string errorToMsg();

  public:
    explicit Exception();
  };
}

#endif
