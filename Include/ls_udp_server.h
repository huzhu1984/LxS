#ifndef LS_UDP_SERVER_H
#define LS_UDP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls_include.h"
#include "ls_log.h"

typedef struct CLASS_UDP_SERVER_T
{
    /*
     * Send data to remote UDP client.
     * pucBuf[in]: Buffer of data to send. (pucData!=NULL)
     * iLen[in]: Length data to send. (iLen>0)
     * pstRemote[in]: Remote UDP address, got in callback function <pfRecvCb>;
     * Return: Length of data send out or LS_ERR.
     */
    LS_S32 (*sendto_p)(struct CLASS_UDP_SERVER_T *pThis, LS_U8 *pucBuf, LS_S32 iLen, struct sockaddr *pstRemote);

    LS_VOID (*del)(struct CLASS_UDP_SERVER_T *pThis);

    PRIVATE_T *pPriv;
} CLASS_UDP_SERVER_T;

typedef LS_VOID (*UDP_SERVER_RECV_CB_FUNC)(LS_U8 *pucPacket, LS_S32 iLen, struct sockaddr *pstRemote, LS_VOID *pData);


/*
 * iPort[in]: Network port. (iPort > 1024)
 * pfRecvCb[in]: Call back function, packet will be ignored if pfRecvCB==NULL.
 * pData[in]: User data passed to <pfRecvCb>.
 */
CLASS_UDP_SERVER_T *new_CLASS_UDP_SERVER_T(LS_S32 iPort, UDP_SERVER_RECV_CB_FUNC pfRecvCb, LS_VOID *pData);

#ifdef __cplusplus
}
#endif

#endif // LS_UDP_SERVER_H
