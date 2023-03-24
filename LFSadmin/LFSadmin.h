/******************************************************************************
*                                                                             *
* LFSadmin.h  - 管理与支撑函数                                                *
*                                                                             *
* Version 1.00                                                                *
*                                                                             *
******************************************************************************/

#ifndef __INC_LFSADMIN__H
#define __INC_LFSADMIN__H

#ifdef __cplusplus
extern "C" {
#endif

#include    "LFSapi.h"

/*   字节对齐设置为1   */
#pragma pack(push,1)

/* LFMAllocateBuffer 中flags参数的取值 */

#define LFS_MEM_SHARE                        0x00000001
#define LFS_MEM_ZEROINIT                     0x00000002

typedef VOID (*CallBack)(LPSTR lpstrObject_Name, DWORD dwEventID, UINT wParam, LONG lParam);

/****** 支撑函数 *************************************************************/

HRESULT extern LFSAPI LFMAllocateBuffer(ULONG ulSize, ULONG ulFlags, LPVOID *lppData);

HRESULT extern LFSAPI LFMAllocateMore(ULONG ulSize, LPVOID lpvdOriginal, LPVOID *lppData);

HRESULT extern LFSAPI LFMFreeBuffer(LPVOID lpvdData);

HRESULT extern LFSAPI LFMSetTraceLevel(HSERVICE usService, DWORD dwTrace_Level);

HRESULT extern LFSAPI LFMGetTraceLevel(HSERVICE usService, DWORD dwTrace_Level);

HRESULT extern LFSAPI LFMSetTimer(LPSTR object_name, LPVOID  context, DWORD timeval, ULONG* timer_id);

HRESULT extern LFSAPI LFMKillTimer(ULONG timer_id);

HRESULT extern LFSAPI LFMOutputTraceData(LPSTR data);

HRESULT extern LFSAPI LFMReleaseLib(HPROVIDER provider);

HRESULT extern LFSAPI LFMGetMessage(LPSTR lpstrObject_Name, DWORD* dwEventID, UINT* wParam, LONG* lParam, DWORD dwTime_Out);

HRESULT extern LFSAPI LFMPostMessage(LPSTR lpstrObject_Name, DWORD dwEventID, UINT wParam, LONG lParam, DWORD dwTime_Out);

HRESULT extern LFSAPI LFMRegCallBack(LPSTR lpstrObject_Name, CallBack *pcallback);

HRESULT extern LFSAPI LFMUnRegCallBack(LPSTR lpstrObject_Name);



/*   恢复字节对齐方式   */
#pragma pack(pop)

#ifdef __cplusplus
}       /*extern "C"*/
#endif

#endif    /* __INC_LFSADMIN__H */
