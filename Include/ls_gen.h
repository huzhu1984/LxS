#ifndef LS_GEN_H
#define LS_GEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls_include.h"
#include "ls_log.h"

typedef struct CLASS_GEN_T
{
    LS_S32 (*method)(struct CLASS_GEN_T *pThis, LS_S32 iArg);
    LS_VOID (*del)(struct CLASS_GEN_T *pThis);

    PRIVATE_T *pPriv;
} CLASS_GEN_T;

CLASS_GEN_T *new_CLASS_GEN_T(LS_S32 iArg);

#ifdef __cplusplus
}
#endif

#endif // LS_GEN_H
