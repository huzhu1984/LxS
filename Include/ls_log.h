#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls_include.h"

#define M_LOG_BUF_SIZE              (256)

/* Default settings. */
#define M_MASKED_DEF                (LS_FALSE)
#define M_FLAG_ERROR_DEF            (E_LOG_TARGET_TTY | E_LOG_TARGET_BUFFER | E_LOG_TARGET_FILE | E_LOG_TARGET_SERVER)
#define M_FLAG_WARN_DEF             (E_LOG_TARGET_TTY | E_LOG_TARGET_BUFFER | E_LOG_TARGET_SERVER)
#define M_FLAG_INFO_DEF             (E_LOG_TARGET_TTY | E_LOG_TARGET_BUFFER)
#define M_FLAG_DEBUG_DEF            (0)
#define M_FLAG_TRACE_DEF            (0)

/* Typed of information. */
typedef enum
{
    E_LOG_TYPE_ERROR = 0,
    E_LOG_TYPE_WARN,
    E_LOG_TYPE_INFO,
    E_LOG_TYPE_DEBUG,
    E_LOG_TYPE_TRACE,
    E_LOG_TYPE_ALL,
    E_LOG_TYPE_CNT
} LS_LOG_TYPE_EN;

/* Target of information. */
typedef enum
{
    E_LOG_TARGET_TTY = 0,
    E_LOG_TARGET_BUFFER,
    E_LOG_TARGET_FILE,
    E_LOG_TARGET_SERVER,
    E_LOG_TARGET_ALL,
    E_LOG_TARGET_CNT
} LS_LOG_TARGET_EN;

typedef struct CLASS_LOG_T
{
    LS_VOID (*output)(struct CLASS_LOG_T *pThis, LS_LOG_TYPE_EN eType, LS_S8 *format, ...);
    LS_VOID (*setMasked)(struct CLASS_LOG_T *pThis, LS_BOOL bMasked);
    LS_VOID (*setFlag)(struct CLASS_LOG_T *pThis, LS_LOG_TYPE_EN eType, LS_LOG_TARGET_EN eTarget);
    LS_VOID (*clrFlag)(struct CLASS_LOG_T *pThis, LS_LOG_TYPE_EN eType, LS_LOG_TARGET_EN eTarget);
    LS_VOID (*del)(struct CLASS_LOG_T *pThis);

    PRIVATE_T *pPriv;
} CLASS_LOG_T;

CLASS_LOG_T *new_CLASS_LOG_T();

#ifdef __cplusplus
}
#endif

#endif // LOG_H
