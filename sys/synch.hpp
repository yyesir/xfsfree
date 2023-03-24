/* sys/synch.hpp
 *
 * Copyright (C) 2007 Antonio Di Monaco
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#pragma once
#ifndef __SYS_SYNCH_HPP__
#define __SYS_SYNCH_HPP__

#include <string>
#include "sys/handle.hpp"
#include "util/constraints.hpp"
#include "util/noexcept.hpp"
#include <semaphore.h>

namespace LFS
{
namespace Synch
{
#if 0
class Lockable : public Handle<>
{
    NON_COPYABLE(Lockable);

public:
    explicit Lockable(HANDLE h);

    void lock();
};

class Mutex : public Lockable
{
    NON_COPYABLE(Mutex);

public:
    explicit Mutex(const std::wstring &sName = std::wstring());

    void unlock();
};
#endif
class Semaphore //: public Handle<>
{
    NON_COPYABLE(Semaphore);

public:
    explicit Semaphore(LONG start, LONG max, const std::string &sName = std::string());

    void acquire();
    void release();

private:
    sem_t m_sem;
};

#if 0
class Event : public Handle<>
{
    NON_COPYABLE(Event);

public:
    explicit Event(const std::wstring &sName = std::wstring());

    void waitFor();
    void set();
};

template< typename T >
class Locker
{
    bool _ok;
    T &_ref;

    Locker(const Locker &);
    Locker &operator=(const Locker &);

public:
    explicit Locker(T &ref) : _ok(false), _ref(ref)
    {
        _ref.lock();
        _ok = true;
    }

    ~Locker() NOEXCEPT(false)
    {
        if (_ok)
            _ref.unlock();
    }
};

template<>
class Locker< HANDLE >
{
    bool _ok;
    HANDLE _ref;

    NON_COPYABLE(Locker);

public:
    explicit Locker(HANDLE ref) : _ok(false), _ref(ref)
    {
        //        if (WaitForSingleObjectEx(_ref,INFINITE,FALSE) == WAIT_FAILED)
        //          throw Exception();

        _ok = true;
    }

    Locker(Locker &&o) :
        _ref(o._ref),
        _ok(o._ok)
    {
        o._ref = NULL;
    }

    ~Locker() NOEXCEPT(false)
    {
        if ((_ref != NULL) && _ok)
        {
            //          if (!ReleaseMutex(_ref))
            //            throw Exception();
        }
    }

    Locker &operator=(Locker &&o)
    {
        _ref = o._ref;
        _ok = o._ok;
        o._ref = NULL;

        return *this;
    }
};
#endif

}
}

#endif
