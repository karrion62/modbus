#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define CLIENTMAX 2

typedef struct server_tbl_s{
    struct sockaddr_in clientaddr, serveraddr;
    int ssock, csock;
    int recvbyte;
    int sendbyte;
    unsigned short modbus_cnt; // 0xffff 에서 1증가하면 자동으로 0이됨
    int client[CLIENTMAX];
    char client_ip[CLIENTMAX][20];
    unsigned char txbuf[512];
    unsigned char rxbuf[512];
} server_tbl_t;

typedef struct client_tbl_s{
    struct sockaddr_in clientaddr, serveraddr;
    int sock;
    int recvbyte;
    int sendbyte;
    unsigned short modbus_cnt; // 0xffff 에서 1증가하면 자동으로 0이됨
    unsigned char txbuf[512];
    unsigned char rxbuf[512];
} client_tbl_t;

int network_client_init(char *ip, int port);
int network_server_init(int port);
int net_send(int sock, unsigned char *buf, int blen);
int net_recv(int sock, unsigned char *buf);
int recv_wait(int fd, int ms);


#endif // __NETWORK_H__
