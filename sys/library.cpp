/* sys/library.cpp
 *
 * Copyright (C) 2007 Antonio Di Monaco
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "library.hpp"
#include "log/log.hpp"

using namespace LFS;

Library::Library(const std::string &sLibPath) :
  Handle< void* >(dlopen(sLibPath.c_str(), RTLD_NOW), dlclose)
{
  ::Log::Method m(__SIGNATURE__);
}

Library::~Library()
{
  ::Log::Method m(__SIGNATURE__);
}
