#include "sys/msgwnd.hpp"
#include "sys/synch.hpp"
//#include "sys/uuid.hpp"
#include "util/memory.hpp"
#include "util/methodscope.hpp"
#include "msgqueue/MessageQueue.h"
#include "log/log.hpp"
#include <unistd.h>
#include <iostream>

using namespace LFS;

MsgWnd::MsgWnd(std::string path, std::function< void (DWORD, LPARAM) > f)
    : m_f(f),
      m_hWnd(path)
{

}

MsgWnd::~MsgWnd()
{

}

void MsgWnd::start()
{
    Log::Logger::streamInstance() << __FUNCTION__ << "\tobject_name:\t" << m_hWnd << std::endl;
    LFS::Synch::Semaphore sem(0,1);
    m_thread.reset(new std::thread([this, &sem](){
        MsgSeq msg;
        CMessageQueue messageQueue(897654321, 0x1000);
        while (true)
        {
            std::cout << "=== msg thread. " << std::this_thread::get_id() << std::endl;
            if(messageQueue.Read(&msg, sizeof(msg)) > 0)
            {
                if(m_f && msg.strObject_Name.compare(m_hWnd) == 0) {
                    m_f(msg.dwEventID, msg.lParam);
                }
            }
            usleep(50 * 1000);
        }

        sem.release();
    }));

    sem.acquire();
}

