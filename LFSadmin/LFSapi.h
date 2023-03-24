/************************************************************************
*                                                                       *
* LFSapi.h   - API的函数、类型与定义                                    *
*                                                                       *
* Version 1.00                                                          *
*                                                                       *
************************************************************************/

#ifndef __inc_LFSapi__h
#define __inc_LFSapi__h

#ifdef __cplusplus
extern "C" {
#endif

/* 字节对齐设置为1 */
#pragma pack(push,1)

#include <sys/types.h>
#include <pthread.h>

typedef void * HANDLE;
typedef void * LPVOID;
typedef char * LPSTR;
typedef wchar_t* LPWSTR;
typedef unsigned int DWORD;
typedef DWORD* LPDWORD;
typedef unsigned short WORD;
typedef WORD* LPWORD;
typedef unsigned char BYTE;
typedef BYTE* LPBYTE;
typedef long LONG;
typedef int BOOL;

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME;

#define LFSAPI                                  __attribute__((__cdecl__))

/****** 通用类型 *********************************************************/

typedef unsigned short USHORT;
typedef char CHAR;
typedef short SHORT;
typedef unsigned long ULONG;
typedef unsigned char UCHAR;
typedef SHORT * LPSHORT;
typedef LPVOID * LPLPVOID;
typedef ULONG * LPULONG;
typedef USHORT * LPUSHORT;
typedef void  VOID;

typedef HANDLE HPROVIDER;

typedef ULONG REQUESTID;
typedef REQUESTID * LPREQUESTID;

typedef HANDLE HAPP;
typedef HAPP * LPHAPP;

typedef USHORT HSERVICE;
typedef HSERVICE * LPHSERVICE;

typedef LONG HRESULT;
typedef HRESULT * LPHRESULT;

typedef DWORD THREADID; 

typedef BOOL (LFSAPI * LFSBLOCKINGHOOK)(void);
typedef LFSBLOCKINGHOOK * LPLFSBLOCKINGHOOK;

typedef LPSTR HWND;
typedef const CHAR *LPCSTR, *PCSTR,*LPCTSTR;
typedef LONG WPARAM;
typedef LONG LPARAM;
typedef unsigned int UINT;

/****** 消息名称与 DM_USER 定义 ****************************************/
#define LFS_MGR_MSG_NAME_PREF "org.lfs.manager"
#define LFS_MGR_MSG_INTF_NAME "org.lfs.compatible_intf"
#define LFS_MGR_MSG_METHOD_NAME "event_callback"

#define DM_USER                                 (0x0400)

#define TRUE                                     (1)
#define FALSE                                    (0)

/****** 字符串长度定义 ***************************************************/

#define LFSDDESCRIPTION_LEN                     256
#define LFSDSYSSTATUS_LEN                       256

/****** LFSDEVSTATUS.state 的取值 *****************************************/

#define LFS_STAT_DEVONLINE                      (0)
#define LFS_STAT_DEVOFFLINE                     (1)
#define LFS_STAT_DEVPOWEROFF                    (2)
#define LFS_STAT_DEVNODEVICE                    (3)
#define LFS_STAT_DEVHWERROR                     (4)
#define LFS_STAT_DEVUSERERROR                   (5)
#define LFS_STAT_DEVBUSY                        (6)
#define LFS_STAT_DEVFRAUDATTEMPT                (7)     
#define LFS_STAT_DEVPOTENTIALFRAUD              (8)  

/****** LFS_DEFAULT_HAPP 的值 ********************************************/

#define LFS_DEFAULT_HAPP                        (0)

/****** 数据结构定义 *****************************************************/

typedef struct _LFS_result
{
    REQUESTID       ulRequest_ID;
    HSERVICE        usService;
    SYSTEMTIME      tsTimestamp;
    HRESULT         lResult;
    union 
    {
        DWORD       dwCommand_Code;
        DWORD       dwEvent_ID;
    } u;
    LPVOID          lpvdBuffer;
} LFSRESULT, * LPLFSRESULT;

typedef struct _LFS_version
{
    WORD            wVersion;
    WORD            wLow_Version;
    WORD            wHigh_Version;
    CHAR            strDescription[LFSDDESCRIPTION_LEN+1];
    CHAR            strSystem_Status[LFSDSYSSTATUS_LEN+1];
} LFSVERSION, * LPLFSVERSION;

/****** 消息结构体定义 ***************************************************/

typedef struct _LFS_devstatus
{
    LPSTR           lpstrPhysical_Name;
    LPSTR           lpstrWorkstation_Name;
    DWORD           dwState;
} LFSDEVSTATUS, * LPLFSDEVSTATUS;

typedef struct _LFS_undevmsg
{
    LPSTR           lpstrLogical_Name;
    LPSTR           lpstrWorkstation_Name;
    LPSTR           lpstrApp_ID;
    DWORD           dwSize;
    LPBYTE          lpbDescription;
    DWORD           dwMsg;
    LPLFSRESULT     lpLFS_Result;
} LFSUNDEVMSG, * LPLFSUNDEVMSG;

typedef struct _LFS_appdisc
{
    LPSTR           lpstrLogical_Name;
    LPSTR           lpstrWorkstation_Name;
    LPSTR           lpstrApp_ID;
} LFSAPPDISC, * LPLFSAPPDISC;

typedef struct _LFS_hwerror
{
    LPSTR lpstrLogical_Name;
    LPSTR lpstrPhysical_Name;
    LPSTR lpstrWorkstation_Name;
    LPSTR lpstrApp_ID;
    DWORD dwAction;
    DWORD dwSize;
    LPBYTE lpbDescription;
} LFSHWERROR, * LPLFSHWERROR;

typedef struct _LFS_vrsnerror
{
    LPSTR lpstrLogical_Name;
    LPSTR lpstrWorkstation_Name;
    LPSTR lpstrApp_ID;
    DWORD dwSize;
    LPBYTE lpbDescription;
    LPLFSVERSION lpLFS_Version;
} LFSVRSNERROR, * LPLFSVRSNERROR;

/****** 错误码 ************************************************************/
#define LFS_SUCCESS                             (0)
#define LFS_ERR_ALREADY_STARTED                 (-1)
#define LFS_ERR_API_VER_TOO_HIGH                (-2)
#define LFS_ERR_API_VER_TOO_LOW                 (-3)
#define LFS_ERR_CANCELED                        (-4)
#define LFS_ERR_DEV_NOT_READY                   (-13)
#define LFS_ERR_HARDWARE_ERROR                  (-14)
#define LFS_ERR_INTERNAL_ERROR                  (-15)
#define LFS_ERR_INVALID_ADDRESS                 (-16)
#define LFS_ERR_INVALID_APP_HANDLE              (-17)
#define LFS_ERR_INVALID_BUFFER                  (-18)
#define LFS_ERR_INVALID_CATEGORY                (-19)
#define LFS_ERR_INVALID_COMMAND                 (-20)
#define LFS_ERR_INVALID_EVENT_CLASS             (-21)
#define LFS_ERR_INVALID_HSERVICE                (-22)
#define LFS_ERR_INVALID_HPROVIDER               (-23)
#define LFS_ERR_INVALID_MSG_OBJECT              (-24)
#define LFS_ERR_INVALID_MSG_REG_OBJECT          (-25)
#define LFS_ERR_INVALID_POINTER                 (-26)
#define LFS_ERR_INVALID_REQ_ID                  (-27)
#define LFS_ERR_INVALID_RESULT                  (-28)
#define LFS_ERR_INVALID_SERVPROV                (-29)
#define LFS_ERR_INVALID_TIMER                   (-30)
#define LFS_ERR_INVALID_TRACELEVEL              (-31)
#define LFS_ERR_LOCKED                          (-32)
#define LFS_ERR_NO_BLOCKING_CALL                (-33)
#define LFS_ERR_NO_SERVPROV                     (-34)
#define LFS_ERR_NO_SUCH_THREAD                  (-35)
#define LFS_ERR_NO_TIMER                        (-36)
#define LFS_ERR_NOT_LOCKED                      (-37)
#define LFS_ERR_NOT_OK_TO_UNLOAD                (-38)
#define LFS_ERR_NOT_STARTED                     (-39)
#define LFS_ERR_NOT_REGISTERED                  (-40)
#define LFS_ERR_OP_IN_PROGRESS                  (-41)
#define LFS_ERR_OUT_OF_MEMORY                   (-42)
#define LFS_ERR_SERVICE_NOT_FOUND               (-43)
#define LFS_ERR_SPI_VER_TOO_HIGH                (-44)
#define LFS_ERR_SPI_VER_TOO_LOW                 (-45)
#define LFS_ERR_SRVC_VER_TOO_HIGH               (-46)
#define LFS_ERR_SRVC_VER_TOO_LOW                (-47)
#define LFS_ERR_TIMEOUT                         (-48)
#define LFS_ERR_UNSUPP_CATEGORY                 (-49)
#define LFS_ERR_UNSUPP_COMMAND                  (-50)
#define LFS_ERR_VERSION_ERROR_IN_SRVC           (-51)
#define LFS_ERR_INVALID_DATA                    (-52)
#define LFS_ERR_SOFTWARE_ERROR                  (-53)
#define LFS_ERR_CONNECTION_LOST                 (-54)
#define LFS_ERR_USER_ERROR                      (-55)
#define LFS_ERR_UNSUPP_DATA                     (-56)

#define LFS_INDEFINITE_WAIT                     (0)

/****** 消息定义 **********************************************************/

#define LFS_OPEN_COMPLETE                       (DM_USER + 1)
#define LFS_CLOSE_COMPLETE                      (DM_USER + 2)
#define LFS_LOCK_COMPLETE                       (DM_USER + 3)
#define LFS_UNLOCK_COMPLETE                     (DM_USER + 4)
#define LFS_REGISTER_COMPLETE                   (DM_USER + 5)
#define LFS_DEREGISTER_COMPLETE                 (DM_USER + 6)
#define LFS_GETINFO_COMPLETE                    (DM_USER + 7)
#define LFS_EXECUTE_COMPLETE                    (DM_USER + 8)

#define LFS_EXECUTE_EVENT                       (DM_USER + 20)
#define LFS_SERVICE_EVENT                       (DM_USER + 21)
#define LFS_USER_EVENT                          (DM_USER + 22)
#define LFS_SYSTEM_EVENT                        (DM_USER + 23)

#define LFS_TIMER_EVENT                         (DM_USER + 100)

/****** 事件类型 **********************************************************/

#define SERVICE_EVENTS                          (1)
#define USER_EVENTS                             (2)
#define SYSTEM_EVENTS                           (4)
#define EXECUTE_EVENTS                          (8)

/****** 系统事件ID ********************************************************/

#define LFS_SYSE_UNDELIVERABLE_MSG              (1)
#define LFS_SYSE_HARDWARE_ERROR                 (2)
#define LFS_SYSE_VERSION_ERROR                  (3)
#define LFS_SYSE_DEVICE_STATUS                  (4)
#define LFS_SYSE_APP_DISCONNECT                 (5)
#define LFS_SYSE_SOFTWARE_ERROR                 (6)
#define LFS_SYSE_USER_ERROR                     (7)
#define LFS_SYSE_LOCK_REQUESTED                 (8)

/****** LFS 日志级别 ******************************************************/

#define LFS_TRACE_API                           0x00000001
#define LFS_TRACE_ALL_API                       0x00000002
#define LFS_TRACE_SPI                           0x00000004
#define LFS_TRACE_ALL_SPI                       0x00000008
#define LFS_TRACE_MGR                           0x00000010

/****** LFS 错误时采取的动作 *********************************************/

#define LFS_ERR_ACT_NOACTION                    (0x0000)
#define LFS_ERR_ACT_RESET                       (0x0001)
#define LFS_ERR_ACT_SWERROR                     (0x0002)
#define LFS_ERR_ACT_CONFIG                      (0x0004)
#define LFS_ERR_ACT_HWCLEAR                     (0x0008)
#define LFS_ERR_ACT_HWMAINT                     (0x0010)
#define LFS_ERR_ACT_SUSPEND                     (0x0020)

/****** API 函数 **********************************************************/

HRESULT extern LFSAPI LFSCancelAsyncRequest(HSERVICE usService, REQUESTID ulRequest_ID);

HRESULT extern LFSAPI LFSCancelBlockingCall(ULONG dwThread_ID);

HRESULT extern LFSAPI LFSCleanUp();

HRESULT extern LFSAPI LFSClose(HSERVICE usService);

HRESULT extern LFSAPI LFSAsyncClose(HSERVICE usService, LPSTR lpstrObject_Name, LPREQUESTID ulRequest_ID);

HRESULT extern LFSAPI LFSCreateAppHandle(LPHAPP lphdApp_Handle);

HRESULT extern LFSAPI LFSDeregister(HSERVICE usService, DWORD dwEvent_Class, LPSTR lpstrObject_Reg_Name);

HRESULT extern LFSAPI LFSAsyncDeregister(HSERVICE usService, DWORD dwEvent_Class, LPSTR lpstrObject_Reg_Name, LPSTR lpstrObject_Name, LPREQUESTID ulRequest_ID);

HRESULT extern LFSAPI LFSDestroyAppHandle(HAPP lpstrApp_Handle);

HRESULT extern LFSAPI LFSExecute(HSERVICE usService, DWORD dwCommand, LPVOID lpvdCmd_Data, DWORD dwTime_Out, LPLFSRESULT *lppResult);

HRESULT extern LFSAPI LFSAsyncExecute(HSERVICE usService, DWORD dwCommand, LPVOID lpvdCmd_Data, DWORD dwTime_Out, LPSTR lpstrObject_Name, LPREQUESTID ulRequest_ID);

HRESULT extern LFSAPI LFSFreeResult(LPLFSRESULT lpResult);

HRESULT extern LFSAPI LFSGetInfo(HSERVICE usService, DWORD dwCategory, LPVOID lpvdQuery_Details, DWORD dwTime_Out, LPLFSRESULT *lppResult);

HRESULT extern LFSAPI LFSAsyncGetInfo(HSERVICE usService, DWORD dwCategory, LPVOID lpvdQuery_Details, DWORD dwTime_Out, LPSTR lpstrObject_Name, LPREQUESTID ulRequest_ID);

BOOL extern LFSAPI LFSIsBlocking();

HRESULT extern LFSAPI LFSLock(HSERVICE usService, DWORD dwTime_Out, LPLFSRESULT *lppResult);

HRESULT extern LFSAPI LFSAsyncLock(HSERVICE usService, DWORD dwTime_Out, LPSTR lpstrObject_Name,  LPREQUESTID ulRequest_ID);

HRESULT extern LFSAPI LFSOpen(LPSTR lpstrLogical_Name, HAPP lpvdApp_Handle, LPSTR lpstrApp_ID, DWORD dwTrace_Level, DWORD dwTime_Out, DWORD dwSrvc_Versions_Required, LPLFSVERSION lpSrvc_Version , LPLFSVERSION lpSPI_Version, LPHSERVICE lpusService);

HRESULT extern LFSAPI LFSAsyncOpen(LPSTR lpstrLogical_Name, HAPP lpvdApp_Handle, LPSTR lpstrApp_ID, DWORD dwTrace_Level, DWORD dwTime_Out, LPHSERVICE lpusService, LPSTR lpstrObject_Name, DWORD dwSrvc_Versions_Required, LPLFSVERSION lpSrvc_Version, LPLFSVERSION lpSPI_Version, LPREQUESTID ulRequest_ID);

HRESULT extern LFSAPI LFSRegister(HSERVICE usService, DWORD dwEvent_Class, LPSTR lpstrObject_Reg_Name);

HRESULT extern LFSAPI LFSAsyncRegister(HSERVICE usService, DWORD dwEvent_Class, LPSTR lpstrObject_Reg_Name, LPSTR lpstrObject_Name, LPREQUESTID ulRequest_ID);

HRESULT extern LFSAPI LFSSetBlockingHook(LFSBLOCKINGHOOK lpBlock_Func, LPLFSBLOCKINGHOOK lppPrev_Func);

HRESULT extern LFSAPI LFSStartUp(DWORD dwVersions_Required, LPLFSVERSION lpLFS_Version);

HRESULT extern LFSAPI LFSUnhookBlockingHook();

HRESULT extern LFSAPI LFSUnlock(HSERVICE usService);

HRESULT extern LFSAPI LFSAsyncUnlock(HSERVICE usService, LPSTR lpstrObject_Name, LPREQUESTID ulRequest_ID);

HRESULT extern LFSAPI LFMSetTraceLevel(HSERVICE usService, DWORD dwTrace_Level);

/*   恢复字节对齐方式   */
#pragma pack(pop)

#ifdef __cplusplus
}       /*extern "C"*/
#endif

#endif  /* __inc_LFSapi__h */

