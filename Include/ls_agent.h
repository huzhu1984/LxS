#ifndef LS_AGENT_H
#define LS_AGENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls_include.h"
#include "ls_log.h"

/* Message head. */
typedef struct
{
    LS_U32 uiOpcode;            /* Operation code */
    LS_S32 iPayloadLen;         /* Length of payload */
} __attribute__((packed)) LS_AGENT_MSG_HEAD_T;

/*
 * Callback funcion for processing the message received.
 * pucMsg[in]: Buffer holding message received.
 * pResp[in]: Data for calling method <resp>, note that the data maybe not valid
 *            after <AGENT_MSG_CB_FUNC_F> return.
 */
typedef LS_S32 (*AGENT_MSG_CB_FUNC_F)(LS_U8* pucMsg, LS_VOID *pResp, LS_VOID *pData);

/* Parameter needed to register an opcode to agent. */
typedef struct
{
    LS_U32 uiOpcode;            /* Operation code */
    LS_S32 iPayloadLen;         /* Length of payload */
    AGENT_MSG_CB_FUNC_F pfCb;   /* Call back function */
    LS_VOID *pData;             /* User data passed to <pfCb> */
} LS_AGENT_REG_T;

typedef struct CLASS_AGENT_T
{
    /*
     * Register a callback function for an operation-code, if [uiOpcode] inf head of
     * a received massage is matched, <pstReg->pfCb> will be called.
     * pReg[in]: Register information.
     * Return: LS_OK or LS_ERR.
     */
    LS_S32 (*reg)(struct CLASS_AGENT_T *pThis, LS_AGENT_REG_T *pReg);

    /*
     * Response to client in a callback function.
     * pucBuf[in]: Data to response(pucBuf!=NULL).
     * iLen[in]: Length of data to response(iLen>0).
     * pResp[in]: Response data passed to the call back function(pResp!=NULL).
     */
    LS_S32 (*resp)(struct CLASS_AGENT_T *pThis, LS_U8 *pucBuf, LS_S32 iLen, LS_VOID *pResp);

    /*
     * Set log object.
     * pLog[in]: Log object.
     */
    LS_VOID (*setLog)(struct CLASS_AGENT_T *pThis, CLASS_LOG_T *pLog);

    /*
     * Get log object.
     * Return: Log object.
     */
    CLASS_LOG_T* (*getLog)(struct CLASS_AGENT_T *pThis);

    LS_VOID (*del)(struct CLASS_AGENT_T *pThis);

    PRIVATE_T *pPriv;
} CLASS_AGENT_T;

/* Type of communication used in new agent. */
typedef enum
{
    E_AGENT_COMM_STREAM = 0,    /* TCP/IP */
    E_AGENT_COMM_DGRAM,         /* UDP/IP */
    E_AGENT_COMM_CNT
} AGENT_COMM_TYPE_EN;

/*
 * eCommType[in]: Communication type.
 * iPort[in]: Network port(iPort<1024).
 */
CLASS_AGENT_T *new_CLASS_AGENT_T(AGENT_COMM_TYPE_EN eCommType, LS_S32 iPort);

#ifdef __cplusplus
}
#endif

#endif // LS_AGENT_H
