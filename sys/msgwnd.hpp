#ifndef MSG_WND_HPP__H
#define MSG_WND_HPP__H

#include <functional>
#include <memory>
#include <thread>
//#include "win32/thread.hpp"
#include "util/constraints.hpp"
#include "sys/synch.hpp"
#include "util/constraints.hpp"
#include "LFSapi.h"

typedef struct
{
    std::string strObject_Name;
    DWORD dwEventID;
    UINT wParam;
    LONG lParam;
}MsgSeq;

namespace LFS
{
class MsgWnd
{
    NON_COPYABLE(MsgWnd);
public:
    explicit MsgWnd(std::string path, std::function< void (DWORD, LPARAM) > f);
    ~MsgWnd();

public:
    void start();

private:
    std::function< void (DWORD, LPARAM) > m_f;
    std::string m_hWnd;
    std::unique_ptr<std::thread> m_thread;
};
}

#endif
