#include "ls_agent.h"
#include "ls_tcp_server.h"
#include "ls_udp_server.h"
#include "ls_list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define M_MSG_BUF_SIZE          (1024)

struct PRIVATE_T
{
    AGENT_COMM_TYPE_EN eCommType;   /* Type of communication */
    CLASS_TCP_SERVER_T *pTcpServer; /* TCP server for receiving message */
    CLASS_UDP_SERVER_T *pUdpServer; /* UDP server for receiving message */
    CLASS_LIST_T *pOpCodeList;      /* List of registered opcode */
    CLASS_LOG_T *pLog;              /* Log object */
};

/*
 * Process a message.
 */
static LS_VOID _procMsg(CLASS_AGENT_T *pThis, LS_U8 *pucBuf, LS_S32 iLen, LS_VOID *pResp)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_AGENT_MSG_HEAD_T stMsgHead;
    LS_AGENT_REG_T *pstReg;
    LS_U8 *pucPayload = NULL;
    LS_S32 iDataLen = 0;

    /* Check length of message. */
    iDataLen = iLen - sizeof(stMsgHead);
    if (iDataLen < 0)
    {
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Recv error length: Total(%d)", iLen);
        return;
    }

    if (iDataLen > 0)
    {
        pucPayload = &pucBuf[sizeof(stMsgHead)];
    }

    memcpy(&stMsgHead, pucBuf, sizeof(stMsgHead));
    stMsgHead.uiOpcode = ntohl(stMsgHead.uiOpcode);
    stMsgHead.iPayloadLen = ntohl(stMsgHead.iPayloadLen);

    /* Check length of payload. */
    if ((LS_U32)iLen != sizeof(stMsgHead) + stMsgHead.iPayloadLen)
    {
        LOG_DEBUG(E_LOG_TYPE_TRACE, "Error payload length: Net(%d)", iLen);
        return;
    }

    /* Walk the list, if operation codes are match, call the callback function. */
    LS_CALL(pPriv->pOpCodeList, startIter);
    while (NULL != (pstReg = LS_CALL(pPriv->pOpCodeList, getIterNext)))
    {
        if (pstReg->uiOpcode == stMsgHead.uiOpcode)
        {
            if (pstReg->iPayloadLen != stMsgHead.iPayloadLen)
            {
                LOG_DEBUG(E_LOG_TYPE_TRACE, "Error payload length: Host(%d)-Net(%d)",
                          pstReg->iPayloadLen, stMsgHead.iPayloadLen);
                return;
            }

            if (NULL == pstReg->pfCb)
            {
                return;
            }

            pstReg->pfCb(pucPayload, pResp, pstReg->pData);
        }
    }

    return;
}

/*
 * Received a TCP packet.
 */
static LS_VOID _recvTCP(LS_U8 *pucPacket, LS_S32 iLen, LS_S32 iSession, LS_VOID *pData)
{
    _procMsg((CLASS_AGENT_T*)pData, pucPacket, iLen, (LS_VOID*)&iSession);
}

/*
 * Received an UDP packet.
 */
static LS_VOID _recvUDP(LS_U8 *pucPacket, LS_S32 iLen, struct sockaddr *pstRemote, LS_VOID *pData)
{
    _procMsg((CLASS_AGENT_T*)pData, pucPacket, iLen, (LS_VOID*)pstRemote);
}

static LS_S32 resp(struct CLASS_AGENT_T *pThis, LS_U8 *pucBuf, LS_S32 iLen, LS_VOID *pResp)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_S32 iRet = LS_ERR;
    LS_S32 *pSession = pResp;

    if (NULL == pucBuf || 0 == iLen || NULL == pResp)
    {
        return LS_ERR;
    }

    switch (pPriv->eCommType)
    {
    case E_AGENT_COMM_STREAM:
        iRet = LS_CALL(pPriv->pTcpServer, send_p, pucBuf, iLen, *pSession);
        break;
    case E_AGENT_COMM_DGRAM:
        iRet = LS_CALL(pPriv->pUdpServer, sendto_p, pucBuf, iLen, (struct sockaddr*)pResp);
        break;
    default:
        break;
    }

    return iRet;
}

static LS_S32 reg(CLASS_AGENT_T *pThis, LS_AGENT_REG_T *pstReg)
{
    PRIVATE_T *pPriv = pThis->pPriv;

    if (NULL == pstReg)
    {
        return LS_ERR;
    }

    /* Walk the list, make sure no duplicate opcodes are accepted. */
    LS_CALL(pPriv->pOpCodeList, startIter);
    while (NULL != (pstReg = LS_CALL(pPriv->pOpCodeList, getIterNext)))
    {
        if (pstReg->uiOpcode == pstReg->uiOpcode)
        {
            LOG_DEBUG(E_LOG_TYPE_ERROR, "Opcode has been registered.");
            return LS_ERR;
        }
    }

    LS_CALL(pPriv->pOpCodeList, append, pstReg);

    return LS_OK;
}

static LS_VOID setLog(struct CLASS_AGENT_T *pThis, CLASS_LOG_T *pLog)
{
    pThis->pPriv->pLog = pLog;
    return;
}

static CLASS_LOG_T* getLog(struct CLASS_AGENT_T *pThis)
{
    return pThis->pPriv->pLog;
}

static LS_VOID del(CLASS_AGENT_T *pThis)
{
    LS_DELETE(pThis->pPriv->pTcpServer);
    LS_DELETE(pThis->pPriv->pUdpServer);
    LS_DELETE(pThis->pPriv->pOpCodeList);
    LS_DELETE(pThis->pPriv->pLog);
    LS_FREE(pThis->pPriv);
    LS_FREE(pThis);

    return;
}

CLASS_AGENT_T *new_CLASS_AGENT_T(AGENT_COMM_TYPE_EN eCommType, LS_S32 iPort)
{
    CLASS_AGENT_T *pThis = NULL;

    if (eCommType >= E_AGENT_COMM_CNT || iPort <= 1024)
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis = calloc(1, sizeof(CLASS_AGENT_T))))
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis->pPriv = calloc(1, sizeof(PRIVATE_T))))
    {
        goto ERR_LABEL;
    }

    /* Create communication server. */
    switch (eCommType)
    {
    case E_AGENT_COMM_STREAM:
        if (NULL == (pThis->pPriv->pTcpServer = LS_NEW(CLASS_TCP_SERVER_T, iPort, _recvTCP, pThis)))
        {
            goto ERR_LABEL;
        }
        break;
    case E_AGENT_COMM_DGRAM:
        if (NULL == (pThis->pPriv->pUdpServer = LS_NEW(CLASS_UDP_SERVER_T, iPort, _recvUDP, pThis)))
        {
            goto ERR_LABEL;
        }
        break;
    default:
        goto ERR_LABEL;
    }

    /* Create list for operation-code. */
    if (NULL == (pThis->pPriv->pOpCodeList = LS_NEW(CLASS_LIST_T)))
    {
        goto ERR_LABEL;
    }

    pThis->pPriv->eCommType = eCommType;

    pThis->reg = reg;
    pThis->resp = resp;
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
