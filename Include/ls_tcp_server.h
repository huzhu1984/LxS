#ifndef LS_TCP_SERVER_H
#define LS_TCP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls_include.h"
#include "ls_log.h"

/* Maximun sessions can accepted. */
#define TCP_SERVER_MAX_SESSION              (10)

typedef struct CLASS_TCP_SERVER_T
{
    /*
     * Send data to TCP client.
     * pucData[in]: Buffer of data to send. (pucData!=NULL)
     * iLen[in]: Length data to send. (iLen>0)
     * iSession[in]: Corresponding to a TCP client, got in callback function <pfRecvCb>;
     * Return: Length of data send out or LS_ERR.
     */
    LS_S32 (*send_p)(struct CLASS_TCP_SERVER_T *pThis, LS_U8* pucData, LS_S32 iLen, LS_S32 iSession);

    /*
     * Broadcast data to all TCP client in connecting.
     * pucData[in]: Buffer of data to broadcast. (pucData!=NULL)
     * iLen[in]: Length data to broadcast. (iLen>0)
     * return: LS_OK or L_ERR
     */
    LS_S32 (*broadcast)(struct CLASS_TCP_SERVER_T *pThis, LS_U8* pucData, LS_S32 iLen);

    /*
     * Set log object.
     * pLog[in]: Log object.
     */
    LS_VOID (*setLog)(struct CLASS_TCP_SERVER_T *pThis, CLASS_LOG_T *pLog);

    /*
     * Get log object.
     * Return: Log object.
     */
    CLASS_LOG_T* (*getLog)(struct CLASS_TCP_SERVER_T *pThis);

    LS_VOID (*del)(struct CLASS_TCP_SERVER_T *pThis);

    PRIVATE_T *pPriv;
} CLASS_TCP_SERVER_T;

/*
 * Call back function for new packet is received.
 * pucPacket[in]: Buffer holding packet received, may be unavailable after return of this function.
 *                So, if you process it in other thread, make a copy.
 * iLen[in]: Length of data received.
 * iSession[in]: Session number for responsing back, you should pass it to method <send>.
 */
typedef LS_VOID (*TCP_SERVER_RECV_CB_FUNC)(LS_U8 *pucPacket, LS_S32 iLen, LS_S32 iSession, LS_VOID *pData);

/*
 * iPort[in]: Network port. (iPort > 1024)
 * pfRecvCb[in]: Call back function, packet will be ignored if pfRecvCB==NULL.
 * pData[in]: User data passed to <pfRecvCb>.
 */
CLASS_TCP_SERVER_T *new_CLASS_TCP_SERVER_T(LS_S32 iPort, TCP_SERVER_RECV_CB_FUNC pfRecvCb, LS_VOID *pData);

#ifdef __cplusplus
}
#endif

#endif // LS_TCP_SERVER_H
