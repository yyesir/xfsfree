/* sys/shmem.cpp
 *
 * Copyright (C) 2007 Antonio Di Monaco
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "shmem.hpp"
#include "sys/exception.hpp"
#include <boost/thread/thread.hpp>

using namespace LFS;

BaseRawSharedMemory::BaseRawSharedMemory(DWORD dwSize, const std::string &sName) :
    _shmname(sName),
    _m(open_or_create, (sName + "_MUTEX").c_str()),
    _size(dwSize)
{
    _shm = shared_memory_object(open_or_create, sName.c_str(), read_write);
    _shm.truncate(dwSize);
    _mapreg = new mapped_region(_shm, read_write);
    _ptr = _mapreg->get_address();

    if (_ptr == NULL)
        throw Exception();

    ::Log::Method m(__SIGNATURE__, sName);
}

BaseRawSharedMemory::~BaseRawSharedMemory()
{
    ::Log::Method m(__SIGNATURE__);

    delete _mapreg;
    shared_memory_object::remove(_shm.get_name());
}

void BaseRawSharedMemory::access(std::function< void(DWORD, LPVOID) > f)
{
    ::Log::Method m(__SIGNATURE__);

    boost::lock_guard<named_mutex> lock(_m);

    f(_size, _mapreg->get_address());
}

RawSharedMemory::RawSharedMemory(DWORD dwSize,const std::string &sName) :
    BaseRawSharedMemory(dwSize,sName)
{
    ::Log::Method m(__SIGNATURE__);
}

RawSharedMemory::~RawSharedMemory()
{
    ::Log::Method m(__SIGNATURE__);
}

void RawSharedMemory::access(std::function< void(DWORD, LPVOID) > f)
{
    ::Log::Method m(__SIGNATURE__);

    BaseRawSharedMemory::access(f);
}
