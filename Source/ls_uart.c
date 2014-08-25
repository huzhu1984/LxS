#include "ls_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PRIVATE_T
{
#ifdef _WIN32
    HANDLE hCommFile;
#else
    LS_S32 iFd;
#endif

    LS_S32 iTimeoutMs;
};

static LS_S32 setAttr(CLASS_UART_T *pThis, LS_S32 iBaud, LS_S8 cParity, LS_S32 iDataBit, LS_S32 iStopBit)
{
    PRIVATE_T *pPriv = pThis->pPriv;

#ifdef _WIN32
    DCB CommAttr;

    GetCommState(hCommFile, &CommAttr);

    switch (iBaud)
    {
    case 600:
        CommAttr.BaudRate = CBR_600;
        break;
    case 1200:
        CommAttr.BaudRate = CBR_1200;
        break;
    case 2400:
        CommAttr.BaudRate = CBR_2400;
        break;
    case 4800:
        CommAttr.BaudRate = CBR_4800;
        break;
    case 9600:
        CommAttr.BaudRate = CBR_9600;
        break;
    case 19200:
        CommAttr.BaudRate = CBR_19200;
        break;
    case 38400:
        CommAttr.BaudRate = CBR_38400;
        break;
    case 57600:
        CommAttr.BaudRate = CBR_57600;
        break;
    case 115200:
        CommAttr.BaudRate = CBR_115200;
        break;
    default:
        CommAttr.BaudRate = CBR_9600;
        break;
    }

    CommAttr.ByteSize = iDataBit;

    switch (cParity)
    {
    case 0:
        CommAttr.Parity = NOPARITY;
        break;
    case 1:
        CommAttr.Parity = ODDPARITY;
        break;
    case 2:
        CommAttr.Parity = EVENPARITY;
        break;
    default:
        CommAttr.Parity = NOPARITY;
        break;
    }

    switch (iStopBit)
    {
    case 0:
        CommAttr.StopBits = ONESTOPBIT;
        break;
    case 1:
        CommAttr.StopBits = TWOSTOPBITS;
        break;
    default:
        CommAttr.StopBits = ONESTOPBIT;
        break;
    }

    CommAttr.fRtsControl = RTS_CONTROL_ENABLE;
    SetCommState(hCommFile, &CommAttr);

#else
    struct termios stNewTios, stOldTios;
    speed_t speed;

    tcgetattr(pPriv->iFd, &stOldTios);

    switch (iBaud)
    {
    case 600:
        speed = B600;
        break;
    case 1200:
        speed = B1200;
        break;
    case 2400:
        speed = B2400;
        break;
    case 4800:
        speed = B4800;
        break;
    case 9600:
        speed = B9600;
        break;
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
    case 57600:
        speed = B57600;
        break;
    case 115200:
        speed = B115200;
        break;
    default:
        speed = B9600;
        break;
    }

    cfsetspeed(&stNewTios, speed);
    stNewTios.c_cflag |= (CREAD | CLOCAL);
    stNewTios.c_cflag &= ~CSIZE;

    switch (iDataBit)
    {
    case 5:
        stNewTios.c_cflag |= CS5;
        break;
    case 6:
        stNewTios.c_cflag |= CS6;
        break;
    case 7:
        stNewTios.c_cflag |= CS7;
        break;
    case 8:
        stNewTios.c_cflag |= CS8;
        break;
    default:
        stNewTios.c_cflag |= CS8;
        break;
    }

    switch (iStopBit)
    {
    case 1:
        stNewTios.c_cflag &= ~CSTOPB;
        break;
    case 2:
        stNewTios.c_cflag |= CSTOPB;
        break;
    default:
        stNewTios.c_cflag &= ~CSTOPB;
        break;
    }

    switch (cParity)
    {
    case 'N':
    case 'n':
        stNewTios.c_cflag &= ~PARENB;
        stNewTios.c_iflag &= ~INPCK;
        break;
    case 'E':
    case 'e':
        stNewTios.c_cflag |= PARENB;
        stNewTios.c_cflag &= ~PARODD;
        stNewTios.c_iflag |= INPCK;
        break;
    case 'O':
    case 'o':
        stNewTios.c_cflag |= PARENB;
        stNewTios.c_cflag |= PARODD;
        stNewTios.c_iflag |= INPCK;
        break;
    default:
        stNewTios.c_cflag &= ~PARENB;
        stNewTios.c_iflag &= ~INPCK;
        break;
    }

    stNewTios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    stNewTios.c_iflag &= ~(IXON | IXOFF | IXANY);
    stNewTios.c_oflag &= ~OPOST;
    stNewTios.c_cc[VMIN] = 0;
    stNewTios.c_cc[VTIME] = 0;

    if (tcsetattr(pPriv->iFd, TCSANOW, &stNewTios))
    {
        tcsetattr(pPriv->iFd, TCSANOW, &stOldTios);
        return LS_ERR;
    }
#endif
    return LS_OK;
}

static LS_VOID setTimeout(CLASS_UART_T *pThis, LS_S32 iTimeoutMs)
{
    PRIVATE_T *pPriv = pThis->pPriv;

#ifdef _WIN32
    COMMTIMEOUTS commTimeout = {0};
    SetCommTimeouts(pThis->pPriv->hCommFile, &commTimeout);
    commTimeout.ReadTotalTimeoutConstant = iTimeoutMs;
#endif

    pPriv->iTimeoutMs = iTimeoutMs;

    return;
}

static LS_VOID flush(CLASS_UART_T *pThis)
{
    PRIVATE_T *pPriv = pThis->pPriv;
#ifdef _WIN32
    PurgeComm(pPriv->hCommFile, PURGE_RXABORT|PURGE_RXCLEAR);
#else
    tcflush(pPriv->iFd, TCIOFLUSH);
#endif
    return;
}

static LS_S32 writeData(CLASS_UART_T *pThis, LS_U8 *pucData, LS_S32 iLen)
{
    PRIVATE_T *pPriv = pThis->pPriv;
    LS_S32 iWriteLen = 0;

    if (NULL == pucData || iLen <= 0)
    {
        return LS_ERR;
    }

#ifdef _WIN32
    WriteFile(pPriv->hCommFile, pucData, iLen, &iWriteLen, NULL);

#else
    iWriteLen = write(pPriv->iFd, pucData, iLen);
#endif

    if (iWriteLen <= 0)
    {
        return LS_ERR;
    }

    return iWriteLen;
}

static LS_S32 readData(CLASS_UART_T *pThis, LS_U8 *pucData, LS_S32 iLen)
{
    PRIVATE_T *pPriv = pThis->pPriv;

    LS_S32 iReadLen = 0;

    if (NULL == pucData || iLen <= 0)
    {
        return LS_ERR;
    }

#ifdef _WIN32
    ReadFile(pPriv->hCommFile, pucData, iLen, &iReadLen, NULL);
#else
    while (cmnSelectRead(pPriv->iFd, pPriv->iTimeoutMs) > 0)
    {
        iReadLen += read(pPriv->iFd, &pucData[iReadLen], iLen - iReadLen);
        if (iReadLen >= iLen)
        {
            break;
        }
    }

    if (iReadLen <= 0)
    {
        return LS_ERR;
    }
#endif

    return iReadLen;
}

static LS_VOID del(CLASS_UART_T *pThis)
{
    LS_CLOSE(pThis->pPriv->iFd);
    LS_FREE(pThis->pPriv);
    LS_FREE(pThis);

    return;
}

CLASS_UART_T *new_CLASS_UART_T(LS_S8 *pszDev)
{
    CLASS_UART_T *pThis = NULL;
#ifdef _WIN32
    DCB CommAttr;
    COMMTIMEOUTS commTimeout = {0};
#else
    struct termios stTios;
#endif

    if (NULL == pszDev)
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis = calloc(1, sizeof(CLASS_UART_T))))
    {
        goto ERR_LABEL;
    }

    if (NULL == (pThis->pPriv = calloc(1, sizeof(PRIVATE_T))))
    {
        goto ERR_LABEL;
    }
#ifdef _WIN32
    pThis->pPriv->hCommFile = CreateFileA(pszDev, GENERIC_READ|GENERIC_WRITE, 0, NULL,
                                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == pThis->pPriv->hCommFile)
    {
        goto ERR_LABEL;
    }

    GetCommState(hCommFile, &CommAttr);
    CommAttr.BaudRate = CBR_9600;
    CommAttr.ByteSize = 8;
    CommAttr.Parity = NOPARITY;
    CommAttr.StopBits = ONESTOPBIT;
    CommAttr.fRtsControl = RTS_CONTROL_ENABLE;
    SetCommState(pThis->pPriv->hCommFile, &CommAttr);

    commTimeout.ReadTotalTimeoutConstant = M_TIMEOUT_DEF;
    if (false == SetCommTimeouts(pThis->pPriv->hCommFile, &commTimeout))
    {
        goto ERR_LABEL;
    }
#else
    if (-1 == (pThis->pPriv->iFd = open(pszDev, O_RDWR)))
    {
        goto ERR_LABEL;
    }

    bzero(&stTios, sizeof(stTios));
    cfsetispeed(&stTios, B9600);
    stTios.c_cflag |= (CREAD | CLOCAL);
    stTios.c_cflag &= ~CSIZE;
    stTios.c_cflag |= CS8;
    stTios.c_cflag &= ~CSTOPB;
    stTios.c_cflag &= ~PARENB;
    stTios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    stTios.c_iflag &= ~INPCK;
    stTios.c_iflag &= ~(IXON | IXOFF | IXANY);
    stTios.c_oflag &=~ OPOST;
    stTios.c_cc[VMIN] = 0;
    stTios.c_cc[VTIME] = 0;
    if (tcsetattr(pThis->pPriv->iFd, TCSANOW, &stTios) < 0)
    {
        goto ERR_LABEL;
    }
#endif

    pThis->pPriv->iTimeoutMs = M_TIMEOUT_DEF;

    pThis->setAttr = setAttr;
    pThis->setTimeout = setTimeout;
    pThis->flush = flush;
    pThis->write = writeData;
    pThis->read = readData;
    pThis->del = del;

    return pThis;

ERR_LABEL:
    del(pThis);

    return NULL;
}

#ifdef __cplusplus
}
#endif

