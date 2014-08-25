#include "ls_udp_server.h"

#ifdef __cplusplus
extern "C" {
#endif

#define M_PACKET_BUF_SIZE       (1024)

struct PRIVATE_T
{
    LS_S32 iSocket;             /* UDP socket */
    LS_S32 iPort;               /* Port of network */
    LS_BOOL bRunning;           /* Running flag of UDP server */
    UDP_SERVER_RECV_CB_FUNC pfRecvCb;   /* Callback function when a packet is received */
    LS_VOID *pData;             /* User datas */
    pthread_t thread;           /* Thread of UDP server */
};

/*
 * Thread for receiving and processing network message.
 */
static LS_VOID *_thread(LS_VOID *arg)
{
    CLASS_UDP_SERVER_T *pThis = arg;
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_U8 aucMsgBuf[M_PACKET_BUF_SIZE] = {0};
    LS_S32 iRecvLen;
    struct sockaddr stRemoteAddr;
    socklen_t addrLen;

    while (pPriv->bRunning)
    {
        /* Poll the socket, if message comes, receive it. */
        while (cmnSelectRead(pPriv->iSocket, 0) > 0)
        {
            addrLen = sizeof(struct sockaddr_in);
            iRecvLen = recvfrom(pPriv->iSocket, aucMsgBuf, M_PACKET_BUF_SIZE,
                                0, &stRemoteAddr, &addrLen);

            if (iRecvLen > 0)
            {
                pPriv->pfRecvCb(aucMsgBuf, iRecvLen, &stRemoteAddr, pPriv->pData);
            }
        }

        sleep(1);
    }

    return NULL;
}

static LS_S32 sendto_p(CLASS_UDP_SERVER_T *pThis, LS_U8 *pucBuf, LS_S32 iLen, struct sockaddr *pstRemote)
{
    PRIVATE_T *pPriv = pThis->pPriv;

    if (NULL == pucBuf || 0 == iLen || NULL == pstRemote)
    {
        return LS_ERR;
    }

    return sendto(pPriv->iSocket, pucBuf, iLen, 0, pstRemote, sizeof(struct sockaddr));
}

static LS_VOID del(CLASS_UDP_SERVER_T *pThis)
{
    LS_VOID *ret;

    /* Set running flag to false and wait for the thread's return. */
    if (LS_TRUE == pThis->pPriv->bRunning)
    {
        pThis->pPriv->bRunning = LS_FALSE;
        pthread_join(pThis->pPriv->thread, &ret);
    }

    LS_CLOSE(pThis->pPriv->iSocket);

    LS_FREE(pThis->pPriv);
    LS_FREE(pThis);

    return;
}

CLASS_UDP_SERVER_T *new_CLASS_UDP_SERVER_T(LS_S32 iPort, UDP_SERVER_RECV_CB_FUNC pfRecvCb, LS_VOID *pData)
{
    CLASS_UDP_SERVER_T *pThis = NULL;
    struct sockaddr_in stHostAddr;

    if (iPort <= 1024)
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis = calloc(1, sizeof(CLASS_UDP_SERVER_T))))
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis->pPriv = calloc(1, sizeof(PRIVATE_T))))
    {
        goto ERR_LABEL;
    }

    pThis->pPriv->pfRecvCb = pfRecvCb;
    pThis->pPriv->pData = pData;

    /* Create and bind a UDP socket. */
    if (-1 == (pThis->pPriv->iSocket = socket(AF_INET, SOCK_DGRAM, 0)))
    {
        goto ERR_LABEL;
    }

    stHostAddr.sin_family = AF_INET;
    stHostAddr.sin_port = htons(iPort);
    stHostAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (-1 == bind(pThis->pPriv->iSocket, (struct sockaddr*)&stHostAddr, sizeof(struct sockaddr)))
    {
        goto ERR_LABEL;
    }

    /* Create thread to receive packets. */
    pThis->pPriv->bRunning = LS_TRUE;
    if (0 != pthread_create(&(pThis->pPriv->thread), NULL, _thread, pThis))
    {
        pThis->pPriv->bRunning = LS_FALSE;
        goto ERR_LABEL;
    }

    pThis->sendto_p = sendto_p;
    pThis->del = del;

    return pThis;

ERR_LABEL:
    del(pThis);

    return NULL;
}

#ifdef __cplusplus
}
#endif
