#include "LFSadmin.h"
#include <memory>
#include <map>
#include <list>
#include <string>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "sys/shmem.hpp"
#include "sys/library.hpp"
#include "sys/synch.hpp"
#include "lfs/version.hpp"
#include "util/memory.hpp"
#include "util/methodscope.hpp"
#include "util/constraints.hpp"
#include <boost/thread/thread.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <queue>
#include "sys/msgwnd.hpp"
#include "msgqueue/MessageQueue.h"
#include "inisetting.h"

//LFS_manager.ini
//service_providers
//  LFScam.ini
//logical_services
//  cam.ini
//extensions
//  ...
//machine.ini
//sp_connect_args.ini
#define PISA_MANAGER_CONFIG_PATH "/etc/LFS/"

//xxx.log
#define TRACE_FILE_PATH          "/var/log/LFS/"

#define SHARE_FILE_PATH          "/dev/shm/"

//aeye
//  xxx.log
#define SP_LOG_PATH              "var/log/SPLog"

//#ifdef __cplusplus
//extern "C" {
//#endif

/*   字节对齐设置为1   */
//#pragma pack(push,1)

struct ShMemLayout
{
    struct AppData
    {
        struct SPData
        {
            REQUESTID reqId;
            LFS::Library *spLib;
        };

        SPData hServices[8192];
        bool handles[8192];
    };

    AppData apps[64];
    DWORD pidTable[64];

    struct Timer
    {
        LPVOID lpContext;
        char object_name[64];
    };

    Timer timers[65535];
};

//HINSTANCE dllInstance = NULL;
named_mutex mutexHandle(open_or_create, "LFS_MANAGER_MUTEX");
class Context
{
    NON_COPYABLE(Context);
    NON_MOVEABLE(Context);

public:
    LFS::SharedMemory< ShMemLayout > shMem;
    std::map< void *, std::list< void * > > allocMap;
    //    std::map< WORD, std::tuple< HWND, LPVOID > > timers;
    std::map<std::string, CallBack> callbackMap;
    std::map<std::string, LFS::Synch::Semaphore*> semapMap;

    explicit Context() : shMem("LFS_MANAGER")
    {

    }
} *_ctx = nullptr;

void initializeContext()
{
    if (!_ctx) {
        boost::lock_guard<named_mutex> lock(mutexHandle);
        if (!_ctx)
            _ctx = new Context;
    }
}

static void GenerateInternalObjectName(LPSTR object_name)
{
    if(object_name != NULL) {
        struct tm* temptm;
        struct timeval time;
        gettimeofday(&time, NULL);
        temptm = localtime(&time.tv_sec);
        snprintf(object_name
                 , 256
                 , "/LFS/app000/listener_%04d%02d%02d%02d%02d%02d%03ld%03ld"
                 , temptm->tm_year + 1900
                 , temptm->tm_mon
                 , temptm->tm_mday
                 , temptm->tm_hour
                 , temptm->tm_min
                 , temptm->tm_sec
                 , time.tv_usec / 1000
                 , time.tv_usec % 1000);
    }
}

HRESULT LFSAPI LFMAllocateBuffer(ULONG ulSize, ULONG ulFlags, LPVOID *lppData)
{
    ::Log::Method m(__SIGNATURE__);

    initializeContext();

    boost::lock_guard<named_mutex> lock(mutexHandle);

    if (!lppData)
        return LFS_ERR_INVALID_POINTER;

    *lppData = malloc(ulSize);
    ::memset(*lppData, 0, ulSize);

    if (!*lppData)
        return LFS_ERR_OUT_OF_MEMORY;

    _ctx->allocMap.emplace(*lppData, std::list< void * >());

    return LFS_SUCCESS;
}

HRESULT LFSAPI LFMAllocateMore(ULONG ulSize, LPVOID lpvdOriginal, LPVOID *lppData)
{
    ::Log::Method m(__SIGNATURE__);

    initializeContext();

    boost::lock_guard<named_mutex> lock(mutexHandle);

    if (!lppData)
        return LFS_ERR_INVALID_POINTER;

    auto it = _ctx->allocMap.find(lpvdOriginal);
    if (it == _ctx->allocMap.cend())
        return LFS_ERR_INVALID_ADDRESS;

    *lppData = malloc(ulSize);
    ::memset(*lppData, 0, ulSize);
    if (!*lppData)
        return LFS_ERR_OUT_OF_MEMORY;

    _ctx->allocMap[lpvdOriginal].push_back(*lppData);

    return LFS_SUCCESS;
}

HRESULT LFSAPI LFMFreeBuffer(LPVOID lpvdData)
{
    ::Log::Method m(__SIGNATURE__);

    initializeContext();

    boost::lock_guard<named_mutex> lock(mutexHandle);

    if (!lpvdData)
        return LFS_ERR_INVALID_POINTER;

    auto it = _ctx->allocMap.find(lpvdData);
    if (it == _ctx->allocMap.cend())
        return LFS_ERR_INVALID_BUFFER;

    for (void *p : _ctx->allocMap.at(lpvdData))
        free(p);

    free(lpvdData);

    _ctx->allocMap.erase(it);

    return LFS_SUCCESS;
}

HRESULT LFSAPI LFMSetTraceLevel(HSERVICE usService, DWORD dwTrace_Level)
{
    initializeContext();
    return LFS_SUCCESS;
}

HRESULT LFSAPI LFMGetTraceLevel(HSERVICE usService, DWORD dwTrace_Level)
{
    initializeContext();
    return LFS_SUCCESS;
}

struct TimerParam
{
    char object_name[64];
    DWORD dwEventID;
    //    UINT wParam; //UNUSED
    //    LONG lParam;
    DWORD dwTime_Out;
};

void timerProc(union sigval v)
{
    TimerParam* tp = static_cast<TimerParam*>(v.sival_ptr);
    _ctx->shMem.access([tp] (ShMemLayout *p)
    {
        LFMPostMessage(tp->object_name, LFS_TIMER_EVENT, tp->dwEventID, 0, reinterpret_cast< LONG >(p->timers[tp->dwEventID].lpContext));
    });
    delete tp;
}

HRESULT LFSAPI LFMSetTimer(LPSTR object_name, LPVOID  context, DWORD timeval, ULONG* timer_id)
{
    initializeContext();

    if (!timer_id)
        return LFS_ERR_INVALID_POINTER;

    if (!timeval)
        return LFS_ERR_INVALID_DATA;

    HRESULT hRes = LFS_SUCCESS;
    _ctx->shMem.access([&hRes, object_name, context, timeval, timer_id] (ShMemLayout *p)
    {
        auto it = std::find_if(std::begin(p->timers),std::end(p->timers),[] (const ShMemLayout::Timer &t) { return (strlen(t.object_name)==0); });
        if (it == std::end(p->timers)) {
            hRes = LFS_ERR_INTERNAL_ERROR;
            return;
        }

        memcpy(it->object_name, object_name, strlen(object_name)+1);
        it->lpContext = context;

        TimerParam* tp = new TimerParam;
        memcpy(tp->object_name, object_name, strlen(object_name)+1);
        tp->dwEventID = static_cast<DWORD>(std::distance(std::begin(p->timers),it));
        //        tp->wParam = 0;
        //        tp->lParam = (LONG)context;
        tp->dwTime_Out = timeval;

        struct sigevent sigev;
        memset(&sigev, 0, sizeof(sigev));
        sigev.sigev_value.sival_int = 111;
        sigev.sigev_value.sival_ptr = static_cast<void*>(tp);
        sigev.sigev_notify = SIGEV_THREAD;
        sigev.sigev_notify_function = timerProc;

        timer_t timerid;
        if (timer_create(CLOCK_REALTIME, &sigev, &timerid) == -1) {
            hRes = LFS_ERR_INTERNAL_ERROR;
            memset(it->object_name, 0, sizeof(it->object_name));
            return;
        }

        struct itimerspec its;
        its.it_interval.tv_sec = timeval / 1000;
        its.it_interval.tv_nsec = timeval % 1000;
        its.it_value.tv_sec = timeval / 1000;
        its.it_value.tv_nsec = timeval % 1000;
        if(timer_settime(timerid, 0, &its, NULL) == -1) {
            hRes = LFS_ERR_INTERNAL_ERROR;
            memset(it->object_name, 0, sizeof(it->object_name));
            return;
        }

        //output timerid
        if(timer_id) {
            *timer_id = reinterpret_cast<ULONG>(timerid);
        }

        if(timeval == LFS_INDEFINITE_WAIT) {
            // wait until finished
        }
    });

    return hRes;
}

HRESULT LFSAPI LFMKillTimer(ULONG timer_id)
{
    initializeContext();

    HRESULT hRes = LFS_SUCCESS;
    _ctx->shMem.access([&hRes, timer_id]  (ShMemLayout *p)
    {
        if (strlen(p->timers[timer_id].object_name) == 0) {
            hRes = LFS_ERR_INVALID_TIMER;
            return;
        }

        timer_t tt = reinterpret_cast<timer_t>(timer_id);
        if (!timer_delete(tt)) {
            hRes = LFS_ERR_INTERNAL_ERROR;
            return;
        }

        memset(p->timers[timer_id].object_name, 0, sizeof(p->timers[timer_id].object_name));
    });

    return hRes;
}

HRESULT LFSAPI LFMOutputTraceData(LPSTR data)
{
    initializeContext();
    return LFS_SUCCESS;
}

HRESULT LFSAPI LFMReleaseLib(HPROVIDER provider)
{
    initializeContext();
    return LFS_SUCCESS;
}

////////////////////////////////// API //////////////////////////////////

#define CHECK_IF_STARTED \
    bool ok = false; \
    _ctx->shMem.access([&ok](ShMemLayout *p) \
{ \
    ok = (std::find(std::begin(p->pidTable),std::end(p->pidTable),getpid()) != std::end(p->pidTable)); \
    }); \
    \
    if (!ok) \
    return LFS_ERR_NOT_STARTED;

#define GET_LIB_AND_REQUEST \
    if (!usService) \
    return LFS_ERR_INVALID_HSERVICE; \
    \
    if (!ulRequest_ID) \
    return LFS_ERR_INVALID_POINTER; \
    \
    bool ok = false; \
    LFS::Library *lib = nullptr; \
    _ctx->shMem.access([&ok, &lib, usService, ulRequest_ID] (ShMemLayout *p) \
{ \
    auto it = std::find(std::begin(p->pidTable),std::end(p->pidTable),getpid()); \
    \
    if (it == std::end(p->pidTable)) \
    return; \
    \
    ok = true; \
    ShMemLayout::AppData::SPData &item = p->apps[std::distance(std::begin(p->pidTable),it)].hServices[usService - 1]; \
    lib = item.spLib; \
    *ulRequest_ID = item.reqId++; \
    }); \
    \
    if (!ok) \
    return LFS_ERR_NOT_STARTED; \
    \
    if (!lib) \
    return LFS_ERR_INVALID_HSERVICE;


class SynchMsgWnd : public LFS::MsgWnd
{
  NON_COPYABLE(SynchMsgWnd);

public:
  explicit SynchMsgWnd(std::string path, LFS::Synch::Semaphore &sem, DWORD msgId, LPLFSRESULT *lppResult = 0) :
    LFS::MsgWnd(path, [path, &sem, msgId, lppResult] (DWORD uMsg, LPARAM lParam)
      {
        if (uMsg == msgId) {
          if (lppResult != 0)
          {
            *lppResult = reinterpret_cast< LPLFSRESULT >(lParam);
            /*auto it = _ctx->callbackMap.find(path);
            if(it != _ctx->callbackMap.end())
            {
                CallBack cb = (CallBack)it->second;
                (cb)((char*)path.c_str(), msgId, 0, lParam);
            }*/
          }

          sem.release();
        }
      }) {}
};

HRESULT /*extern*/ LFSAPI LFSCancelAsyncRequest(HSERVICE usService, REQUESTID ulRequest_ID)
{
    initializeContext();

    if (!usService)
        return LFS_ERR_INVALID_HSERVICE;

    bool ok = false;
    LFS::Library *lib = nullptr;
    _ctx->shMem.access([&ok, &lib, usService] (ShMemLayout *p)
    {
        auto it = std::find(std::begin(p->pidTable),std::end(p->pidTable), getpid());
        if (it == std::end(p->pidTable))
            return;

        ok = true;
        lib = p->apps[std::distance(std::begin(p->pidTable),it)].hServices[usService - 1].spLib;
    });

    if (!ok)
        return LFS_ERR_NOT_STARTED;

    if (!lib)
        return LFS_ERR_INVALID_HSERVICE;

    return lib->call< HRESULT >("LFPCancelAsyncRequest",usService,ulRequest_ID);
}

HRESULT /*extern*/ LFSAPI LFSCancelBlockingCall(ULONG dwThread_ID)
{
    initializeContext();

    CHECK_IF_STARTED

    return LFS_SUCCESS;
}

HRESULT /*extern*/ LFSAPI LFSCleanUp()
{
    initializeContext();

    bool ok = false;
    _ctx->shMem.access([&ok] (ShMemLayout *p)
    {
        auto it = std::find(std::begin(p->pidTable),std::end(p->pidTable),getpid());
        if (it != std::end(p->pidTable)) {
            *it = 0;
            ok = true;
        }
    });

    return (ok)? LFS_SUCCESS : LFS_ERR_NOT_STARTED;
}

HRESULT /*extern*/ LFSAPI LFSClose(HSERVICE usService)
{
    initializeContext();

    CHECK_IF_STARTED

    LFS::Synch::Semaphore sem(0,1);
    HRESULT hRes = LFS_SUCCESS;

    char object_name[256] = {0};
    GenerateInternalObjectName(object_name);

    SynchMsgWnd wnd(object_name, sem, LFS_CLOSE_COMPLETE);
    wnd.start();

    REQUESTID reqId;
    if ((hRes = LFSAsyncClose(usService, object_name, &reqId)) != LFS_SUCCESS) {
        return hRes;
    }

    sem.acquire();

    return hRes;
}

HRESULT /*extern*/ LFSAPI LFSAsyncClose(HSERVICE usService, LPSTR lpstrObject_Name, LPREQUESTID ulRequest_ID)
{
    initializeContext();

    GET_LIB_AND_REQUEST

    return lib->call< HRESULT >("LFPClose",usService, lpstrObject_Name, *ulRequest_ID);
}

HRESULT /*extern*/ LFSAPI LFSCreateAppHandle(LPHAPP lphdApp_Handle)
{
    initializeContext();

    ::Log::Method m(__SIGNATURE__);
    
    if (!lphdApp_Handle)
        return LFS_ERR_INVALID_POINTER;

    bool ok = false;
    _ctx->shMem.access([&ok, lphdApp_Handle] (ShMemLayout *p)
    {
        auto it = std::find(std::begin(p->pidTable),std::end(p->pidTable), getpid());
        if ((ok = (it != std::end(p->pidTable)))) {
            ShMemLayout::AppData &item = p->apps[std::distance(std::begin(p->pidTable),it)];
            auto ait = std::find(std::begin(item.handles),std::end(item.handles),false);
            if (ait != std::end(item.handles)) {
                *lphdApp_Handle = reinterpret_cast< HAPP >(std::distance(std::begin(item.handles),ait) + 1);
                *ait = true;
            }
        }
    });

    if (!ok)
        return LFS_ERR_NOT_STARTED;

    return (*lphdApp_Handle)? LFS_SUCCESS : LFS_ERR_INTERNAL_ERROR;
}

HRESULT /*extern*/ LFSAPI LFSDeregister(HSERVICE usService, DWORD dwEvent_Class, LPSTR lpstrObject_Reg_Name)
{
    initializeContext();

    CHECK_IF_STARTED

    LFS::Synch::Semaphore sem(0,1);
    HRESULT hRes = LFS_SUCCESS;

    char object_name[256] = {0};
    GenerateInternalObjectName(object_name);

    SynchMsgWnd wnd((object_name), sem, LFS_DEREGISTER_COMPLETE);
    wnd.start();

    REQUESTID reqId;
    if ((hRes = LFSAsyncDeregister(usService,dwEvent_Class,lpstrObject_Reg_Name,object_name,&reqId)) != LFS_SUCCESS)
        return hRes;

    sem.acquire();

    return hRes;
}

HRESULT /*extern*/ LFSAPI LFSAsyncDeregister(HSERVICE usService, DWORD dwEvent_Class, LPSTR lpstrObject_Reg_Name, LPSTR lpstrObject_Name, LPREQUESTID ulRequest_ID)
{
    initializeContext();

    if (!lpstrObject_Reg_Name || strlen(lpstrObject_Reg_Name)==0)
        return LFS_ERR_INVALID_MSG_OBJECT;

    GET_LIB_AND_REQUEST

    return lib->call< HRESULT >("LFPDeregister",usService,dwEvent_Class,lpstrObject_Reg_Name,lpstrObject_Name,*ulRequest_ID);
}

HRESULT /*extern*/ LFSAPI LFSDestroyAppHandle(HAPP lpstrApp_Handle)
{
    initializeContext();

    if (!lpstrApp_Handle)
        return LFS_ERR_INVALID_APP_HANDLE;

    long idx = reinterpret_cast< long >(lpstrApp_Handle) - 1;

    bool ok = false;
    _ctx->shMem.access([&ok, &idx] (ShMemLayout *p)
    {
        auto it = std::find(std::begin(p->pidTable),std::end(p->pidTable),getpid());

        if ((ok = (it != std::end(p->pidTable)))) {
            ShMemLayout::AppData &item = p->apps[std::distance(std::begin(p->pidTable),it)];
            if (item.handles[idx]) {
                item.handles[idx] = false;
                idx = 0;
            }
        }
    });

    if (!ok)
        return LFS_ERR_NOT_STARTED;

    return (!idx)? LFS_SUCCESS : LFS_ERR_INVALID_APP_HANDLE;
}

HRESULT /*extern*/ LFSAPI LFSExecute(HSERVICE usService, DWORD dwCommand, LPVOID lpvdCmd_Data, DWORD dwTime_Out, LPLFSRESULT *lppResult)
{
    initializeContext();

    CHECK_IF_STARTED

    LFS::Synch::Semaphore sem(0,1);
    HRESULT hRes = LFS_SUCCESS;

    char object_name[256] = {0};
    GenerateInternalObjectName(object_name);

    SynchMsgWnd wnd((object_name), sem, LFS_EXECUTE_COMPLETE);
    wnd.start();

    REQUESTID reqId;
    if ((hRes = LFSAsyncExecute(usService,dwCommand,lpvdCmd_Data,dwTime_Out,object_name,&reqId)) != LFS_SUCCESS)
        return hRes;

    sem.acquire();

    return hRes;
}

HRESULT /*extern*/ LFSAPI LFSAsyncExecute(HSERVICE usService, DWORD dwCommand, LPVOID lpvdCmd_Data, DWORD dwTime_Out, LPSTR lpstrObject_Name, LPREQUESTID ulRequest_ID)
{
    initializeContext();

    GET_LIB_AND_REQUEST

    return lib->call< HRESULT >("LFPExecute",usService,dwCommand,lpvdCmd_Data,dwTime_Out,lpstrObject_Name,*ulRequest_ID);
}

HRESULT /*extern*/ LFSAPI LFSFreeResult(LPLFSRESULT lpResult)
{
    initializeContext();

    CHECK_IF_STARTED

    return LFMFreeBuffer(lpResult);
}

HRESULT /*extern*/ LFSAPI LFSGetInfo(HSERVICE usService, DWORD dwCategory, LPVOID lpvdQuery_Details, DWORD dwTime_Out, LPLFSRESULT *lppResult)
{
    ::Log::Method m(__SIGNATURE__);
    initializeContext();

    CHECK_IF_STARTED

    LFS::Synch::Semaphore sem(0,1);
    HRESULT hRes = LFS_SUCCESS;

    char object_name[256] = {0};
    GenerateInternalObjectName(object_name);

    SynchMsgWnd wnd((object_name), sem, LFS_GETINFO_COMPLETE);
    wnd.start();

    REQUESTID reqId;
    if ((hRes = LFSAsyncGetInfo(usService,dwCategory,lpvdQuery_Details,dwTime_Out,object_name,&reqId)) != LFS_SUCCESS)
        return hRes;

    sem.acquire();

    return hRes;

}

HRESULT /*extern*/ LFSAPI LFSAsyncGetInfo(HSERVICE usService, DWORD dwCategory, LPVOID lpvdQuery_Details, DWORD dwTime_Out, LPSTR lpstrObject_Name, LPREQUESTID ulRequest_ID)
{
    initializeContext();

    GET_LIB_AND_REQUEST

    ::Log::Method m(__SIGNATURE__, lpstrObject_Name);

    return lib->call< HRESULT >("LFPGetInfo",usService,dwCategory,lpvdQuery_Details,dwTime_Out,lpstrObject_Name,*ulRequest_ID);
}

BOOL /*extern*/ LFSAPI LFSIsBlocking()
{
    initializeContext();

    CHECK_IF_STARTED

    return FALSE;
}

HRESULT /*extern*/ LFSAPI LFSLock(HSERVICE usService, DWORD dwTime_Out, LPLFSRESULT *lppResult)
{
    initializeContext();

    if (!lppResult)
        return LFS_ERR_INVALID_POINTER;

    CHECK_IF_STARTED

    LFS::Synch::Semaphore sem(0,1);
    HRESULT hRes = LFS_SUCCESS;

    char object_name[256] = {0};
    GenerateInternalObjectName(object_name);

    SynchMsgWnd wnd((object_name), sem, LFS_LOCK_COMPLETE);
    wnd.start();

    REQUESTID reqId;
    if ((hRes = LFSAsyncLock(usService,dwTime_Out,object_name,&reqId)) != LFS_SUCCESS)
        return hRes;

    sem.acquire();

    return hRes;
}

HRESULT /*extern*/ LFSAPI LFSAsyncLock(HSERVICE usService, DWORD dwTime_Out, LPSTR lpstrObject_Name,  LPREQUESTID ulRequest_ID)
{
    initializeContext();

    GET_LIB_AND_REQUEST

    return lib->call< HRESULT >("LFPLock",usService,dwTime_Out,lpstrObject_Name,*ulRequest_ID);
}

HRESULT /*extern*/ LFSAPI LFSOpen(LPSTR lpstrLogical_Name, HAPP lpvdApp_Handle, LPSTR lpstrApp_ID, DWORD dwTrace_Level, DWORD dwTime_Out, DWORD dwSrvc_Versions_Required
                                  , LPLFSVERSION lpSrvc_Version , LPLFSVERSION lpSPI_Version, LPHSERVICE lpusService)
{
    initializeContext();

    ::Log::Method m(__SIGNATURE__, lpstrLogical_Name);

    if (!lpvdApp_Handle)
        return LFS_ERR_INVALID_APP_HANDLE;

    if (!lpstrLogical_Name || !lpstrApp_ID || !lpusService || !lpSrvc_Version || !lpSPI_Version)
        return LFS_ERR_INVALID_POINTER;

    CHECK_IF_STARTED

    LFS::Synch::Semaphore sem(0,1);
    HRESULT hRes = LFS_SUCCESS;

    char object_name[256] = {0};
    GenerateInternalObjectName(object_name);
    
    SynchMsgWnd wnd((object_name), sem, LFS_OPEN_COMPLETE);
    wnd.start();

    REQUESTID reqId;
    if ((hRes = LFSAsyncOpen(lpstrLogical_Name,lpvdApp_Handle,lpstrApp_ID,dwTrace_Level,dwTime_Out
                             ,lpusService,object_name,dwSrvc_Versions_Required,lpSrvc_Version,lpSPI_Version,&reqId)) != LFS_SUCCESS)
        return hRes;

    sem.acquire();

    return hRes;
}

HRESULT /*extern*/ LFSAPI LFSAsyncOpen(LPSTR lpstrLogical_Name, HAPP lpvdApp_Handle, LPSTR lpstrApp_ID, DWORD dwTrace_Level, DWORD dwTime_Out, LPHSERVICE lpusService
                                       , LPSTR lpstrObject_Name, DWORD dwSrvc_Versions_Required, LPLFSVERSION lpSrvc_Version, LPLFSVERSION lpSPI_Version, LPREQUESTID ulRequest_ID)
{
    initializeContext();

    std::cout << "LFSAsyncOpen " << __LINE__ << std::endl;
    /*if (!lpvdApp_Handle)
        return LFS_ERR_INVALID_APP_HANDLE;

    std::cout << "LFSAsyncOpen " << __LINE__ << std::endl;
    if (!lpstrLogical_Name || !lpvdApp_Handle || !lpusService || !lpSrvc_Version || !lpSPI_Version || !ulRequest_ID)
        return LFS_ERR_INVALID_POINTER;*/

    std::cout << "LFSAsyncOpen " << __LINE__ << std::endl;
    *lpusService = 0;
    *ulRequest_ID = 0;
    clearMem(*lpSPI_Version);
    clearMem(*lpSrvc_Version);

    LFS::Library *lib = nullptr;
    HRESULT hRes = LFS_SUCCESS;
    _ctx->shMem.access([&lib, &hRes, &lpvdApp_Handle, lpusService, ulRequest_ID, lpstrLogical_Name] (ShMemLayout *p)
    {
        std::cout << "LFSAsyncOpen " << __LINE__ << std::endl;
        auto it = std::find(std::begin(p->pidTable),std::end(p->pidTable),getpid());

        if (it == std::end(p->pidTable))
        {
            hRes = LFS_ERR_NOT_STARTED;
            return;
        }

        std::cout << "LFSAsyncOpen " << __LINE__ << std::endl;
        ShMemLayout::AppData &item = p->apps[std::distance(std::begin(p->pidTable),it)];

        /*if (!item.handles[reinterpret_cast< long >(lpvdApp_Handle) - 1]) {
            lpvdApp_Handle = NULL;
            hRes = LFS_ERR_INVALID_APP_HANDLE;
            return;
        }*/

        auto sit = std::find_if(std::begin(item.hServices),std::end(item.hServices),[] (const ShMemLayout::AppData::SPData &x) { return x.reqId == 0; });
        /*if (sit == std::end(item.hServices)) {
            hRes = LFS_ERR_INTERNAL_ERROR;
            return;
        }*/

        sit->reqId = 1;

        std::string libPath = inisetting::getInstance()->getSPIPath(lpstrLogical_Name);
        try
        {
            lib = sit->spLib = new LFS::Library(libPath);
            ::Log::Method m(__SIGNATURE__, libPath);
        }
        catch (const LFS::Exception &e)
        {
            hRes = LFS_ERR_INVALID_SERVPROV;
            return;
        }

        *ulRequest_ID = 1;
        *lpusService = static_cast< HSERVICE >(std::distance(std::begin(item.hServices),sit) + 1);
    });

    if (hRes != LFS_SUCCESS)
        return hRes;

    return lib->call< HRESULT >("LFPOpen",
                                *lpusService,
                                lpstrLogical_Name,
                                lpvdApp_Handle,
                                lpstrApp_ID,
                                dwTrace_Level,
                                dwTime_Out,
                                lpstrObject_Name,
                                *ulRequest_ID,
                                lib->handle(),
                                LFS::VersionRange(LFS::Version(3,0),LFS::Version(3,30)).value(),
                                lpSPI_Version,
                                dwSrvc_Versions_Required,
                                lpSrvc_Version);
}

HRESULT /*extern*/ LFSAPI LFSRegister(HSERVICE usService, DWORD dwEvent_Class, LPSTR lpstrObject_Reg_Name)
{
    initializeContext();

    CHECK_IF_STARTED

    LFS::Synch::Semaphore sem(0,1);
    HRESULT hRes = LFS_SUCCESS;

    char object_name[256] = {0};
    GenerateInternalObjectName(object_name);

    SynchMsgWnd wnd((object_name), sem, LFS_REGISTER_COMPLETE);
    wnd.start();

    REQUESTID reqId;
    if ((hRes = LFSAsyncRegister(usService,dwEvent_Class,lpstrObject_Reg_Name,object_name,&reqId)) != LFS_SUCCESS)
        return hRes;

    sem.acquire();

    return hRes;

}

HRESULT /*extern*/ LFSAPI LFSAsyncRegister(HSERVICE usService, DWORD dwEvent_Class, LPSTR lpstrObject_Reg_Name, LPSTR lpstrObject_Name, LPREQUESTID ulRequest_ID)
{
    initializeContext();

    if (!lpstrObject_Reg_Name)
        return LFS_ERR_INVALID_MSG_REG_OBJECT;

    GET_LIB_AND_REQUEST

    return lib->call< HRESULT >("LFPRegister",usService,dwEvent_Class,lpstrObject_Reg_Name,lpstrObject_Name,*ulRequest_ID);
}

HRESULT /*extern*/ LFSAPI LFSSetBlockingHook(LFSBLOCKINGHOOK lpBlock_Func, LPLFSBLOCKINGHOOK lppPrev_Func)
{
    initializeContext();

    CHECK_IF_STARTED

    return LFS_SUCCESS;
}

HRESULT /*extern*/ LFSAPI LFSStartUp(DWORD dwVersions_Required, LPLFSVERSION lpLFS_Version)
{
    initializeContext();
    std::cout << "lfsstartup" << std::endl;
#if 0
    LFS::VersionRange vr(dwVersions_Required);

    if (vr.start() > vr.end())
        return dwVersions_Required;

    if (vr.start() > LFS::Version(3,20))
        return LFS_ERR_API_VER_TOO_HIGH;

    if (vr.end() < LFS::Version(3,20))
        return LFS_ERR_API_VER_TOO_LOW;

    if (!lpLFS_Version)
        return LFS_ERR_INVALID_POINTER;

    clearMem(*lpLFS_Version);
    std::cout << "lfsstartup 876" << std::endl;

    lpLFS_Version->wVersion = LFS::Version(3,20).value();
    lpLFS_Version->wLow_Version = LFS::Version::min(3).value();
    lpLFS_Version->wHigh_Version = LFS::Version::max(3).value();
    std::cout << "lfsstartup 881" << std::endl;
    lpLFS_Version->strSystem_Status[0] = '\0';
    strcpy(lpLFS_Version->strDescription,"LFS Manager");
#endif
    ::Log::Method m(__SIGNATURE__);

    HRESULT hRes = LFS_SUCCESS;
    _ctx->shMem.access([&hRes] (ShMemLayout *p)
    {
        auto it = std::find(std::begin(p->pidTable),std::end(p->pidTable),getpid());
        if (it != std::end(p->pidTable)) {
            hRes = LFS_ERR_ALREADY_STARTED;
            return;
        }

        it = std::find(std::begin(p->pidTable),std::end(p->pidTable),0);
        if (it == std::end(p->pidTable)) {
            hRes = LFS_ERR_INTERNAL_ERROR;
            return;
        }

        *it = getpid();
        clearMem(p->apps[std::distance(std::begin(p->pidTable),it)]);
    });

    return hRes;
}

HRESULT /*extern*/ LFSAPI LFSUnhookBlockingHook()
{
    initializeContext();

    CHECK_IF_STARTED

    return LFS_SUCCESS;
}

HRESULT /*extern*/ LFSAPI LFSUnlock(HSERVICE usService)
{
    initializeContext();

    CHECK_IF_STARTED

    LFS::Synch::Semaphore sem(0,1);
    HRESULT hRes = LFS_SUCCESS;

    char object_name[256] = {0};
    GenerateInternalObjectName(object_name);

    SynchMsgWnd wnd((object_name), sem, LFS_UNLOCK_COMPLETE);
    wnd.start();

    REQUESTID reqId;
    if ((hRes = LFSAsyncUnlock(usService,object_name,&reqId)) != LFS_SUCCESS)
        return hRes;

    sem.acquire();

    return hRes;
}

HRESULT /*extern*/ LFSAPI LFSAsyncUnlock(HSERVICE usService, LPSTR lpstrObject_Name, LPREQUESTID ulRequest_ID)
{
    initializeContext();

    GET_LIB_AND_REQUEST

    return lib->call< HRESULT >("LFPUnlock",usService,lpstrObject_Name,*ulRequest_ID);
}

HRESULT /*extern*/ LFSAPI LFMUnRegCallBack(LPSTR lpstrObject_Name)
{
    ::Log::Method m(__SIGNATURE__, lpstrObject_Name);

    initializeContext();

    CHECK_IF_STARTED

    return LFS_SUCCESS;
}

HRESULT /*extern*/ LFSAPI LFMRegCallBack(LPSTR lpstrObject_Name, CallBack *pcallback)
{
    initializeContext();

    CHECK_IF_STARTED

    ::Log::Method m(__SIGNATURE__, lpstrObject_Name);

    _ctx->callbackMap[std::string(lpstrObject_Name)] = *pcallback;

    return LFS_SUCCESS;
}

#define MESSAGE_QUEUE_TRACE

HRESULT /*extern*/ LFSAPI LFMPostMessage(LPSTR lpstrObject_Name, DWORD dwEventID, UINT wParam, LONG lParam, DWORD dwTime_Out)
{
    ::Log::Method m(__SIGNATURE__, lpstrObject_Name);

    initializeContext();

    CHECK_IF_STARTED
    
    std::thread a([=](){
        auto it = _ctx->callbackMap.find(lpstrObject_Name);
        if(it != _ctx->callbackMap.end())
        {
            CallBack cb = (CallBack)it->second;
            if(cb)
                (cb)(lpstrObject_Name, dwEventID, wParam, lParam);
        }
    });
    a.detach();

#if 0
    MsgSeq msg;
    msg.strObject_Name = std::string(lpstrObject_Name);
    msg.dwEventID = dwEventID;
    msg.wParam = wParam;
    msg.lParam = lParam;

    CMessageQueue messageQueue(897654321, 0x1000);
    int n = messageQueue.Write(&msg, sizeof(msg));
    std::cout << "messageQueue Write ret: " << n << std::endl;
#endif
    return LFS_SUCCESS;
}
/*   恢复字节对齐方式   */
//#pragma pack(pop)

//#ifdef __cplusplus
//}       /*extern "C"*/
//#endif
