#include <iostream>

#include "unp.h"
#define SERVER_PORT 6666


#define MAX_LEN 1024

void do_echo_client(char* server_ip, char* file);
void do_echo_server();

int main(int argc , char** argv) {
    std::cout << "Hello, World!" << std::endl;
    if(strcmp(argv[1],"1") == 0) {
        do_echo_server();
    }
    else {
        do_echo_client(argv[2],argv[3]);
    }
    return 0;
}


struct Msg{
    uint32_t len;
    uint8_t data[MAX_LEN];
};


void do_echo_client(char* server_ip, char* file) {
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    Inet_pton(AF_INET, server_ip, &server.sin_addr.s_addr);
    socklen_t len = sizeof(sockaddr_in);

    int fd = Socket(AF_INET, SOCK_DGRAM, 0);
    int on;
    SetSocket(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    FILE* f = fopen(file,"r");
    int n = 0;

    Msg msg;
    while((n = fread(msg.data,1,1,f) )!= 0) {
        msg.len = n;
        fprintf(stderr,"send Msg, len: %u, content: %s \n",msg.len, (char*)msg.data);
        timeval t;
        gettimeofday(&t, NULL);

        ssize_t s = sendto(fd, &msg, sizeof(uint32_t) + n, 0, (SA*)&server, len );
        if(s == sizeof(uint32_t) + n) {
            fprintf(stderr, "send success \n");
            ssize_t r = recvfrom(fd, &msg, sizeof(uint32_t) + n ,0, (SA*)&server, &len );
            if(r < 0 ) {
                fprintf(stderr,"[%d,%d] recv error %s \n",__LINE__,__FILE__, strerror(errno));
            }
            else if(r == (sizeof(uint32_t) + msg.len)) {
                timeval t_2;
                gettimeofday(&t_2, NULL);

                double ms = (double (t_2.tv_sec-t.tv_sec) * 1000.0) + (double (t_2.tv_usec - t.tv_usec) / 1000.0);

                fprintf(stderr,"recv reply, len:%u , msg: %s, use time %lf \n",msg.len, (char*)msg.data, ms);


            }
            else {
                fprintf(stderr,"recv  error len %u \n",r);
            }

        }
        else if(s < 0) {
            fprintf(stderr,"[%d,%d] send error %s \n",__LINE__,__FILE__, strerror(errno));
        }
        else {
            fprintf(stderr,"send  error len %u \n",s);
        }
        sleep(10);
    }


}

void do_echo_server() {
    sockaddr_in server,client;
    server.sin_family = client.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    socklen_t len = sizeof(sockaddr_in);

    int fd = Socket(AF_INET, SOCK_DGRAM, 0);
    int on = 1;
    SetSocket(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind_Socket(fd, (SA*)&server, sizeof(server));
    Msg msg;
    size_t nbyte = sizeof(Msg);
    while (1) {
        memset(&msg,0,sizeof(Msg));
        ssize_t n = recvfrom(fd, &msg, nbyte,0, (SA*)&client, &len);
        if(n < 0 ) {
            fprintf(stderr,"[%d,%d] recv error %s \n",__LINE__,__FILE__, strerror(errno));
        }
        else if(n == (sizeof(uint32_t) + msg.len)) {
            fprintf(stderr,"recv len:%u , msg: %s\n",msg.len, (char*)msg.data);
            ssize_t s = sendto(fd, &msg, n, 0, (SA*)&client, len );
            if(s == n) {
                fprintf(stderr, "reply success \n");
            }
            else if(s < 0) {
                fprintf(stderr,"[%d,%d] send error %s \n",__LINE__,__FILE__, strerror(errno));
            }
            else {
                fprintf(stderr,"send  error len %u \n",s);
            }
        }
        else {
            fprintf(stderr,"recv  error len %u \n",n);
        }
    }
}