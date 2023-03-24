/* sys/shmem.hpp
 *
 * Copyright (C) 2007 Antonio Di Monaco
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once
#ifndef __SHMEM_HPP__
#define __SHMEM_HPP__

#include <functional>
#include "log/log.hpp"
#include "util/memory.hpp"
#include "util/constraints.hpp"
#include "LFSapi.h"
#include <boost/thread/mutex.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

using namespace boost::interprocess;

namespace LFS
{
  class BaseRawSharedMemory
  {
    NON_COPYABLE(BaseRawSharedMemory);

    std::string _shmname;
    shared_memory_object _shm;
    mapped_region* _mapreg;

    named_mutex _m;
    DWORD _size;
    LPVOID _ptr;

  protected:
    explicit BaseRawSharedMemory(DWORD dwSize, const std::string &sName);
    ~BaseRawSharedMemory();

    void access(std::function< void(DWORD, LPVOID) > f);
  };

  class RawSharedMemory : public BaseRawSharedMemory
  {
    NON_COPYABLE(RawSharedMemory);

  public:
    explicit RawSharedMemory(DWORD dwSize, const std::string &sName);
    ~RawSharedMemory();

    void access(std::function< void(DWORD, LPVOID) > f);
  };

  template< typename T >
  class SharedMemory : public BaseRawSharedMemory
  {
    NON_COPYABLE(SharedMemory);

  public:
    explicit SharedMemory(const std::string &sName) :
      BaseRawSharedMemory(sizeof(T), sName)
    {
      ::Log::Method m(__SIGNATURE__);
    }

    ~SharedMemory()
    {
      ::Log::Method m(__SIGNATURE__);
    }

    void access(std::function< void(T *) > f)
    {
      BaseRawSharedMemory::access([f] (DWORD, LPVOID ptr)
        {
          f(reinterpret_cast< T * >(ptr));
        });
    }
  };
}

#endif
