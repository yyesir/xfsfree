/* sys/library.hpp
 *
 * Copyright (C) 2007 Antonio Di Monaco
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once
#ifndef __SYS_LIBRARY_HPP__
#define __SYS_LIBRARY_HPP__

#include <string>
#include <stdexcept>
#include <dlfcn.h>
#include "util/constraints.hpp"
#include "LFSapi.h"
#include "handle.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4191)
#endif

namespace LFS
{
  class Library : public Handle< HANDLE >
  {
    NON_COPYABLE(Library);

  public:
    explicit Library(const std::string &sLibPath);
    ~Library();

    template< typename R, typename... Args >
    R call(const char* funcName, Args... args)
    {
      auto f = reinterpret_cast< R (/*__stdcall*/ *)(Args...) >(dlsym(handle(),funcName));
      if (f == NULL)
        throw Exception();

      return f(args...);
    }
  };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
