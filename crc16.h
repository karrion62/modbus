#ifndef __CRC16_H__
#define __CRC16_H__

unsigned short CRC16(const unsigned char *nData, unsigned char wLength);

union {
    unsigned short a;
    unsigned char b[2];
} crcBuf;

#endif // __CRC16_H__
