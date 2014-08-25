#ifndef LS_MODBUS_CLIENT_H
#define LS_MODBUS_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls_include.h"
#include "ls_uart.h"
#include "ls_log.h"

/* Error code. */
#define M_MODBUS_ERRC_TIMEOUT   (-2)    /* Time out at waiting response */
#define M_MODBUS_ERRC_SEND      (-3)    /* Send data failed */
#define M_MODBUS_ERRC_LENGTH    (-4)    /* Length of received data incorrect.*/
#define M_MODBUS_ERRC_CRC       (-5)    /* CRC error */
#define M_MODBUS_ERRC_ADDR      (-6)    /* Address of modbus incorrect */
#define M_MODBUS_ERRC_FUNCC     (-7)    /* Function code uncorrect */

typedef struct CLASS_MODBUS_CLIENT_T
{
    /*
     * Read registers.
     * usRegAddr[in]: Address of register.
     * usRegCnt[in]: How many registers to read.
     * pucOut[out]: Buffer for loading data read.
     * Return: LS_OK or Error code.
     */
    LS_S32 (*readReg)(struct CLASS_MODBUS_CLIENT_T *pThis, LS_U16 usRegAddr, LS_U16 usRegCnt, LS_U8 *pucOut);

    /*
     * Write registers.
     * usRegAddr[in]: Address of register.
     * usRegCnt[in]: How many registers to write.
     * pucIn[in]: Buffer of data to write.
     * Return: LS_OK or Error code.
     */
    LS_S32 (*writeReg)(struct CLASS_MODBUS_CLIENT_T *pThis, LS_U16 usRegAddr, LS_U16 usRegCnt, LS_U8 *pucIn);

    /*
     * Write a register.
     * usRegAddr[in]: Address of register.
     * pucIn[in]: Buffer of data to write.
     * Return: LS_OK or Error code.
     */
    LS_S32 (*writeRegSig)(struct CLASS_MODBUS_CLIENT_T *pThis, LS_U16 usRegAddr, LS_U8 *pucIn);

    /*
     * Set log object.
     * pLog[in]: Log object.
     */
    LS_VOID (*setLog)(struct CLASS_MODBUS_CLIENT_T *pThis, CLASS_LOG_T *pLog);

    /*
     * Get log object.
     * Return: Log object.
     */
    CLASS_LOG_T* (*getLog)(struct CLASS_MODBUS_CLIENT_T *pThis);

    LS_VOID (*del)(struct CLASS_MODBUS_CLIENT_T *pThis);

    PRIVATE_T *pPriv;
} CLASS_MODBUS_CLIENT_T;

/*
 * Callback function when modbus want to receive data.
 * pucData[Out]: Buffer for loading data received.
 * iLen[in]: How much data to received.
 * pData[in]: User data.
 * Return: LS_OK or LS_ERR.
 */
typedef LS_S32 (*MODBUS_RECV_FUNC)(LS_U8* pucData, LS_S32 iLen, LS_VOID *pData);

/*
 * Callback function when modbus want to receive data.
 * pucData[Out]: Buffer for loading data received.
 * iLen[in]: How much data to received.
 * Return: LS_OK or LS_ERR.
 */
typedef LS_S32 (*MODBUS_SEND_FUNC)(LS_U8* pucData, LS_S32 iLen, LS_VOID *pData);

/*
 * pfRecv[in]: Callback funtion for receiving data.
 * pfSend[in]: Callback function of sending data.
 * pData[in]: User data passed to <pfRecv> and <pfSend>.
 */
CLASS_MODBUS_CLIENT_T *new_CLASS_MODBUS_CLIENT_T(LS_U8 ucMbAddr,
                                                 MODBUS_RECV_FUNC pfRecv,
                                                 MODBUS_SEND_FUNC pfSend,
                                                 LS_VOID *pData);

#ifdef __cplusplus
}
#endif

#endif // LS_MODBUS_CLIENT_H
