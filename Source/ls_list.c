#include "ls_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LIST_NODE_T
{
    struct LIST_NODE_T *pstPrev;    /* Pointer to previous node */
    struct LIST_NODE_T *pstNext;    /* Pointer to next node */
    LS_VOID *pData;                 /* Attachive data */
} LIST_NODE_T;

struct PRIVATE_T
{
#ifndef _WIN32
    pthread_mutex_t lock;           /* Mutex lock */
#endif
    LS_S32 iCount;                  /* How much node holed in list */
    LIST_NODE_T *pstStart;          /* Start node of list */
    LIST_NODE_T *pstEnd;            /* End node of list*/
    LIST_NODE_T *pstCurrent;        /* Current iterator in list*/
};

static LS_S32 append(CLASS_LIST_T *pThis, LS_VOID *pData)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LIST_NODE_T *pstNewNode = NULL;

    M_MUTEX_LOCK;

    if (NULL == (pstNewNode = calloc(1, sizeof(LIST_NODE_T))))
    {
        M_MUTEX_UNLOCK;
        return LS_ERR;
    }

    pstNewNode->pData = pData;
    pstNewNode->pstNext = NULL;

    if (NULL == pPriv->pstStart)
    {
        /* List is empty, new Node is to be start&end node. */
        pstNewNode->pstPrev = NULL;
        pPriv->pstStart = pstNewNode;
        pPriv->pstEnd = pstNewNode;
        pPriv->pstCurrent = pstNewNode;
    }
    else
    {
        /* Add new Node to tail, and to be end node. */
        pstNewNode->pstPrev = pPriv->pstEnd;
        pPriv->pstEnd->pstNext = pstNewNode;
        pPriv->pstEnd = pstNewNode;
    }

    pPriv->iCount++;

    M_MUTEX_UNLOCK;

    return LS_OK;
}

static LS_VOID startIter(CLASS_LIST_T *pThis)
{
    PRIVATE_T *pPriv = pThis->pPriv;

    M_MUTEX_LOCK;
    pPriv->pstCurrent = pPriv->pstStart;
    M_MUTEX_UNLOCK;

    return;
}

static LS_VOID endIter(CLASS_LIST_T *pThis)
{
    PRIVATE_T *pPriv = pThis->pPriv;

    M_MUTEX_LOCK;
    pPriv->pstCurrent = pPriv->pstEnd;
    M_MUTEX_UNLOCK;

    return;
}

static LS_VOID nextIter(CLASS_LIST_T *pThis)
{
    PRIVATE_T *pPriv = pThis->pPriv;

    M_MUTEX_LOCK;

    if (NULL != pPriv->pstCurrent)
    {
        pPriv->pstCurrent = pPriv->pstCurrent->pstNext;
    }

    M_MUTEX_UNLOCK;

    return;
}

static LS_VOID *getIter(CLASS_LIST_T *pThis)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_VOID *pData = NULL;

    M_MUTEX_LOCK;

    if (NULL != pPriv->pstCurrent)
    {
        pData = pPriv->pstCurrent->pData;
    }

    M_MUTEX_UNLOCK;

    return pData;
}

static LS_VOID *getIterNext(CLASS_LIST_T *pThis)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_VOID *pData = NULL;

    M_MUTEX_LOCK;

    if (NULL != pPriv->pstCurrent)
    {
        pData = pPriv->pstCurrent->pData;
        pPriv->pstCurrent = pPriv->pstCurrent->pstNext;
    }

    M_MUTEX_UNLOCK;

    return pData;
}

static LS_S32 insertIter(CLASS_LIST_T *pThis, LS_VOID *pData)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LIST_NODE_T *pstNewNode = NULL;

    M_MUTEX_LOCK;

    if (NULL == (pstNewNode = calloc(1, sizeof(LIST_NODE_T))))
    {
        M_MUTEX_UNLOCK;
        return LS_ERR;
    }

    pstNewNode->pData = pData;
    pstNewNode->pstNext = NULL;

    if (NULL == pPriv->pstStart)
    {
        /* List is empty, new node is to be start&end node. */
        pstNewNode->pstPrev = NULL;
        pPriv->pstStart = pstNewNode;
        pPriv->pstEnd = pstNewNode;
        pPriv->pstCurrent = pstNewNode;
    }
    else if (NULL != pPriv->pstCurrent)
    {
        /* Insert the new node after current interator. */
        pstNewNode->pstPrev = pPriv->pstCurrent;
        pstNewNode->pstNext = pPriv->pstCurrent->pstNext;
        pPriv->pstCurrent->pstNext = pstNewNode;

        if (NULL == pstNewNode->pstNext)
        {
            /* New node is end of the list. */
            pPriv->pstEnd = pstNewNode;
        }
        else
        {
            /* New node is new prevous one of the next node. */
            pstNewNode->pstNext->pstPrev = pstNewNode;
        }
    }
    else
    {
        /* Iterator points to next of end node, cannot be inserted. */
        LS_FREE(pstNewNode);
        M_MUTEX_UNLOCK;
        return LS_ERR;
    }

    M_MUTEX_UNLOCK;

    return LS_OK;
}

static LS_S32 removeIter(CLASS_LIST_T *pThis)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LIST_NODE_T *pThisCurrent = NULL;

    M_MUTEX_LOCK;

    if (NULL == pPriv->pstCurrent)
    {
        /* List is empty, nothing to be remove. */
        M_MUTEX_UNLOCK;
        return LS_ERR;
    }

    if (pPriv->pstCurrent == pPriv->pstStart)
    {
        /* Remove the start node. */
        pPriv->pstStart = pPriv->pstCurrent->pstNext;
    }
    else
    {
        pPriv->pstCurrent->pstPrev->pstNext = pPriv->pstCurrent->pstNext;
    }

    if (pPriv->pstCurrent == pPriv->pstEnd)
    {
        /* Remove the end node. */
        pPriv->pstEnd = pPriv->pstCurrent->pstPrev;
    }
    else
    {
        pPriv->pstCurrent->pstNext->pstPrev = pPriv->pstCurrent->pstPrev;
    }

    /* If remove the current iterator node, move iterator to its next. */
    pThisCurrent = pPriv->pstCurrent->pstNext;
    LS_FREE(pPriv->pstCurrent);
    pPriv->pstCurrent = pThisCurrent;

    M_MUTEX_UNLOCK;

    return LS_ERR;
}

static LS_VOID forEach(struct CLASS_LIST_T *pThis, LIST_NODE_PROC_FUNC_F pfProc)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LIST_NODE_T *pstNode = NULL;

    if (NULL == pfProc)
    {
        return;
    }

    M_MUTEX_LOCK;

    pstNode = pThis->pPriv->pstStart;
    while (NULL != pstNode)
    {
        pfProc(pstNode->pData);
        pstNode = pstNode->pstNext;
    }

    M_MUTEX_UNLOCK;

    return;
}

static LS_VOID del(CLASS_LIST_T *pThis)
{
    LIST_NODE_T *pstTail = NULL;
    LIST_NODE_T *pstPreTail = NULL;

    pstTail = pThis->pPriv->pstStart;

    while (NULL != pstTail)
    {
        pstPreTail = pstTail;
        pstTail = pstTail->pstNext;

        LS_FREE(pstPreTail);
    }

    LS_FREE(pThis->pPriv);
    LS_FREE(pThis);

    return;
}

CLASS_LIST_T *new_CLASS_LIST_T()
{
    CLASS_LIST_T *pThis = NULL;

    if (NULL == (pThis = calloc(1, sizeof(CLASS_LIST_T))))
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis->pPriv = calloc(1, sizeof(PRIVATE_T))))
    {
        goto ERR_LABEL;
    }

    pThis->append = append;
    pThis->startIter = startIter;
    pThis->endIter = endIter;
    pThis->nextIter = nextIter;
    pThis->getIter = getIter;
    pThis->getIterNext = getIterNext;
    pThis->insertIter = insertIter;
    pThis->removeIter = removeIter;
    pThis->forEach = forEach;
    pThis->del = del;

#ifndef _WIN32
    pthread_mutex_init(&(pThis->pPriv->lock), NULL);
#endif

    pThis->pPriv = pThis->pPriv;

    return pThis;

ERR_LABEL:
    del(pThis);

    return NULL;
}

#ifdef __cplusplus
}
#endif
