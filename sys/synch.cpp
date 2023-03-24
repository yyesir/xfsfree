/* sys/synch.cpp
 *
 * Copyright (C) 2007 Antonio Di Monaco
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "synch.hpp"


using namespace LFS::Synch;
#if 0
Lockable::Lockable(HANDLE h) : Handle(h)
{

}

void Lockable::lock()
{
  if (WaitForSingleObjectEx(handle(),INFINITE,FALSE) == WAIT_FAILED)
    throw Exception();
}

Mutex::Mutex(const std::wstring &sName)
    : Lockable(CreateMutex(NULL,FALSE,(sName.empty())? NULL : (std::wstring(L"Local\\") + sName).c_str()))
{

}

void Mutex::unlock()
{
  if (!ReleaseMutex(handle()))
    throw Exception();
}
#endif
Semaphore::Semaphore(LONG start, LONG max, const std::string &sName)
{
    sem_init(&m_sem, 0, 0);
}

void Semaphore::acquire()
{
    int ret = sem_wait(&m_sem);
}

void Semaphore::release()
{
    int ret = sem_post(&m_sem);
}

#if 0
Event::Event(const std::wstring &sName)
    : Handle(CreateEvent(NULL,TRUE,FALSE,(sName.empty())? NULL : (std::wstring(L"Local\\") + sName).c_str()))
{

}

void Event::waitFor()
{
  if (WaitForSingleObjectEx(handle(),INFINITE,FALSE) == WAIT_FAILED)
    throw Exception();
}

void Event::set()
{
  if (!SetEvent(handle()))
    throw Exception();
}
#endif
