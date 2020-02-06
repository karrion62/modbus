#include <stdio.h>
#include "modbus.h"
#include "crc16.h"

int ttt(int *gg)
{
    static int b[10] = {1, 2, 3, 4, 5, 6, 7, 0};
    int cnt=0;
    printf("\n@@@@@@@\n");
    for (int i = 0; i < 5; i++)
    {
        gg[i] = b[i];
        printf("%d ", b[i]);
        cnt++;
    }
    printf("\n");

    return cnt;
}

int main(){
    int i;
    modbus_t test1, test2;

    unsigned char read[] = {0x01, 0x04, 0x5a, 0x00, 0x11, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x64, 0x01, 0xf9, 0x00, 0x00, 0x01, 0x7e, 0x01, 0x7a, 0x01, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x59, 0x00, 0xdc, 0x00, 0xdc, 0x00, 0xdc, 0x00, 0x48, 0x00, 0x48, 0x00, 0x48, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe9, 0x02, 0x59, 0x00, 0xf8, 0x07, 0xe3, 0x00, 0x0a, 0x00, 0x17, 0x00, 0x03, 0x00, 0x11, 0x00, 0x25, 0x00, 0x0f, 0x3d, 0xe6};
    unsigned char read2[] = {0x01, 0x05, 0x00, 0x08, 0xff, 0x00, 0x0d, 0xf8};

    printf("\n ----- create data test start ----- \n");
    modbus_frame(&test1, RTU);
    modbus_struct_init(&test1);
    printf("init1 %d %d %d %d %s %s\n", test1.modbus_frame, test1.transactionID, test1.txlen, test1.rxlen, test1.txbuf, test1.rxbuf);

    // make_modbus(주소, 기능코드, 시작번지, 요청개수, modbus_t)
    make_modbus(&test1, 1, 4, 0, 50);

    printf("\n<result1>\n");
    for (i = 0; i < test1.txlen; i++)
    {
        printf("txbuf %02d %02x\n", i, test1.txbuf[i]);
    }

    modbus_frame(&test2, TCP);
    modbus_struct_init(&test2);
    printf("init2 %d %d %d %d %s %s\n", test2.modbus_frame, test2.transactionID, test2.txlen, test2.rxlen, test2.txbuf, test2.rxbuf);

    // make_modbus(주소, 기능코드, 시작번지, 요청개수, modbus_t)
    make_modbus(&test2, 2, 4, 10, 40);
    printf("\n<result2>\n");
    for (i = 0; i < test2.txlen; i++)
    {
        printf("txbuf %02d %02x\n", i, test2.txbuf[i]);
    }
    printf("\n ----- create data test end ----- \n");

    printf("\n ----- read data test start ----- \n");

    for (i = 0; i < sizeof(read); i++)
    {
        test1.rxbuf[i] = read[i];
    }
    test1.rxlen = sizeof(read);
    read_modbus(&test1);

    for (i = 0; i < sizeof(read2); i++)
    {
        test2.txbuf[i] = test2.rxbuf[i] = read2[i];
    }
    test2.txlen = test2.rxlen = sizeof(read2);
    read_modbus(&test2);

    printf("\n\n ----- read data test end ----- \n");

    int a = 0x32b7;
    int b = 0x237b;
    printf("\n\n%x %x", a, b);
    printf("\n\n%x %x %x %x", High8(a), High8(b), Low8(a), Low8(b));

    crcBuf.a = 0x1f2f;
    printf("\n\n%x %x %x", crcBuf.a, crcBuf.b[0], crcBuf.b[1]);

    unsigned short crc = 0;

    char t[] = {0x02, 0xf2};
    crc = CRC16(t, 2);
    printf("\n\n%x %x %x [%x]", Low8(crc), High8(crc), crc, MOD_CRC(crc));

    crcBuf.a = CRC16(t, 2);
    printf("\n\n%x %x %x", crcBuf.b[0], crcBuf.b[1], crc);


    int gg[10];
    int ccc = ttt(gg);
    // printf("\n###### %d %d\n ", sizeof(gg), strlen(gg));
    for (int i = 0; i < ccc; i++)
    {
        printf("%d ", gg[i]);
    }
    printf("\n");

    return 1;
}