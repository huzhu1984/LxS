#include "ls_cli.h"
#include "ls_list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define M_CLI_HTY_CMD_CNT           (10)

struct PRIVATE_T
{
    CLI_PRINT_F pfPrint;
    CLASS_LIST_T *apCmdList[E_CLI_GRP_CNT];
    LS_S8 aszCmdBuf[M_CLI_HTY_CMD_CNT][M_CLI_CMD_BUF_SIZE + 1];
    LS_S8 szHtyCmd[M_CLI_CMD_BUF_SIZE + 1];
    LS_S32 iCmdIdx;
    LS_S32 iCurrCmdIdx;
    LS_S32 iCmdLen;
    LS_S8 szCtrlBuf[4];
    LS_S8 szTknBuf[M_CLI_CMD_BUF_SIZE + 1];
    LS_S8 *apszArgv[M_CLI_TKN_MAX + 1];
    LS_S32 iArgc;
    CLI_GRP_EN eCurrGrp;
};

typedef enum
{
    E_TOKEN_TYPE_S = 0,
    E_TOKEN_TYPE_M,
    E_TOKEN_TYPE_C,
    E_TOKEN_TYPE_CNT
} TOKEN_TYPE_EN;

typedef struct
{
    LS_S8 *pszName;
    LS_S8 *pszTip;
    TOKEN_TYPE_EN eType;
    CLASS_LIST_T *pChild;
    CLI_CALLBACK_F pfCallBack;
} CLI_TOKEN_T;

#define CLI_PRINT(format, args...)  pPriv->pfPrint(format, ##args)

/*
 * Generate a command from arguments.
 */
static LS_VOID _genNewCmd(CLASS_CLI_T *pThis, LS_S8 *pszCmd)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_S32 i = 0;

    *pszCmd = '\0';

    while (NULL != pPriv->apszArgv[i])
    {
        strcat(pszCmd, pPriv->apszArgv[i]);
        strcat(pszCmd, " ");
        i++;
    }

    return;
}

/*
 * Walk a command list and return the halfmatch token counts.
 */
static LS_S32 _getHalfMatch(CLI_TOKEN_T *pstToken, LS_S8 *pszIn, LS_S8 *pszOut)
{
    LS_S8 szOut[M_CLI_TKN_LEN + 1] = {0};
    LS_S8 *pszStart = NULL;
    LS_S8 *pszEnd = NULL;
    LS_S32 iStrLen = 0;
    LS_S32 iRet = 0;

    iStrLen = strlen(pszIn);

    switch (pstToken->eType)
    {
    case E_TOKEN_TYPE_S:
        if (0 == strncmp(pstToken->pszName, pszIn, iStrLen))
        {
            strncpy(pszOut, pstToken->pszName, M_CLI_TKN_LEN);
            iRet++;
        }
        break;
    case E_TOKEN_TYPE_M:
        szOut[0] = '(';
        pszStart = pstToken->pszName;

        do
        {
            pszStart++;
            if (0 == strncmp(pszIn, pszStart, iStrLen))
            {
                if (NULL != (pszEnd = strchr(pszStart, '|')) ||
                    NULL != (pszEnd = strchr(pszStart, ')')))
                {
                    strncat(szOut, pszStart, pszEnd-pszStart);
                    strcat(szOut, "|");
                }

                iRet++;
            }
        } while (NULL != (pszStart = strchr(pszStart, '|')));

        iStrLen = strlen(szOut);
        if (iStrLen > 1)
        {
            szOut[iStrLen - 1] = ')';
            strncpy(pszOut, szOut, M_CLI_TKN_LEN);
        }
        break;
    case E_TOKEN_TYPE_C:
        if (0 == strcmp(pstToken->pszName, M_CLI_CUSTOM_TOKEN))
        {
            strcpy(pszOut, M_CLI_CUSTOM_TOKEN);
            iRet = -1;
        }
        break;
    default:
        break;
    }

    return iRet;
}

/*
 * Walk a multi-token(may be), search the token with name.
 */
static LS_S8 *_walkTknNode(CLI_TOKEN_T *pstToken, LS_S8 *pszName)
{
    LS_S8 *pszRet = NULL;
    LS_S8 *pszStart = NULL;
    LS_S8 *pszEnd = NULL;
    LS_S32 iStrLen = 0;

    iStrLen = strlen(pszName);

    if (0 == strcmp(pstToken->pszName, pszName))
    {
        pszRet = pstToken->pszName;
    }
    else if ('(' == pstToken->pszName[0])
    {
        pszStart = pstToken->pszName;

        do
        {
            pszStart++;

            if (NULL != (pszEnd = strchr(pszStart, '|')) ||
                NULL != (pszEnd = strchr(pszStart, ')')))
            {
                iStrLen = pszEnd - pszStart;
            }

            if (0 == strncmp(pszStart, pszName, iStrLen))
            {
                pszRet = pszStart;
                break;
            }
        } while (NULL != (pszStart = strchr(pszStart, '|')));
    }

    return pszRet;
}

/*
 * Walk the command list, search the token with name.
 */
static CLI_TOKEN_T *_walkCmdList(CLASS_LIST_T *pList, LS_S8 *pszName)
{
    CLI_TOKEN_T *pstToken = NULL;
    CLI_TOKEN_T *pstTokenRet = NULL;

    LS_CALL(pList, startIter);
    while (NULL != (pstToken = LS_CALL(pList, getIterNext)))
    {
        if (NULL == pstToken)
        {
            break;
        }

        if (NULL != strstr(pstToken->pszName, M_CLI_CUSTOM_TOKEN))
        {
            pstTokenRet = pstToken;
        }

        if (NULL != _walkTknNode(pstToken, pszName))
        {
            pstTokenRet = pstToken;
            break;
        }
    }

    return pstTokenRet;
}

/*
 * Walk the token list, search the token with name.
 */
static CLI_TOKEN_T *_walkTknList(CLASS_LIST_T *pList, LS_S8 *pszName)
{
    CLI_TOKEN_T *pstToken = NULL;

    LS_CALL(pList, startIter);
    while (NULL != (pstToken = LS_CALL(pList, getIterNext)))
    {
        if (NULL == pstToken)
        {
            return NULL;
        }

        if (0 == strcmp(pstToken->pszName, pszName))
        {
            return pstToken;
        }
    }

    return NULL;
}

/*
 * Delete all the tokens in a tree, recursively.
 */
static LS_VOID _delToken(CLASS_LIST_T *pList)
{
    CLI_TOKEN_T *pstToken = NULL;

    if (NULL == pList)
    {
        return;
    }

    LS_CALL(pList, startIter);
    while (NULL != (pstToken = LS_CALL(pList, getIterNext)))
    {
        if (NULL != pstToken->pChild)
        {
            /* Recursive call. */
            _delToken(pstToken->pChild);
            LS_DELETE(pstToken->pChild);
        }

        LS_FREE(pstToken);
    }

    return;
}

/*
 * Parse a command string and spilit it to tokens.
 */
static LS_VOID _parseCmd(CLASS_CLI_T *pThis)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_S32 iBufLen = 0;
    LS_S32 i = 0;
    LS_S8 bDelimiter = LS_TRUE;

    bzero(pPriv->szTknBuf, sizeof(pPriv->szTknBuf));
    bzero(pPriv->apszArgv, sizeof(pPriv->apszArgv));
    pPriv->iArgc = 0;

    iBufLen = strlen(pPriv->aszCmdBuf[pPriv->iCmdIdx]);
    if (pPriv->iCmdIdx != pPriv->iCurrCmdIdx)
    {
        strcpy(pPriv->szTknBuf, pPriv->szHtyCmd);
    }
    else
    {
        strcpy(pPriv->szTknBuf, pPriv->aszCmdBuf[pPriv->iCurrCmdIdx]);
    }

    /* Split the tokens to the argv array. */
    for (i=0; i<pPriv->iCmdLen; i++)
    {
        if (' ' == pPriv->szTknBuf[i])
        {
            pPriv->szTknBuf[i] = '\0';
            bDelimiter = LS_TRUE;
        }
        else
        {
            if (LS_TRUE == bDelimiter)
            {
                pPriv->apszArgv[pPriv->iArgc] = &(pPriv->szTknBuf[i]);
                pPriv->iArgc++;
                bDelimiter = LS_FALSE;

                if (pPriv->iArgc > M_CLI_TKN_MAX)
                {
                    break;
                }
            }
        }
    }

    return;
}

/*
 * Auto complete a command.
 */
static LS_VOID _autoComplete(CLASS_CLI_T *pThis)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    CLASS_LIST_T *pList = NULL;
    CLASS_LIST_T *pstOldList = NULL;
    CLI_TOKEN_T *pstToken = NULL;
    CLI_TOKEN_T *pstOldToken = NULL;
    CLI_TOKEN_T *pstFixToken = NULL;
    CLI_TOKEN_T *pstCtmToken = NULL;
    LS_S8 *pszCmd = NULL;
    LS_S8 szToken[M_CLI_TKN_LEN + 1] = {0};
    LS_S8 szCmpToken[M_CLI_TKN_LEN + 1] = {0};
    LS_S8 *pszLastToken = NULL;
    LS_S8 cLastChar = 0;
    LS_S32 iFixTokenCnt = 0;
    LS_S32 iCtmTokenCnt = 0;
    LS_S32 iStrlen = 0;
    LS_S32 i = 0;
    LS_S32 j = 0;

    pList = pPriv->apCmdList[pPriv->eCurrGrp];

    /* No charactor inputed, print the first level token. */
    if (0 == pPriv->iCmdLen)
    {
        LS_CALL(pList, startIter);
        while (NULL != (pstToken = LS_CALL(pList, getIterNext)))
        {
            CLI_PRINT("\r\n%-48s-%s", pstToken->pszName, pstToken->pszTip);
        }

        CLI_PRINT("\r\n->");

        return;
    }

    _parseCmd(pThis);

    /* Select current command or history commond to be completed. */
    if (pPriv->iCmdIdx != pPriv->iCurrCmdIdx)
    {
        pszCmd = pPriv->szHtyCmd;
    }
    else
    {
        pszCmd = pPriv->aszCmdBuf[pPriv->iCmdIdx];
    }

    pszLastToken = pPriv->apszArgv[pPriv->iArgc - 1];
    cLastChar = pszCmd[pPriv->iCmdLen - 1];

    /* Search for the token and excute the callback function. */
    while (NULL != pPriv->apszArgv[i])
    {
        if (NULL == (pstToken = _walkCmdList(pList, pPriv->apszArgv[i])))
        {
            break;
        }

        pstOldList = pList;
        pstOldToken = pstToken;

        pList = pstToken->pChild;
        i++;
    }

    /* Not match at all. */
    if (i < pPriv->iArgc - 1)
    {
        return;
    }

    /* Space is a terminate symbol. */
    if (' ' == cLastChar)
    {
        if (i == pPriv->iArgc)
        {
            LS_CALL(pList, startIter);
            while (NULL != (pstToken = LS_CALL(pList, getIterNext)))
            {
                CLI_PRINT("\r\n%-48s-%s", pstToken->pszName, pstToken->pszTip);
                j++;
            }

            if (0 == j)
            {
                /* All command's token is match. */
                CLI_PRINT("\r\n<End>");
            }

            CLI_PRINT("\r\n->%s", pszCmd);
        }

        return;
    }

    /* If last token is matched as a <*> type, check its sibling. */
    if (i == pPriv->iArgc && NULL != pstOldToken &&
        (E_TOKEN_TYPE_C == pstOldToken->eType || ' ' != cLastChar))
    {
        pList = pstOldList;
    }

    /* Calculate how much match token exist. */
    LS_CALL(pList, startIter);
    while (NULL != (pstToken = LS_CALL(pList, getIterNext)))
    {
        i = _getHalfMatch(pstToken, pszLastToken, szToken);
        switch (i)
        {
        case -1:
            pstCtmToken = pstToken;
            iCtmTokenCnt++;
            break;
        case 0:
            break;
        default:
            pstFixToken = pstToken;
            iFixTokenCnt += i;
            break;
        }
    }

    /* Only one fix token is match, complete it. */
    if (1 == iFixTokenCnt)
    {
        iStrlen = strlen(pszLastToken);
        _getHalfMatch(pstFixToken, pszLastToken, szToken);

        if (E_TOKEN_TYPE_M == pstFixToken->eType)
        {
            strcpy(szCmpToken, &(szToken[iStrlen + 1]));
            szCmpToken[strlen(szCmpToken) - 1] = '\0';
        }
        else
        {
            strcpy(szCmpToken, &(szToken[iStrlen]));
        }

        strcat(szCmpToken, " ");
        CLI_PRINT("%s", szCmpToken);
        strcat(pszCmd, szCmpToken);
        pPriv->iCmdLen += strlen(szCmpToken);

        iFixTokenCnt = 0;
    }

    /* Multi-match token exist, list then. */
    if (iFixTokenCnt + iCtmTokenCnt > 0)
    {
        LS_CALL(pList, startIter);
        while (NULL != (pstToken = LS_CALL(pList, getIterNext)))
        {
            if (0 != _getHalfMatch(pstToken, pszLastToken, szToken))
            {
                CLI_PRINT("\r\n%-48s-%s", pstToken->pszName, pstToken->pszTip);
            }
        }

        CLI_PRINT("\r\n->%s", pszCmd);
    }

    return;
}

/*
 * A valid command has been inputed, call its callback function.
 */
static LS_VOID _excuteCmd(CLASS_CLI_T *pThis)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    CLASS_LIST_T *pList = NULL;
    CLI_TOKEN_T *pstToken = NULL;
    LS_S32 i = 0;

    _parseCmd(pThis);

    pList = pPriv->apCmdList[pPriv->eCurrGrp];

    /* Search for the token and excute the callback function. */
    while (NULL != pPriv->apszArgv[i])
    {
        if (NULL == (pstToken = _walkCmdList(pList, pPriv->apszArgv[i])))
        {
            break;
        }

        pList = pstToken->pChild;
        i++;
    }

    if (NULL == pPriv->apszArgv[i] && NULL != pstToken->pfCallBack)
    {
        pstToken->pfCallBack(pPriv->apszArgv);
    }
    else
    {
        CLI_PRINT("Unknown command.\r\n");
    }

    return;
}

/*
 * Process the character inputed.
 */
static LS_VOID _procInputChar(CLASS_CLI_T *pThis, LS_S8 cIn)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_S32 iPrevIdx = 0;

    switch (cIn)
    {
    case '\t':
        _autoComplete(pThis);
        break;
    case '\r':
    case '\n':
        /* Process Input stream. */
        CLI_PRINT("\r\n");

        if (pPriv->iCmdLen > 0)
        {
            _excuteCmd(pThis);
            _genNewCmd(pThis, pPriv->szHtyCmd);

            /* Copy new command into history command array. */
            iPrevIdx = (pPriv->iCurrCmdIdx - 1 < 0) ? (M_CLI_HTY_CMD_CNT - 1) : (pPriv->iCurrCmdIdx - 1);
            if (LS_OK == cmnStrCmpNoCaseSpace(pPriv->aszCmdBuf[iPrevIdx], pPriv->szHtyCmd))
            {
                /* Use current commnad buffer. */
                strcpy(pPriv->aszCmdBuf[iPrevIdx], pPriv->szHtyCmd);
                pPriv->iCmdIdx = pPriv->iCurrCmdIdx;
            }
            else
            {
                /* Switch to next commnad buffer. */
                strcpy(pPriv->aszCmdBuf[pPriv->iCurrCmdIdx], pPriv->szHtyCmd);
                pPriv->iCmdIdx = (pPriv->iCurrCmdIdx + 1) % M_CLI_HTY_CMD_CNT;
                pPriv->iCurrCmdIdx = pPriv->iCmdIdx;
            }

            /* Initialize next command buffer. */
            bzero(pPriv->aszCmdBuf[pPriv->iCmdIdx], M_CLI_CMD_BUF_SIZE + 1);
            bzero(pPriv->szHtyCmd, M_CLI_CMD_BUF_SIZE + 1);
            pPriv->iCmdLen = 0;
            fflush(stdin);
        }
        CLI_PRINT("->");
        break;
    case '\b':
        /* Backspace input. */
        if (pPriv->iCmdLen > 0)
        {
            pPriv->iCmdLen--;

            /* Select current command or history command to be backspaced. */
            if (pPriv->iCmdIdx != pPriv->iCurrCmdIdx)
            {
                pPriv->szHtyCmd[pPriv->iCmdLen] = '\0';
            }
            else
            {
                pPriv->aszCmdBuf[pPriv->iCmdIdx][pPriv->iCmdLen] = '\0';
            }
            CLI_PRINT("\b \b");
        }
        break;
    default:
        /* Command buffer is full. */
        if (pPriv->iCmdLen > M_CLI_CMD_BUF_SIZE)
        {
            break;
        }

        /* Put Legal character into command buffer. */
        if (cIn < ' ' || cIn > '~')
        {
            break;
        }

        CLI_PRINT("%c", cIn);

        /* Select current command or histroy command to put the charactor in. */
        if (pPriv->iCmdIdx != pPriv->iCurrCmdIdx)
        {
            pPriv->szHtyCmd[pPriv->iCmdLen] = cIn;
        }
        else
        {
            pPriv->aszCmdBuf[pPriv->iCmdIdx][pPriv->iCmdLen] = cIn;
        }

        pPriv->iCmdLen++;
        break;
    }

    return;
}

static LS_S32 regCmd(CLASS_CLI_T *pThis, CLI_GRP_EN eGrp,
                     CLI_TOKEN_REG_T *pstTokenReg)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    CLASS_LIST_T *pList = NULL;
    CLI_TOKEN_T *pstToken = NULL;

    if (eGrp >= E_CLI_GRP_CNT || NULL == pstTokenReg)
    {
        return LS_ERR;
    }

    /* Find tree level where the token with same level is unique. */
    pList = pPriv->apCmdList[eGrp];
    while (NULL != pstTokenReg->pszName)
    {
        /* Walk level of tree, if same token exists, walk next level for next
           next token. */
        if (NULL == (pstToken = _walkTknList(pList, pstTokenReg->pszName)))
        {
            break;
        }

        pList = pstToken->pChild;
        pstTokenReg++;
    }

    /* Build command tree. */
    while (NULL != pstTokenReg->pszName)
    {
        /* Add token to the last match tree level. */
        if (NULL == (pstToken = calloc(1, sizeof(CLI_TOKEN_T))))
        {
            return LS_ERR;
        }

        pList->append(pList, pstToken);

        /* Every token has a list for his sons. */
        if (NULL == (pList = LS_NEW(CLASS_LIST_T)))
        {
            return LS_ERR;
        }

        pstToken->pszName = pstTokenReg->pszName;
        pstToken->pszTip = pstTokenReg->pszTip;
        pstToken->pfCallBack = pstTokenReg->pfCallBack;
        pstToken->pChild = pList;
        if (0 == strcmp(pstToken->pszName, M_CLI_CUSTOM_TOKEN))
        {
            pstToken->eType = E_TOKEN_TYPE_C;
        }
        else if ('(' == pstToken->pszName[0])
        {
            pstToken->eType = E_TOKEN_TYPE_M;
        }
        else
        {
            pstToken->eType = E_TOKEN_TYPE_S;
        }

        pstTokenReg++;
    }

    return LS_OK;
}

static LS_VOID feed(CLASS_CLI_T *pThis, LS_S8 cIn)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_S8 *pszCmd = NULL;
    LS_S32 iCmdLen = 0;
    LS_S32 iPrevCmdIdx = 0;
    LS_S32 i;

    pszCmd = pPriv->aszCmdBuf[pPriv->iCmdIdx];
    iCmdLen = strlen(pszCmd);

    /* Handle the up, down, left, right arrows input. */
    if (0x1B == cIn)
    {
        pPriv->szCtrlBuf[0] = cIn;
        pPriv->szCtrlBuf[1] = '\0';
        return;
    }

    if (0x1B == pPriv->szCtrlBuf[0] && 0x5B == cIn)
    {
        pPriv->szCtrlBuf[1] = cIn;
        return;
    }

    if (0x1B == pPriv->szCtrlBuf[0] && 0x5B == pPriv->szCtrlBuf[1])
    {
        switch (cIn)
        {
        case 0x41:
            iPrevCmdIdx = (pPriv->iCmdIdx - 1 < 0) ? (M_CLI_HTY_CMD_CNT - 1) : (pPriv->iCmdIdx - 1);
            if (iPrevCmdIdx == pPriv->iCurrCmdIdx || 0 == strlen(pPriv->aszCmdBuf[iPrevCmdIdx]))
            {
                /* No recursive. */
                return;
            }
            for (i=0; i<iCmdLen; i++)
            {
                CLI_PRINT("\b");
            }
            for (i=0; i<iCmdLen; i++)
            {
                CLI_PRINT(" ");
            }
            for (i=0; i<iCmdLen; i++)
            {
                CLI_PRINT("\b");
            }
            pPriv->iCmdIdx = (pPriv->iCmdIdx - 1 < 0) ? (M_CLI_HTY_CMD_CNT - 1) : pPriv->iCmdIdx - 1;
            pPriv->iCmdLen = strlen(pPriv->aszCmdBuf[pPriv->iCmdIdx]);
            strcpy(pPriv->szHtyCmd, pPriv->aszCmdBuf[pPriv->iCmdIdx]);
            CLI_PRINT("%s", pPriv->aszCmdBuf[pPriv->iCmdIdx]);
            return;
        case 0x42:
            if (pPriv->iCmdIdx == pPriv->iCurrCmdIdx)
            {
                /* Down to the last command, stop. */
                return;
            }
            for (i=0; i<iCmdLen; i++)
            {
                CLI_PRINT("\b");
            }
            for (i=0; i<iCmdLen; i++)
            {
                CLI_PRINT(" ");
            }
            for (i=0; i<iCmdLen; i++)
            {
                CLI_PRINT("\b");
            }
            pPriv->iCmdIdx = (pPriv->iCmdIdx + 1) % 10;
            pPriv->iCmdLen = strlen(pPriv->aszCmdBuf[pPriv->iCmdIdx]);
            strcpy(pPriv->szHtyCmd, pPriv->aszCmdBuf[pPriv->iCmdIdx]);
            CLI_PRINT("%s", pPriv->aszCmdBuf[pPriv->iCmdIdx]);
            return;
        case 0x43:
            return;
        case 0x44:
            return;
        default:
            break;
        }
    }

    _procInputChar(pThis, cIn);

    bzero(pPriv->szCtrlBuf, sizeof(pPriv->szCtrlBuf));

    return;
}

static LS_S32 switchGrp(CLASS_CLI_T *pThis, CLI_GRP_EN eGrp)
{
    PRIVATE_T *pPriv = pThis->pPriv;

    if (eGrp >= E_CLI_GRP_CNT)
    {
        return LS_ERR;
    }

    pPriv->eCurrGrp = eGrp;

    return LS_OK;
}

static LS_VOID del(CLASS_CLI_T *pThis)
{
    LS_S32 i;

    for (i=0; i<E_CLI_GRP_CNT; i++)
    {
        _delToken(pThis->pPriv->apCmdList[i]);
        LS_DELETE(pThis->pPriv->apCmdList[i]);
    }

    LS_FREE(pThis->pPriv);
    LS_FREE(pThis);

    return;
}

CLASS_CLI_T *new_CLASS_CLI_T(CLI_PRINT_F pfPrint)
{
    CLASS_CLI_T *pThis = NULL;
    CLASS_LIST_T *pList = NULL;
    LS_S32 i;

    if (NULL == pfPrint)
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis = calloc(1, sizeof(CLASS_CLI_T))))
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis->pPriv = calloc(1, sizeof(PRIVATE_T))))
    {
        goto ERR_LABEL;
    }

    pThis->pPriv->pfPrint = pfPrint;
    pThis->pPriv->eCurrGrp = E_CLI_GRP_0;

    /* Create the command group trees. */
    for (i=0; i<E_CLI_GRP_CNT; i++)
    {
        if (NULL == (pList = LS_NEW(CLASS_LIST_T)))
        {
            goto ERR_LABEL;
        }

        pThis->pPriv->apCmdList[i] = pList;
    }

    pfPrint("\r\nWelcome! Press 'Tab' to get tip and auto-completion.");
    pfPrint("\n\r->");

    pThis->regCmd = regCmd;
    pThis->feed = feed;
    pThis->switchGrp = switchGrp;
    pThis->del = del;

    return pThis;

ERR_LABEL:
    del(pThis);

    return NULL;
}

#ifdef __cplusplus
}
#endif
