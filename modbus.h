#include "crc16.h"

#define RTU 1000
#define TCP 1001
#define TCPRTU 1002

#define PCS 2001
#define BAT 2002

#define High8(x) ((x & 0xff00) >> 8)
#define Low8(x) (x & 0x00ff)
#define MOD_CRC(x) (Low8(x) + High8(x))

typedef struct modbus_s
{
    unsigned char txbuf[32];
    unsigned char rxbuf[1024];
    int data[125];
    int datalen;
    int startaddr;
    int txlen;
    int rxlen;
    int transactionID;
    int modbus_frame;
    char read_flag;
} modbus_t;

void modbus_struct_init(modbus_t *modbus);
void modbus_frame(modbus_t *modbus, int frame);
int read_modbus(modbus_t *modbus);
int make_modbus(modbus_t *modbus, int addr, int fcode, int start_addr, int request_cnt);
int modbus_TCP(modbus_t *modbus, unsigned char *data, int mcnt, int len);

