#include "ls_modbus_client.h"

#ifdef __cplusplus
extern "C" {
#endif

#define M_LOG_BUF_LEN           (255)
#define M_MSG_BUF_LEN           (256+9)

struct PRIVATE_T
{
#ifndef _WIN32
    pthread_mutex_t lock;       /* Mutex lock */
#endif
    LS_U8 ucMbAddr;             /* Remote server address */
    MODBUS_RECV_FUNC pfRecv;    /* Function for sending data */
    MODBUS_SEND_FUNC pfSend;    /* Function for receving data */
    LS_VOID *pData;             /* User data */
    CLASS_LOG_T *pLog;          /* Log object */
};

typedef struct
{
    LS_U8 ucMbAddr;
    LS_U8 ucFuncCode;
    LS_U16 usRegAddr;
    LS_U16 usRegCnt;
    LS_U16 usCrc;
} __attribute__((packed)) FRAME_READ_REG_SEND_T, FRAME_WRITE_REG_RECV_T;

#ifdef _WIN32
#define MODBUS_USLEEP(us)       Sleep(us/1000)
#define MODBUS_HTONS(usVal)     LS_CORDER_16(usVal)
#define MODBUS_NTOHS(usVal)     LS_CORDER_16(usVal)
#else
#define MODBUS_USLEEP(us)       usleep(us)
#define MODBUS_HTONS(usVal)     htons(usVal)
#define MODBUS_NTOHS(usVal)     ntohs(usVal)
#endif

static LS_S32 readReg(CLASS_MODBUS_CLIENT_T *pThis, LS_U16 usRegAddr, LS_U16 usRegCnt, LS_U8 *pucOut)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    FRAME_READ_REG_SEND_T stSendFrame;
    LS_U8 aucRecvBuf[M_MSG_BUF_LEN] = {0};
    LS_S8 szHexBuf[M_LOG_BUF_LEN] = {0};
    LS_S32 iReadLen, iLen;
    LS_S32 i;
    LS_U16 usVal;

    M_MUTEX_LOCK;

    iReadLen = usRegCnt * 2 + 5;

    if (0 == usRegCnt || iReadLen > M_MSG_BUF_LEN || NULL == pucOut)
    {
        LOG_DEBUG(E_LOG_TYPE_DEBUG, "Error param: %d 0x%x", usRegCnt, (LS_U32)pucOut);
        M_MUTEX_UNLOCK;
        return LS_ERR;
    }

    stSendFrame.ucMbAddr = pPriv->ucMbAddr;
    stSendFrame.ucFuncCode = 0x03;
    stSendFrame.usRegAddr = MODBUS_HTONS(usRegAddr);
    stSendFrame.usRegCnt = MODBUS_HTONS(usRegCnt);
    stSendFrame.usCrc = MODBUS_HTONS(cmnCrc16((LS_U8*)&stSendFrame, sizeof(stSendFrame)-2));

    MODBUS_USLEEP(100000);

    iLen = pPriv->pfSend((LS_U8*)&stSendFrame, sizeof(stSendFrame), pPriv->pData);
    if (iLen > 0)
    {
        cmnSprintfHex(szHexBuf, (LS_U8*)&stSendFrame, iLen);
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Send data(%d):\r\n%s", iLen, szHexBuf);
    }
    else
    {
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Send data failed(%d).", iLen);
    }

    MODBUS_USLEEP(100000);

    iLen = pPriv->pfRecv(aucRecvBuf, iReadLen, pPriv->pData);
    if (iLen > 0)
    {
        cmnSprintfHex(szHexBuf, aucRecvBuf, iLen);
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Recv data(%d):\r\n%s", iLen, szHexBuf);
    }
    else
    {
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Recv data failed(%d).", iLen);
    }

    if (iLen <= 0)
    {
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_TIMEOUT;
    }

    if (iReadLen != iLen)
    {
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_LENGTH;
    }

    if (aucRecvBuf[0] != pPriv->ucMbAddr)
    {
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_ADDR;
    }

    if (aucRecvBuf[1] != 0x03)
    {
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_FUNCC;
    }

    memcpy(&usVal, &aucRecvBuf[iReadLen-2], sizeof(usVal));
    usVal = MODBUS_NTOHS(usVal);
    if (usVal != cmnCrc16(aucRecvBuf, iReadLen-2))
    {
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_CRC;
    }

    for (i=3; i<iReadLen-2; i+=2)
    {
        memcpy(&usVal, &aucRecvBuf[i], 2);
        usVal = MODBUS_NTOHS(usVal);
        memcpy(&pucOut[i-3], &usVal, 2);
    }

    M_MUTEX_UNLOCK;

    return LS_OK;
}

static LS_S32 writeReg(CLASS_MODBUS_CLIENT_T *pThis, LS_U16 usRegAddr, LS_U16 usRegCnt, LS_U8 *pucIn)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    FRAME_WRITE_REG_RECV_T stRecvFrame;
    LS_U8 aucSendBuf[M_MSG_BUF_LEN] = {0};
    LS_S8 szHexBuf[M_LOG_BUF_LEN] = {0};
    LS_S32 iWriteLen, iLen;
    LS_S32 i;
    LS_U16 usVal;

    M_MUTEX_LOCK;

    iWriteLen = usRegCnt * 2 + 9;

    if (0 == usRegCnt || iWriteLen > M_MSG_BUF_LEN || NULL == pucIn)
    {
        LOG_DEBUG(E_LOG_TYPE_DEBUG, "Error param: %d 0x%x", usRegCnt, (LS_U32)pucIn);
        M_MUTEX_UNLOCK;
        return LS_ERR;
    }

    aucSendBuf[0] = pPriv->ucMbAddr;

    aucSendBuf[1] = 0x10;

    usVal = MODBUS_HTONS(usRegAddr);
    memcpy(&aucSendBuf[2], &usVal, sizeof(usVal));

    usVal = MODBUS_HTONS(usRegCnt);
    memcpy(&aucSendBuf[4], &usVal, sizeof(usVal));

    aucSendBuf[6] = (LS_U8)(usRegCnt*2);

    for (i=7; i<iWriteLen-2; i+=2)
    {
        memcpy(&usVal, &pucIn[i-7], 2);
        usVal = MODBUS_HTONS(usVal);
        memcpy(&aucSendBuf[i], &usVal, 2);
    }

    usVal = MODBUS_HTONS(cmnCrc16(aucSendBuf, iWriteLen-2));
    memcpy(&aucSendBuf[iWriteLen-2], &usVal, sizeof(usVal));

    iLen = pPriv->pfSend(aucSendBuf, iWriteLen, pPriv->pData);
    if (iLen > 0)
    {
        cmnSprintfHex(szHexBuf, aucSendBuf, iLen);
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Send data(%d):\r\n%s", iLen, szHexBuf);
    }
    else
    {
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Send data failed(%d).", iLen);
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_SEND;
    }

    iLen = pPriv->pfRecv((LS_U8*)&stRecvFrame, sizeof(stRecvFrame), pPriv->pData);
    if (iLen > 0)
    {
        cmnSprintfHex(szHexBuf, (LS_U8*)&stRecvFrame, iLen);
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Recv data(%d):\r\n%s", iLen, szHexBuf);
    }
    else
    {
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Read data failed(%d).", iLen);
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_TIMEOUT;
    }

    if (sizeof(stRecvFrame) != iLen)
    {
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_LENGTH;
    }

    stRecvFrame.usRegAddr = MODBUS_NTOHS(stRecvFrame.usRegAddr);
    stRecvFrame.usRegCnt = MODBUS_NTOHS(stRecvFrame.usRegCnt);
    stRecvFrame.usCrc = MODBUS_NTOHS(stRecvFrame.usCrc);

    if (stRecvFrame.ucMbAddr != pPriv->ucMbAddr)
    {
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_ADDR;
    }

    if (stRecvFrame.ucFuncCode != 0x10)
    {
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_FUNCC;
    }

    if (stRecvFrame.usCrc != cmnCrc16((LS_U8*)&stRecvFrame, sizeof(stRecvFrame)-2))
    {
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_CRC;
    }

    M_MUTEX_UNLOCK;

    return LS_OK;
}

static LS_S32 writeRegSig(CLASS_MODBUS_CLIENT_T *pThis, LS_U16 usRegAddr, LS_U8 *pucIn)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    FRAME_WRITE_REG_RECV_T stRecvFrame;
    LS_U8 aucSendBuf[M_MSG_BUF_LEN] = {0};
    LS_S8 szHexBuf[M_LOG_BUF_LEN] = {0};
    LS_S32 iWriteLen, iLen;
    LS_U16 usVal;

    M_MUTEX_LOCK;

    iWriteLen = 2 + 6;

    if (NULL == pucIn)
    {
        LOG_DEBUG(E_LOG_TYPE_DEBUG, "Error param: 0x%x", pucIn);
        M_MUTEX_UNLOCK;
        return LS_ERR;
    }

    aucSendBuf[0] = pPriv->ucMbAddr;

    aucSendBuf[1] = 0x6;

    usVal = MODBUS_HTONS(usRegAddr);
    memcpy(&aucSendBuf[2], &usVal, sizeof(usVal));

    memcpy(&usVal, pucIn, 2);
    usVal = MODBUS_HTONS(usVal);
    memcpy(&aucSendBuf[4], &usVal, 2);

    usVal = MODBUS_HTONS(cmnCrc16(aucSendBuf, iWriteLen-2));
    memcpy(&aucSendBuf[iWriteLen-2], &usVal, sizeof(usVal));

    iLen = pPriv->pfSend(aucSendBuf, iWriteLen, pPriv->pData);
    if (iLen > 0)
    {
        cmnSprintfHex(szHexBuf, aucSendBuf, iLen);
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Send data(%d):\r\n%s", iLen, szHexBuf);
    }
    else
    {
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Send data failed(%d).", iLen);
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_SEND;
    }

    iLen = pPriv->pfRecv((LS_U8*)&stRecvFrame, sizeof(stRecvFrame), pPriv->pData);
    if (iLen > 0)
    {
        cmnSprintfHex(szHexBuf, (LS_U8*)&stRecvFrame, iLen);
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Recv data(%d):\r\n%s", iLen, szHexBuf);
    }
    else
    {
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Read data failed(%d).", iLen);
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_TIMEOUT;
    }

    if (sizeof(stRecvFrame) != iLen)
    {
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_LENGTH;
    }

    stRecvFrame.usRegAddr = MODBUS_NTOHS(stRecvFrame.usRegAddr);
    stRecvFrame.usRegCnt = MODBUS_NTOHS(stRecvFrame.usRegCnt);
    stRecvFrame.usCrc = MODBUS_NTOHS(stRecvFrame.usCrc);

    if (stRecvFrame.ucMbAddr != pPriv->ucMbAddr)
    {
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_ADDR;
    }

    if (stRecvFrame.ucFuncCode != 0x06)
    {
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_FUNCC;
    }

    if (stRecvFrame.usCrc != cmnCrc16((LS_U8*)&stRecvFrame, sizeof(stRecvFrame)-2))
    {
        M_MUTEX_UNLOCK;
        return M_MODBUS_ERRC_CRC;
    }

    M_MUTEX_UNLOCK;

    return LS_OK;
}

/*
 * Set log object.
 */
static LS_VOID setLog(struct CLASS_MODBUS_CLIENT_T *pThis, CLASS_LOG_T *pLog)
{
    pThis->pPriv->pLog = pLog;
    return;
}

/*
 * Get log object.
 */
static CLASS_LOG_T* getLog(struct CLASS_MODBUS_CLIENT_T *pThis)
{
    return pThis->pPriv->pLog;
}

static LS_VOID del(CLASS_MODBUS_CLIENT_T *pThis)
{
    LS_DELETE(pThis->pPriv->pLog);
    LS_FREE(pThis->pPriv);
    LS_FREE(pThis);

    return;
}

CLASS_MODBUS_CLIENT_T *new_CLASS_MODBUS_CLIENT_T(LS_U8 ucMbAddr,
                                                 MODBUS_RECV_FUNC pfRecv,
                                                 MODBUS_SEND_FUNC pfSend,
                                                 LS_VOID *pData)
{
    CLASS_MODBUS_CLIENT_T *pThis = NULL;

    if (NULL == pfRecv || NULL == pfSend)
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis = calloc(1, sizeof(CLASS_MODBUS_CLIENT_T))))
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis->pPriv = calloc(1, sizeof(PRIVATE_T))))
    {
        goto ERR_LABEL;
    }

    pThis->pPriv->ucMbAddr = ucMbAddr;
    pThis->pPriv->pfRecv = pfRecv;
    pThis->pPriv->pfSend = pfSend;
    pThis->pPriv->pData = pData;

#ifndef _WIN32
    pthread_mutex_init(&(pThis->pPriv->lock), NULL);
#endif

    pThis->readReg = readReg;
    pThis->writeReg = writeReg;
    pThis->writeRegSig = writeRegSig;
    pThis->setLog = setLog;
    pThis->getLog = getLog;
    pThis->del = del;

    return pThis;

ERR_LABEL:
    del(pThis);

    return NULL;
}

#ifdef __cplusplus
}
#endif

