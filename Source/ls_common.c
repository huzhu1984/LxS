#include "ls_common.h"
#include "ls_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Table of CRC values for high-order byte */
static const LS_U8 table_crc_hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/* Table of CRC values for low-order byte */
static const LS_U8 table_crc_lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

/*
 * Calculate CRC16 value.
 */
LS_U16 cmnCrc16(LS_U8 *pucBuf, LS_U16 usLen)
{
    LS_U8 crc_hi = 0xFF;
    LS_U8 crc_lo = 0xFF;
    unsigned int i;

    while (usLen--) {
        i = crc_hi ^ *pucBuf++;
        crc_hi = crc_lo ^ table_crc_hi[i];
        crc_lo = table_crc_lo[i];
    }

    return (crc_hi << 8 | crc_lo);
}

/*
 * Modify a string to lower case.
 */
LS_VOID cmnStrLower(LS_S8 *pszStr)
{
    while (*pszStr != '\0')
    {
        if (*pszStr >= 'A' && *pszStr <= 'Z')
        {
            *pszStr += ('a' - 'A');
        }

        pszStr++;
    }

    return;
}

/*
 * Modify a string to upper case.
 */
LS_VOID cmnStrUpper(LS_S8 *pszStr)
{
    while (*pszStr != '\0')
    {
        if (*pszStr >= 'a' && *pszStr <= 'z')
        {
            *pszStr -= ('a' - 'A');
        }

        pszStr++;
    }

    return;
}

/*
 * Compare two strings and return how many first characters is same.
 */
LS_S32 cmnStrCmpCnt(LS_S8 *pszStr1, LS_S8 *pszStr2)
{
    LS_S32 i = 0;

    while ('\0' != pszStr1[i] && '\0' != pszStr2[i])
    {
        if (pszStr1[i] == pszStr2[i])
        {
            i++;
        }
        else
        {
            break;
        }
    }

    return i;
}

/*
 * Compare two strings ignoring spaces;
 */
LS_S32 cmnStrCmpNoCaseSpace(LS_S8 *pszStr1, LS_S8 *pszStr2)
{
    while ('\0' != *pszStr1 || '\0' != *pszStr2)
    {
        if (*pszStr1 == ' ')
        {
            pszStr1++;
            continue;
        }

        if (*pszStr2 == ' ')
        {
            pszStr2++;
            continue;
        }

        if (*pszStr1 == *pszStr2)
        {
            pszStr1++;
            pszStr2++;
        }
        else
        {
            return LS_ERR;
        }
    }

    return LS_OK;
}

/*
 * Sprintf a given data to a buffer in ASCII.
 */
LS_VOID cmnSprintfHex(LS_S8 *pszStr, LS_U8 *pucData, LS_S32 iSize)
{
    LS_S8 szVal[8] = {0};
    LS_S32 i;

    if (NULL == pszStr || NULL == pucData || iSize <= 0)
    {
        return;
    }

    memset(pszStr, 0x00, iSize);

    for (i=0; i<iSize; i++)
    {
        sprintf(szVal, "%02X ", pucData[i]);
        strcat(pszStr, szVal);

        if (0 == (i+1)%16)
        {
            strcat(pszStr, "\r\n");
        }
    }

    return;
}

/*
 * Printf a given data in ASCII.
 */
LS_VOID cmnPrintfHex(LS_U8 *pucData, LS_S32 iSize)
{
    LS_S32 i;

    if (NULL == pucData || iSize <= 0)
    {
        return;
    }

    for (i=0; i<iSize; i++)
    {
        if (0 == i % 16)
        {
            printf("0x%04X:", i);
        }

        if (0 == i % 8)
        {
            printf(" ");
        }

        printf("%02X ", pucData[i]);

        if (0 == (i + 1) % 16)
        {
            printf("\r\n");
        }
    }

    if (0 != (i+1)%16)
    {
        printf("\r\n");
    }

    return;
}

#ifdef _WIN32
    /* [TBD] */
#else
/*
 * Get IP address of given Eth*;
 */
LS_S32 cmnGetIpAddr(LS_S8 *pszEthName, in_addr_t *pIpAddrOut)
{
    LS_S32 iFd;
    struct ifreq stIfr;
    struct sockaddr_in *pstSockAddr;

    if (NULL == pszEthName || NULL == pIpAddrOut)
    {
        return LS_ERR;
    }

    if ((iFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return LS_ERR;
    }

    bzero(&stIfr, sizeof(stIfr));
    stIfr.ifr_addr.sa_family = AF_INET;
    strncpy(stIfr.ifr_name, pszEthName, IFNAMSIZ-1);

    if (0 == ioctl(iFd, SIOCGIFADDR, &stIfr))
    {
        pstSockAddr = (struct sockaddr_in*)(&(stIfr.ifr_addr));
        *pIpAddrOut = pstSockAddr->sin_addr.s_addr;
    }
    else
    {
        close(iFd);
        return LS_ERR;
    }

    close(iFd);
    return LS_OK;
}

/*
 * Set IP address of given Eth*;
 */
LS_S32 cmnGetMacAddr(LS_S8 *pszEthName, LS_U8 *pucMacAddrOut)
{
    LS_S32 iFd;
    struct ifreq stIfr;

    if (NULL == pszEthName || NULL == pucMacAddrOut)
    {
        return LS_ERR;
    }

    if ((iFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return LS_ERR;
    }

    bzero(&stIfr, sizeof(stIfr));
    stIfr.ifr_addr.sa_family = AF_INET;
    strncpy(stIfr.ifr_name, pszEthName, IFNAMSIZ-1);

    if (0 == ioctl(iFd, SIOCGIFHWADDR, &stIfr))
    {
        memcpy(pucMacAddrOut, stIfr.ifr_hwaddr.sa_data, 6);
    }
    else
    {
        close(iFd);
        return LS_ERR;
    }

    close(iFd);
    return LS_OK;
}

/*
 * Get MAC address of given Eth*;
 */
LS_S32 cmnSetIpAddr(LS_S8 *pszEthName, in_addr_t ipAddr)
{
    LS_S32 iFd;
    struct ifreq stIfr;
    struct sockaddr_in stSockAddr;

    if (NULL == pszEthName)
    {
        return LS_ERR;
    }

    if ((iFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return LS_ERR;
    }

    bzero(&stIfr, sizeof(stIfr));
    strncpy(stIfr.ifr_name, pszEthName, IFNAMSIZ-1);

    bzero(&stSockAddr, sizeof(stSockAddr));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_addr.s_addr = ipAddr;

    memcpy(&(stIfr.ifr_ifru.ifru_addr), &stSockAddr, sizeof(struct sockaddr_in));
    if (ioctl(iFd, SIOCSIFADDR, &stIfr) < 0)
    {
        close(iFd);
        return LS_ERR;
    }

    close(iFd);
    return LS_OK;
}

/*
 * Set MAC address of given Eth*;
 */
LS_S32 cmnSetMacAddr(LS_S8 *pszEthName, LS_U8 *pucMacAddr)
{
    LS_S32 iFd;
    struct ifreq stIfr;

    if (NULL == pszEthName || NULL == pucMacAddr)
    {
        return LS_ERR;
    }

    if ((iFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return LS_ERR;
    }

    bzero(&stIfr, sizeof(stIfr));
    stIfr.ifr_addr.sa_family = AF_INET;
    strncpy(stIfr.ifr_name, pszEthName, IFNAMSIZ-1);
    memcpy(stIfr.ifr_hwaddr.sa_data, pucMacAddr, 6);

    if (ioctl(iFd, SIOCSIFHWADDR, &stIfr) < 0)
    {
        close(iFd);
        return LS_ERR;
    }

    close(iFd);
    return LS_OK;
}

/*
 * Get system time as format of "Tue Feb 10 18:27:38 2004".
 */
LS_VOID cmnGetSysTime(LS_S8 *pszBuf, LS_S32 iBufSize)
{
    time_t iTime;
    struct tm stTime;

    if (NULL == pszBuf || iBufSize <= 0)
    {
        return;
    }

    time(&iTime);
    localtime_r(&iTime, &stTime);

    strftime(pszBuf, iBufSize, "%c", &stTime);

    return;
}

/*
 * Replace a line marked by given token in a configuration file and generate a .tmp file.
 */
LS_S32 cmnRplValInFile(LS_S8 *pszOldFile, LS_S8 *pszToken, LS_S8 *pszDlm, LS_S8 *pszNewVal)
{
    FILE *fpOld, *fpNew;
    LS_S32 iLen;
    LS_S8 szNewFile[256] = {0};
    LS_S8 szLineBuf[256] = {0};

    iLen = strlen(pszToken);

    if (NULL == pszOldFile || NULL == pszToken || NULL == pszDlm || NULL == pszNewVal || 0 == iLen)
    {
        return LS_ERR;
    }

    if (NULL == (fpOld = fopen(pszOldFile, "r")))
    {
        return LS_ERR;
    }

    strcpy(szNewFile, pszOldFile);
    strcat(szNewFile, ".tmp");
    if (NULL == (fpNew = fopen(szNewFile, "w")))
    {
        fclose(fpOld);
        return LS_ERR;
    }

    /* Generate the new file. */
    while (NULL != fgets(szLineBuf, 256, fpOld))
    {
        if (0 == strncmp(szLineBuf, pszToken, iLen))
        {
            strcpy(szLineBuf, pszToken);
            strcat(szLineBuf, pszDlm);
            strcat(szLineBuf, pszNewVal);
            strcat(szLineBuf, "\r\n");
            fputs(szLineBuf, fpNew);
        }
        else
        {
            fputs(szLineBuf, fpNew);
        }
    }

    fclose(fpOld);
    fclose(fpNew);

    remove(pszOldFile);
    rename(szNewFile, pszOldFile);

    return LS_OK;
}

/*
 * Select function encapsulation of reading fd.
 */
LS_S32 cmnSelectRead(LS_S32 iFd, LS_S32 iMs)
{
    fd_set rfds;
    struct timeval stTv;

    if (iFd < 0 || iMs < 0)
    {
        return LS_ERR;
    }

    FD_ZERO(&rfds);
    FD_SET(iFd, &rfds);

    stTv.tv_sec = iMs/1000;
    stTv.tv_usec = (iMs%1000)*1000;

    return select(iFd + 1, &rfds, NULL, NULL, &stTv);
}
#endif

#ifdef __cplusplus
}
#endif
