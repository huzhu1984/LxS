#include "ls_tcp_server.h"
#include "ls_list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define M_PACKET_BUF_SIZE       (1024)

typedef struct
{
    pthread_t threadSess;       /* Session thread */
    LS_S32 iSession;            /* ID of session */
    LS_S32 iSocket;             /* Socket of session */
    struct sockaddr stRemoteAddr; /* Address of remote client */
    LS_S32 iExpires;            /* How long an session expires withou receving a packet */
    LS_S32 iTimeouts;           /* How much times the session not receive a packet */
} SESSION_T;

struct PRIVATE_T
{
    LS_S32 iSocket;             /* TCP socket */
    LS_S32 iPort;               /* Port of network */
    pthread_t threadAccept;     /* Thread for accepting new connection */
    LS_BOOL bRunning;           /* Running flag of accept-thread */
    CLASS_LIST_T *pSession;     /* List of session */
    LS_S32 iSession;            /* ID of session, for allocation */
    LS_BOOL bSessStart;         /* New session thread start running */
    TCP_SERVER_RECV_CB_FUNC pfRecvCb; /* Call back funcion to process packet received */
    LS_S32 *pData;              /* User data */
    CLASS_LOG_T *pLog;          /* Log object */
};

/*
 * Get session from session list according to session ID.
 */
static SESSION_T* _getSession(struct CLASS_TCP_SERVER_T *pThis, LS_S32 iSession)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    SESSION_T *pstSession = NULL;

    /* Walk the list and find the matched session data. */
    LS_CALL(pPriv->pSession, startIter);
    while (NULL != (pstSession = LS_CALL(pPriv->pSession, getIterNext)))
    {
        if (iSession == pstSession->iSession)
        {
            break;
        }
    }

    return pstSession;
}

/*
 * Close session and remove it from session list according to session ID.
 */
static LS_S32 closeSession(struct CLASS_TCP_SERVER_T *pThis, LS_S32 iSession)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    SESSION_T *pstSession;

    /* Delete session from session list. */
    LS_CALL(pPriv->pSession, startIter);
    while (NULL != (pstSession = LS_CALL(pPriv->pSession, getIterNext)))
    {
        if (iSession == pstSession->iSession)
        {
            LS_CLOSE(pstSession->iSocket);
            LS_FREE(pstSession);
            LS_CALL(pPriv->pSession, removeIter);
            return LS_OK;
        }
    }

    return LS_ERR;
}

#define M_MSG_BUF_SIZE          (1024)

/*
 * Thread for new session's receiving packets.
 */
static LS_VOID* _threadSession(LS_VOID *arg)
{
    CLASS_TCP_SERVER_T *pThis = arg;
    PRIVATE_T *pPriv = pThis->pPriv;
    SESSION_T *pstSession;
    LS_U8 aucMsgBuf[M_MSG_BUF_SIZE] = {0};
    LS_S32 iRecvLen;
    struct sockaddr stRemoteAddr;
    socklen_t addrLen;
    pstSession = _getSession(pThis, pPriv->iSession);

    /* Make accept thread to be continued. */
    pPriv->bSessStart = LS_TRUE;

    while (pPriv->bRunning)
    {
        /* Poll the socket, if message comes, receive it. */
        if (cmnSelectRead(pPriv->iSocket, 100) >= 0)
        {
            addrLen = sizeof(struct sockaddr_in);
            iRecvLen = recvfrom(pPriv->iSocket, aucMsgBuf, M_MSG_BUF_SIZE,
                                0, &stRemoteAddr, &addrLen);

            if (iRecvLen > 0)
            {
                /* Packet is processed in callback function. */
                pPriv->pfRecvCb(aucMsgBuf, iRecvLen, pstSession->iSession, pPriv->pData);
            }

            if (iRecvLen < 0)
            {
                /* Session is broken, close this session. */
                closeSession(pThis, pstSession->iSession);
                pstSession = NULL;
                break;
            }
        }
    }

    return NULL;
}

/*
 * Thread for accepting new connection.
 */
static LS_VOID* _threadAccept(LS_VOID *arg)
{
    CLASS_TCP_SERVER_T *pThis = arg;
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_S32 iNewSocket;
    struct sockaddr stRemoteAddr;
    socklen_t addrLen;
    SESSION_T *pstSession;

    while (pPriv->bRunning)
    {
        if (-1 == (iNewSocket = accept(pPriv->iSocket, &stRemoteAddr, &addrLen)))
        {
            continue;
        }

        /* Record the client information in to a new session. */
        if (NULL == (pstSession = calloc(1, sizeof(SESSION_T))))
        {
            LS_CLOSE(iNewSocket);
            continue;
        }

        pPriv->iSession++;
        pstSession->iSession = pPriv->iSession;
        pstSession->iSocket = iNewSocket;
        memcpy(&(pstSession->stRemoteAddr), &stRemoteAddr, sizeof(struct sockaddr));

        /* Append the session to session list. */
        LS_CALL(pPriv->pSession, append, pstSession);

        /* Create thread for receiving and processing packets. */
        pPriv->bSessStart = LS_FALSE;
        if (0 != pthread_create(&(pstSession->threadSess), NULL, _threadSession, pThis))
        {
            closeSession(pThis, pstSession->iSession);
            continue;
        }

        /* Wait for the thread to get the session data. */
        while (LS_FALSE == pPriv->bSessStart) ;
        {
            usleep(10);
        }
    }

    return NULL;
}

LS_S32 send_p(struct CLASS_TCP_SERVER_T *pThis, LS_U8* pucData, LS_S32 iLen, LS_S32 iSession)
{
    SESSION_T *pstSession = NULL;

    if (NULL == pucData || iLen <= 0)
    {
        return LS_ERR;
    }

    if (NULL == (pstSession = _getSession(pThis, iSession)))
    {
        return LS_ERR;
    }

    return send(pstSession->iSocket, pucData, iLen, 0);
}

LS_S32 broadcast(struct CLASS_TCP_SERVER_T *pThis, LS_U8* pucData, LS_S32 iLen)
{
    SESSION_T *pstSession = NULL;
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_S32 iClientSend = 0;

    if (NULL == pucData || iLen <= 0)
    {
        return LS_ERR;
    }

    while (NULL != (pstSession = LS_CALL(pPriv->pSession, getIterNext)))
    {
        send(pstSession->iSocket, pucData, iLen, 0);
        iClientSend++;
    }

    return iClientSend;
}

static LS_VOID setLog(struct CLASS_TCP_SERVER_T *pThis, CLASS_LOG_T *pLog)
{
    pThis->pPriv->pLog = pLog;
    return;
}

static CLASS_LOG_T* getLog(struct CLASS_TCP_SERVER_T *pThis)
{
    return pThis->pPriv->pLog;
}

static LS_VOID* _delData(LS_VOID *pData)
{
    LS_FREE(pData);
    return pData;
}

static LS_VOID del(CLASS_TCP_SERVER_T *pThis)
{
    LS_VOID *ret;

    /* Set running flag to false and wait for the thread's return. */
    pThis->pPriv->bRunning = LS_FALSE;
    pthread_join(pThis->pPriv->threadAccept, &ret);

    LS_CLOSE(pThis->pPriv->iSocket);

    /* Free all sessions in list. */
    LS_CALL(pThis->pPriv->pSession, forEach, _delData);
    LS_DELETE(pThis->pPriv->pSession);
    LS_DELETE(pThis->pPriv->pLog);
    LS_FREE(pThis->pPriv);
    LS_FREE(pThis);

    return ;
}

CLASS_TCP_SERVER_T *new_CLASS_TCP_SERVER_T(LS_S32 iPort, TCP_SERVER_RECV_CB_FUNC pfRecvCb, LS_VOID *pData)
{
    CLASS_TCP_SERVER_T *pThis = NULL;
    struct sockaddr_in stHostAddr;

    if (iPort <= 1024)
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis = calloc(1, sizeof(CLASS_TCP_SERVER_T))))
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis->pPriv = calloc(1, sizeof(PRIVATE_T))))
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis->pPriv->pSession = LS_NEW(CLASS_LIST_T)))
    {
        goto ERR_LABEL;
    }

    pThis->pPriv->iSession = 0;

    /* Create and bind a TCP socket. */
    if (-1 == (pThis->pPriv->iSocket = socket(AF_INET, SOCK_STREAM, 0)))
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

    if (-1 == listen(pThis->pPriv->iSocket, TCP_SERVER_MAX_SESSION))
    {
        goto ERR_LABEL;
    }

    pThis->pPriv->pfRecvCb = pfRecvCb;
    pThis->pPriv->pData = pData;

    /* Set the running flag before the thread be activated. */
    pThis->pPriv->bRunning = LS_TRUE;
    if (0 != pthread_create(&(pThis->pPriv->threadAccept), NULL, _threadAccept, pThis))
    {
        goto ERR_LABEL;
    }

    pThis->send_p = send_p;
    pThis->broadcast = broadcast;
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
