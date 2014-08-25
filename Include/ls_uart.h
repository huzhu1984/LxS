#ifndef LS_UART_H
#define LS_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls_include.h"

/* Default timeout at waiting response. */
#define M_TIMEOUT_DEF               (2000)

typedef struct CLASS_UART_T
{
    LS_S32 (*setAttr)(struct CLASS_UART_T *pThis, LS_S32 iBaud, LS_S8 cParity, LS_S32 iDataBit, LS_S32 iStopBit);
    LS_VOID (*setTimeout)(struct CLASS_UART_T *pThis, LS_S32 iTimeoutMs);
    LS_VOID (*flush)(struct CLASS_UART_T *pThis);
    LS_S32 (*write)(struct CLASS_UART_T *pThis, LS_U8 *pucData, LS_S32 iLen);
    LS_S32 (*read)(struct CLASS_UART_T *pThis, LS_U8 *pucData, LS_S32 iLen);
    LS_VOID (*del)(struct CLASS_UART_T *pThis);

    PRIVATE_T *pPriv;
} CLASS_UART_T;

CLASS_UART_T *new_CLASS_UART_T(LS_S8 *pszDev);

#ifdef __cplusplus
}
#endif

#endif // LS_UART_H
