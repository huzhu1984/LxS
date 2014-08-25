#ifndef LS_CLI_H
#define LS_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls_include.h"

#define M_CLI_CMD_BUF_SIZE          (256)
#define M_CLI_TKN_MAX               (32)
#define M_CLI_TKN_LEN               (32)

#define M_CLI_CUSTOM_TOKEN          ("<*>")

typedef LS_S32 (*CLI_PRINT_F)(const LS_S8 *pszFormat, ...);
typedef LS_S32 (*CLI_CALLBACK_F)(LS_S8**);

/* Parameter needed to register a token to CLI. */
typedef struct
{
    LS_S8 *pszName;             /* Name of token */
    LS_S8 *pszTip;              /* Tip for token */
    CLI_CALLBACK_F pfCallBack;  /* Callback function */
} CLI_TOKEN_REG_T;

/* Command group ID. if you need more, redefine it. */
typedef enum
{
    E_CLI_GRP_0 = 0,
    E_CLI_GRP_1,
    E_CLI_GRP_2,
    E_CLI_GRP_3,
    E_CLI_GRP_CNT
} CLI_GRP_EN;

typedef struct CLASS_CLI_T
{
    /*
     * Register a command.
     * eGrp[in]: Command group to which a new command is registered.
     * pstTknReg[in]: New command tokens array.
     * Return: LS_OK or LS_ERR.
     */
    LS_S32 (*regCmd)(struct CLASS_CLI_T *pThis, CLI_GRP_EN eGrp,
                     CLI_TOKEN_REG_T *pstTknReg);

    /*
     * Feed a character to CLI, the character will be analized.
     * cIn[in]: Character inputed.
     */
    LS_VOID (*feed)(struct CLASS_CLI_T *pThis, LS_S8 cIn);

    /*
     * Switch current command group.
     * eGrp[in]: Command group ID.
     * Return: LS_OK or LS_ERR.
     */
    LS_S32 (*switchGrp)(struct CLASS_CLI_T *pThis, CLI_GRP_EN eGrp);

    LS_VOID (*del)(struct CLASS_CLI_T *pThis);

    PRIVATE_T *pPriv;
} CLASS_CLI_T;

/*
 * pfPrint
 */
CLASS_CLI_T *new_CLASS_CLI_T(CLI_PRINT_F pfPrint);

#ifdef __cplusplus
}
#endif

#endif // LS_CLI_H
