#ifndef LS_COMMON_H
#define LS_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls_include.h"

#define LXS_VERSION             ("V1.1.0001")

LS_U16 cmnCrc16(LS_U8 *pucBuf, LS_U16 usLen);

LS_VOID cmnStrLower(LS_S8 *pszStr);
LS_VOID cmnStrUpper(LS_S8 *pszStr);
LS_S32 cmnStrCmpCnt(LS_S8 *pszStr1, LS_S8 *pszStr2);
LS_S32 cmnStrCmpNoCaseSpace(LS_S8 *pszStr1, LS_S8 *pszStr2);

LS_VOID cmnSprintfHex(LS_S8 *pszStr, LS_U8 *pucData, LS_S32 iSize);
LS_VOID cmnPrintfHex(LS_U8 *pucData, LS_S32 iSize);

#ifndef _WIN32
LS_VOID cmnGetSysTime(LS_S8 *pszBuf, LS_S32 iBufSize);
LS_S32 cmnGetIpAddr(LS_S8 *pszEthName, in_addr_t *pIpAddrOut);
LS_S32 cmnGetMacAddr(LS_S8 *pszEthName, LS_U8 *pucMacAddrOut);
LS_S32 cmnSetIpAddr(LS_S8 *pszEthName, in_addr_t ipAddr);
LS_S32 cmnSetMacAddr(LS_S8 *pszEthName, LS_U8 *pucMacAddr);
LS_S32 cmnRplValInFile(LS_S8 *pszOldFile, LS_S8 *pszToken, LS_S8 *pszDlm, LS_S8 *pszNewVal);
LS_S32 cmnSelectRead(LS_S32 iFd, LS_S32 iMs);
#endif

#define LS_CORDER_16(usVal)     (((usVal<<8)&0xFF00)|((usVal>>8)&0x00FF))
#define LS_CORDER_32(uiVal)     (((usVal<<24)&0xFF000000)|((usVal<<8)&0x00FF0000)| \
                                 ((usVal>>8)&0x0000FF00)|((usVal>>24)&0x000000FF)))

#define LS_CTOS(ucHigh, ucLow)  (((ucHigh<<8)&0xFF00)|(ucLow&0xFF00))
#define LS_STOI(usHigh, usLow)  (((usHigh<<16)&0xFFFF0000)|(usLow&0xFFFF))

#ifdef _WIN32
#define LS_CLOSE(fd)            do{if(INVALID_HANDLE_VALUE!=fd){CloseHandle(fd); fd=INVALID_HANDLE_VALUE;}}while(0)
#define LS_FCLOSE(p)            do{if(NULL!=p){fclose(p); p=NULL;}}while(0)
#define LS_FREE(p)              do{if(NULL!=p){free(p); p=NULL;}}while(0)

#define M_MUTEX_LOCK
#define M_MUTEX_UNLOCK
#else
#define LS_CLOSE(fd)            do{if(fd>0){close(fd); fd=-1;}}while(0)
#define LS_FCLOSE(p)            do{if(NULL!=p){fclose(p); p=NULL;}}while(0)
#define LS_FREE(p)              do{if(NULL!=p){free(p); p=NULL;}}while(0)

#define M_MUTEX_LOCK            do{pthread_mutex_lock(&(pPriv->lock));}while(0)
#define M_MUTEX_UNLOCK          do{pthread_mutex_unlock(&(pPriv->lock));}while(0)
#endif

#define LOG_PRINT(eType, fmt, args...) \
    do { \
        if (NULL != pPriv->pLog) \
        { \
            LS_CALL(pPriv->pLog, output, eType, fmt, ##args); \
        } \
    } while (0)

#define LOG_DEBUG(eType, fmt, args...) \
    do { \
        if (NULL != pPriv->pLog) \
        { \
            LS_CALL(pPriv->pLog, output, eType, "%s<%s>(L%d):"fmt"\r\n", \
            strrchr(__FILE__, '/')+1, __FUNCTION__, __LINE__, ##args); \
        } \
    } while (0)

#define LOG_TRACE \
    do { \
        if (NULL != pPriv->pLog) \
        { \
            LS_CALL(pThis->pLog, output, E_LOG_TYPE_TRACE, "%s<%s>(L%d)\r\n", \
            strrchr(__FILE__, '/')+1, __FUNCTION__, __LINE__); \
        } \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif // COMMON_H
