#include "ls_gen.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PRIVATE_T
{
    LS_S32 iData;
};

static LS_S32 method(CLASS_GEN_T *pThis, LS_S32 iArg)
{
    PRIVATE_T *pPriv = pThis->pPriv;

    pPriv->iData = iArg;

    return LS_OK;
}

static LS_VOID del(CLASS_GEN_T *pThis)
{
    LS_FREE(pThis->pPriv);
    LS_FREE(pThis);

    return;
}

CLASS_GEN_T *new_CLASS_GEN_T(LS_S32 iArg)
{
    CLASS_GEN_T *pThis = NULL;

    if (NULL == (pThis = calloc(1, sizeof(CLASS_GEN_T))))
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis->pPriv = calloc(1, sizeof(PRIVATE_T))))
    {
        goto ERR_LABEL;
    }

    /* Initialize private data here. */
    pThis->pPriv->iData = iArg;

    /* Mount methods here. */
    pThis->method = method;
    pThis->del = del;

    return pThis;

ERR_LABEL:
    del(pThis);

    return NULL;
}

#ifdef __cplusplus
}
#endif
