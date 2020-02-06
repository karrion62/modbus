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

#include "network.h"
// #include "_etc.h"
// #include "log.h"

int network_client_init(char *ip, int port){
    //struct hostent *host;
    //struct sockaddr_in server_addr;
    int sock;

    printf("ip %s", ip);
    printf("port %d", port);
    //host = gethostbyname(ip);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket err");
        return -1;
    }

/*
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    memset(&(server_addr.sin_zero), 0x00, 8);

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1) {
        perror("Connect err");
        return -1;
    }
*/

    return sock;
}

int network_server_init(int port){
    //struct hostent *host;
    struct sockaddr_in server_addr;
    int sock=0;
    int option=1, state=0;;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket err");
        return -1;
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    memset(&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    state = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (state == -1) {
        perror("bind error : ");
        exit(0);
    }

    state = listen(sock, CLIENTMAX);
    if (state == -1) {
        perror("listen error : ");
        exit(0);
    }

    return sock;
}

// 데이터 수신전에 해준다 세팅한 ms만큼 대기 fd로 데이터가 날아오면 데이터를 읽어오면 된다
int recv_wait(int fd, int ms){ // select
    int state;

    struct timeval tv;
    fd_set readfds;//, writefds;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    tv.tv_sec = ms/1000;
    tv.tv_usec = (ms%1000)*1000; // 초에서 마이크로 세컨으로 변경

    state = select(fd + 1, &readfds, (fd_set *)0, (fd_set *)0, &tv);
    switch(state)
    {
        case -1:
            printf("select error : %d", state);
            printf("%s", strerror(errno));
            return -1;
        case 0:
            printf("select Time over");
            return 0;
        default:
            printf("select recv data");
            return state;
    }
}

int net_send(int sock, unsigned char *buf, int blen){
    int len = 0;
    len = send(sock, buf, blen, 0);

    return len;
}

int net_recv(int sock, unsigned char *buf){
    int len = 0;
    len = read(sock, buf, 512);

    return len;
}
