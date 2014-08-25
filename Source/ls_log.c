#include "ls_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define M_TIME_BUF_LEN                  (63)

struct PRIVATE_T
{
#ifndef _WIN32
    pthread_mutex_t lock;               /* Mutex lock */
#endif
    LS_BOOL bMasked;                    /* Output mask */
    LS_U8 aucFlag[E_LOG_TYPE_CNT];      /* Output flag */
    LS_S8 szLogBuf[M_LOG_BUF_SIZE + 1]; /* Log buffer */
};

/*
 * Print string to TTY.
 */
static LS_VOID _toTTY(CLASS_LOG_T *pThis, LS_S8 *pszStr)
{
    LS_S8 szTime[M_TIME_BUF_LEN + 1] = {0};

    cmnGetSysTime(szTime, M_TIME_BUF_LEN);
    printf("[%s] %s", szTime, pszStr);

    pThis = NULL;
    return;
}

/*
 * Print string to buffer.
 */
static LS_VOID _toBuf(CLASS_LOG_T *pThis, LS_S8 *pszStr)
{
    /* [TBD] */

    pThis = NULL;
    pszStr = NULL;

    return;
}

/*
 * Print string to file.
 */
static LS_VOID _toFile(CLASS_LOG_T *pThis, LS_S8 *pszStr)
{
    /* [TBD] */

    pThis = NULL;
    pszStr = NULL;

    return;
}

/*
 * Print string to remote server.
 */
static LS_VOID _toServer(CLASS_LOG_T *pThis, LS_S8 *pszStr)
{
    /* [TBD] */

    pThis = NULL;
    pszStr = NULL;

    return;
}

/*
 * Output string in format.
 */
static LS_VOID output(CLASS_LOG_T *pThis, LS_LOG_TYPE_EN eType, LS_S8 *pszFmt, ...)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_U8 ucFlag = 0;
    va_list arg;

    if (eType >= E_LOG_TYPE_CNT || NULL == pszFmt)
    {
        return;
    }

    if (LS_TRUE == pPriv->bMasked)
    {
        return;
    }

    M_MUTEX_LOCK;


    va_start(arg, pszFmt);
    vsnprintf(pPriv->szLogBuf, M_LOG_BUF_SIZE, pszFmt, arg);
    va_end(arg);

    ucFlag = pPriv->aucFlag[eType];

    if ((ucFlag & (1 << E_LOG_TARGET_TTY)) != 0)
    {
        _toTTY(pThis, pPriv->szLogBuf);
    }

    if ((ucFlag & (1 << E_LOG_TARGET_BUFFER)) != 0)
    {
        _toBuf(pThis, pPriv->szLogBuf);
    }

    if ((ucFlag & (1 << E_LOG_TARGET_FILE)) != 0)
    {
        _toFile(pThis, pPriv->szLogBuf);
    }

    if ((ucFlag & (1 << E_LOG_TARGET_SERVER)) != 0)
    {
        _toServer(pThis, pPriv->szLogBuf);
    }

    M_MUTEX_UNLOCK;

    return;
}

/*
 * Set output mask, mask all output if [bMasked] is true.
 */
static LS_VOID setMasked(CLASS_LOG_T* pThis, LS_BOOL bMasked)
{
    PRIVATE_T *pPriv = pThis->pPriv;

    pPriv->bMasked = bMasked;

    return;
}

/*
 * Set output flag, enable information of [eType] to be send to [eTarget].
 */
static LS_VOID setFlag(CLASS_LOG_T* pThis, LS_LOG_TYPE_EN eType, LS_LOG_TARGET_EN eTarget)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_S32 i;

    if (eType >= E_LOG_TYPE_CNT || eTarget >= E_LOG_TARGET_CNT)
    {
        return;
    }

    M_MUTEX_LOCK;

    if (E_LOG_TYPE_ALL == eType && E_LOG_TARGET_ALL == eTarget)
    {
        /* Enable all type of information to all target. */
        memset(pPriv->aucFlag, 0xFF, sizeof(pPriv->aucFlag));
    }
    else if (E_LOG_TYPE_ALL == eType)
    {
        /* Enable all type of information to specified target. */
        for (i=0; i<E_LOG_TYPE_ALL; i++)
        {
            pPriv->aucFlag[i] |= 1 << eTarget;
        }
    }
    else if (E_LOG_TARGET_ALL == eTarget)
    {
        /* Enable specified type of information to all target. */
        pPriv->aucFlag[eType] = 0xFF;
    }
    else
    {
        /* Enable specified type of information to specified target. */
        pPriv->aucFlag[eType] |= (1 << eTarget);
    }

    M_MUTEX_UNLOCK;

    return;
}

/*
 * Clear output flag, disable information of [eType] to be send to [eTarget].
 */
static LS_VOID clrFlag(CLASS_LOG_T* pThis, LS_LOG_TYPE_EN eType, LS_LOG_TARGET_EN eTarget)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_S32 i;

    if (eType >= E_LOG_TYPE_CNT || eTarget >= E_LOG_TARGET_CNT)
    {
        return;
    }

    M_MUTEX_LOCK;

    if (E_LOG_TYPE_ALL == eType && E_LOG_TARGET_ALL == eTarget)
    {
        /* Disable all type of information to all target. */
        memset(pPriv->aucFlag, 0x00, sizeof(pPriv->aucFlag));
    }
    else if (E_LOG_TYPE_ALL == eType)
    {
        /* Disable all type of information to specified target. */
        for (i=0; i<E_LOG_TYPE_ALL; i++)
        {
            pPriv->aucFlag[i] &= ~(1 << eTarget);
        }
    }
    else if (E_LOG_TARGET_ALL == eTarget)
    {
        /* Disable specified type of information to all target. */
        pPriv->aucFlag[eType] = 0x00;
    }
    else
    {
        /* Disable specified type of information to specified target. */
        pPriv->aucFlag[eType] &= ~(1 << eTarget);
    }

    M_MUTEX_UNLOCK;

    return;
}

static LS_VOID del(CLASS_LOG_T *pThis)
{
    LS_FREE(pThis->pPriv);
    LS_FREE(pThis);

    return;
}

CLASS_LOG_T *new_CLASS_LOG_T()
{
    CLASS_LOG_T *pThis = NULL;

    if (NULL == (pThis = calloc(1, sizeof(CLASS_LOG_T))))
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis->pPriv = calloc(1, sizeof(PRIVATE_T))))
    {
        goto ERR_LABEL;
    }

    pThis->pPriv->bMasked = M_MASKED_DEF;
    pThis->pPriv->aucFlag[E_LOG_TYPE_ERROR] = M_FLAG_ERROR_DEF;
    pThis->pPriv->aucFlag[E_LOG_TYPE_WARN] = M_FLAG_WARN_DEF;
    pThis->pPriv->aucFlag[E_LOG_TYPE_INFO] = M_FLAG_INFO_DEF;
    pThis->pPriv->aucFlag[E_LOG_TYPE_DEBUG] = M_FLAG_DEBUG_DEF;
    pThis->pPriv->aucFlag[E_LOG_TYPE_TRACE] = M_FLAG_TRACE_DEF;
#ifndef _WIN32
    pthread_mutex_init(&(pThis->pPriv->lock), NULL);
#endif

    pThis->output = output;
    pThis->setMasked = setMasked;
    pThis->setFlag = setFlag;
    pThis->clrFlag = clrFlag;
    pThis->del = del;

    return pThis;

ERR_LABEL:
    del(pThis);

    return NULL;
}

#ifdef __cplusplus
}
#endif
