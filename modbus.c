#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include "modbus.h"

#define RTU_STEP 3
#define TCP_STEP 9

// ################################################
//    modbus 기능코드 01~06번까지 지원 가능한 버전입니다
// ################################################

// RTU 형식을 TCP로 변경한다
int modbus_tcp(modbus_t *modbus, unsigned char *data, int mcnt, int len)
{
    int i = 0, cnt = 0;

    modbus->txbuf[cnt++] = (mcnt & 0xff00) >> 8;
    modbus->txbuf[cnt++] = (mcnt & 0x00ff);
    modbus->txbuf[cnt++] = 0x00;
    modbus->txbuf[cnt++] = 0x00;
    modbus->txbuf[cnt++] = (len & 0xff00) >> 8;
    modbus->txbuf[cnt++] = (len & 0x00ff);

    for (i = 0; i < len; i++)
    {
        modbus->txbuf[cnt++] = data[i];
    }

    return cnt;
}

// 내부 사용 함수
static int crc_check(modbus_t *modbus) // RTU 에서만 사용함
{
    int size = modbus->rxbuf[2] + 3; // 데이터의 길이 + addr(1) + fcode(1) + length(1)
    unsigned short crc = 0;
    if (modbus->txlen == modbus->rxlen)
    {
        if (memcmp(modbus->txbuf, modbus->rxbuf, modbus->txlen) == 0)
        {
            return 1; // 기능코드 5, 6에 적용, 동일 데이터를 수신하여 데이터 확인 종료
        }
    }
    else if (modbus->txlen != modbus->rxlen)
    {
        crc = CRC16(modbus->rxbuf, (unsigned char)(size - 2));                           // -2를 하는 이유 수신한 데이터에서 CRC 2개를 제외 시킨다
        if (Low8(crc) == modbus->rxbuf[size - 2] && High8(crc) == modbus->rxbuf[size - 1])    // 제외한 CRC 데이터와 새로만든 CRC를 비교한다
        {
            return 2; // 기능코드 3, 4에 적용
        }
    }
    return 0;
}

// 내부 사용 함수
static int header_check(modbus_t *modbus) // TCP 에서만 사용함, 보낸 데이터와 수신 데이터의 헤더가 동일하면 된다
{
    if (modbus->txlen == modbus->rxlen)
    {
        if (memcmp(modbus->txbuf, modbus->rxbuf, modbus->txlen) == 0)
        {
            return 1; // 기능코드 5, 6에 적용, 동일 데이터를 수신하여 데이터 확인 종료
        }
    }
    else if (modbus->txlen != modbus->rxlen)
    {
        if ((modbus->txbuf[0] == modbus->rxbuf[0]) && (modbus->txbuf[1] == modbus->rxbuf[1]) && // Transaction id
            (modbus->txbuf[2] == modbus->rxbuf[2]) && (modbus->txbuf[3] == modbus->rxbuf[3]) && // Protocol id
            (modbus->txbuf[6] == modbus->rxbuf[6])) // Unit id
        {
            return 2;
        }
    }
    return -1;
}

// 기능 코드 1, 2, 3, 4, 5, 6번에 해당하는 데이터를 만든다
// 데이터 만드는 함수 type 통신 타입, addr 장비 주소값, fcode 기능 코드, start_addr 요청 시작 주소, request_cnt 요청 데이터 개수
int make_modbus(modbus_t *modbus, int addr, int fcode, int start_addr, int request_cnt)
{
    unsigned char mbuf[6]={0,}; // 데이터 길이는 무조건 6을 넘을수 없다
    int cnt=0;
    if(request_cnt > 125) request_cnt = 125;

    printf("\nmake_modbus addr %02x fcode %02x saddr %02x rcnt %02x\n", addr, fcode, start_addr, request_cnt);
    memset(modbus->txbuf, 0, sizeof(modbus->txbuf));
    modbus->txlen = 0;

    // 실제 데이터
    modbus->txbuf[modbus->txlen++] = mbuf[cnt++] = (unsigned char)addr;
    modbus->txbuf[modbus->txlen++] = mbuf[cnt++] = (unsigned char)fcode;
    modbus->txbuf[modbus->txlen++] = mbuf[cnt++] = (unsigned char)((start_addr >> 8) & 0xff);
    modbus->txbuf[modbus->txlen++] = mbuf[cnt++] = (unsigned char)(start_addr & 0xff);

    if (fcode == 5) // 장비를 on / off를 관제하는 기능코드 request_cnt 0 은 off request_cnt 1 은 on으로 한다
    {
        if (request_cnt) // on
        {
            modbus->txbuf[modbus->txlen++] = mbuf[cnt++] = (unsigned char)0xff;
        }
        else // off
        {
            modbus->txbuf[modbus->txlen++] = mbuf[cnt++] = (unsigned char)0x00;
        }
    }
    if (1 <= fcode && fcode <= 6)
    {
        modbus->txbuf[modbus->txlen++] = mbuf[cnt++] = (unsigned char)((request_cnt >> 8) & 0xff);
        modbus->txbuf[modbus->txlen++] = mbuf[cnt++] = (unsigned char)(request_cnt & 0xff);
    }

    if (modbus->modbus_frame == RTU || modbus->modbus_frame == TCPRTU) // CRC 사용
    {
        // CRC 데이터
        unsigned short crc = 0;
        crc = CRC16(modbus->txbuf, modbus->txlen); // modbus->txbuf를 직접 넣으면 crc가 이상해져서 mbuf를 따로 사용함

        modbus->txbuf[modbus->txlen++] = Low8(crc); 
        modbus->txbuf[modbus->txlen++] = High8(crc);

        printf("\nRTU\n");
    }
    else if (modbus->modbus_frame == TCP) // CRC 사용 안함
    {
        modbus->txbuf[0] = mbuf[0] = (unsigned char)0x01; // TCP에서는 addr이 사용되지 안는다 무조건 1이 들어간다
        
        memset(modbus->txbuf, 0, sizeof(modbus->txbuf));
        modbus->txlen = 0;

        modbus->txlen = modbus_tcp(modbus, mbuf, modbus->transactionID++, cnt);

        printf("\nTCP\n");
    }

    return 0;
}

// 정상 0, 오류 -1
int read_modbus(modbus_t *modbus)
{
    int ret = 0;
    modbus->read_flag = 0;
    if (modbus->modbus_frame == RTU || modbus->modbus_frame == TCPRTU)
    {
        modbus->read_flag = crc_check(modbus);
        if (modbus->read_flag == 2)
        {
            printf("\n정상 데이터");
            memset(modbus->data, 0, sizeof(modbus->data));
            modbus->datalen = modbus->startaddr = 0;

            modbus->startaddr = (modbus->txbuf[2] << 8) + modbus->txbuf[3];
            modbus->datalen = modbus->rxbuf[2] / 2;
            for (int i = 0; i < modbus->datalen; i ++)
            {
                modbus->data[i] = (modbus->rxbuf[RTU_STEP + (i * 2 + 0)] << 8) + modbus->rxbuf[RTU_STEP + (i * 2) + 1];
            }
            ret = 0;
        }
        else if (modbus->read_flag == 1)
        {
            printf("\n정상 데이터");
            ret = 0;
        }
        else
        {
            printf("\n이상 데이터");
            ret = -1;
        }
    }
    else if (modbus->modbus_frame == TCP)
    {
        modbus->read_flag = header_check(modbus);
        if (modbus->read_flag == 2)
        {
            printf("\n정상 데이터");
            memset(modbus->data, 0, sizeof(modbus->data));
            modbus->datalen = modbus->startaddr = 0;

            modbus->startaddr = (modbus->txbuf[2] << 8) + modbus->txbuf[3];
            modbus->datalen = modbus->rxbuf[2] / 2;
            for (int i = 0; i < modbus->datalen; i++)
            {
                modbus->data[i] = (modbus->rxbuf[TCP_STEP + (i * 2 + 0)] << 8) + modbus->rxbuf[TCP_STEP + (i * 2) + 1];
            }
            ret = 0;
        }
        else if (modbus->read_flag == 1)
        {
            printf("\n정상 데이터");
            ret = 0;
        }
        else
        {
            printf("\n이상 데이터");
            ret = -1;
        }
    }

    return ret;
}

// 구조체 frame 선언
void modbus_frame(modbus_t *modbus, int frame)
{
    modbus->modbus_frame = frame;
}

// 구조체 데이터 전체 초기화
void modbus_struct_init(modbus_t *modbus)
{
    modbus->datalen = 0;
    modbus->startaddr = 0;
    modbus->txlen = 0;
    modbus->rxlen = 0;
    modbus->transactionID = 0;
    modbus->modbus_frame = 0;
    modbus->read_flag = 0;
    
    memset(modbus->data, 0, sizeof(modbus->data));
    memset(modbus->txbuf, 0, sizeof(modbus->txbuf));
    memset(modbus->rxbuf, 0, sizeof(modbus->rxbuf));
}
