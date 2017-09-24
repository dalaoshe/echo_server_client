#include <iostream>

#include "unp.h"
#define SERVER_PORT 6666
#define TUNNEL_PORT 6668

#define MAX_LEN 1024*256

void do_echo_udp_client(char* server_ip, char* server_port, char* client_ip, char* client_port, char* file, int mode);
void do_echo_udp_server(char* server_ip, char* server_port, int mode);
void do_tunnel();

void do_echo_tcp_server();
void do_tcp_tunnel(char* serverport, char* tunnelport);
void do_echo_tcp_client(char* server_ip, char* server_port, char* client_ip, char* client_port, char* file, int mode);
void do_help() {
    const char* info = "Help Info\n";
    const char* options = "";
}


int main(int argc , char** argv) {
    std::cout << "Hello, World!" << std::endl;
    if(strcmp(argv[1],"1") == 0) {
        do_echo_udp_server(argv[2], argv[3], 0);
    }
    else if(strcmp(argv[1],"2") == 0){
        do_echo_udp_client(argv[2],argv[3],argv[4],argv[5], NULL, 0);
    }
    else if(strcmp(argv[1],"3") == 0){
        do_tunnel();
    }
    else if(strcmp(argv[1],"4") == 0) {
        do_echo_tcp_server();
    }
    else if(strcmp(argv[1],"5") == 0) {
        do_echo_tcp_client(argv[2],argv[3],argv[4],argv[5], NULL, 0);
    }
    else if(strcmp(argv[1],"6") == 0) {
        do_tcp_tunnel(argv[2], argv[3]);
    }
    return 0;
}


struct Msg{
    uint32_t len;
    uint8_t data[MAX_LEN];
};


void do_file_request(int fd, char* file, sockaddr_in* server , socklen_t* len) {

    FILE* f = fopen(file,"r");
    int n = 0;
    Msg msg;
    while((n = fread(msg.data,1,1,f) )!= 0) {
        msg.len = n;
        fprintf(stderr,"send Msg, len: %u, content: %s \n",msg.len, (char*)msg.data);
        timeval t;
        gettimeofday(&t, NULL);

        ssize_t s = sendto(fd, &msg, sizeof(uint32_t) + n, 0, (SA*)server, *len );
        if(s == sizeof(uint32_t) + n) {
            fprintf(stderr, "send success \n");
            ssize_t r = recvfrom(fd, &msg, sizeof(uint32_t) + n ,0, (SA*)server, len );
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
        sleep(1);
    }
}

void do_continue_qeuest(int fd, sockaddr_in* server , socklen_t* len) {
    int n = 4,count = 0;
    Msg msg;
    while(1) {
        msg.len = n;
        int *data = (int*)(msg.data);
        *data = count;
    	count++;

        fprintf(stderr,"Info:Try Send Msg, Len: %u, Content: %d \n",msg.len, *(int*)(msg.data));
        timeval t;
        gettimeofday(&t, NULL);

        ssize_t s = sendto(fd, &msg, sizeof(uint32_t) + n, 0, (SA*)server, *len );
        if(s == sizeof(uint32_t) + n) {
            fprintf(stderr, "Info:Client Send Success \n");
            ssize_t r = recvfrom(fd, &msg, sizeof(uint32_t) + n ,0, (SA*)server, len );
            if(r < 0 ) {
                fprintf(stderr,"Error:[%d,%d]Client Recv Error %s !!!!\n",__LINE__,__FILE__, strerror(errno));
            }
            else if(r == (sizeof(uint32_t) + msg.len)) {
                timeval t_2;
                gettimeofday(&t_2, NULL);
                double ms = (double (t_2.tv_sec-t.tv_sec) * 1000.0) + (double (t_2.tv_usec - t.tv_usec) / 1000.0);
                char buf[128];
                Inet_ntop(AF_INET, &(server->sin_addr.s_addr), buf, sizeof(buf));
                fprintf(stderr,"Info:Recv Reply From %s, Len:%u , Msg: %d, Use Time %lf \n",buf, msg.len, *(int*)(msg.data), ms);

            }
            else {
                fprintf(stderr,"Error:Recv Error Len %u \n",r);
            }

        }
        else if(s < 0) {
            fprintf(stderr,"Error:[%d,%d]Client Send Error, %s \n",__LINE__,__FILE__, strerror(errno));
        }
        else {
            fprintf(stderr,"Error:Client Send Error Len: %u \n",s);
        }
        sleep(3);
    }
}

void do_echo_udp_client(char* server_ip, char* server_port, char* client_ip, char* client_port, char* file, int mode) {
    sockaddr_in server,client;
    server.sin_family = client.sin_family = AF_INET;
    server.sin_port = htons(atoi(server_port));
    client.sin_port = htons(atoi(client_port));

    Inet_pton(AF_INET, server_ip, &server.sin_addr.s_addr);
    Inet_pton(AF_INET, client_ip, &client.sin_addr.s_addr);
    socklen_t len = sizeof(sockaddr_in);

    int fd = Socket(AF_INET, SOCK_DGRAM, 0);
    int on;
    SetSocket(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind_Socket(fd, (SA*)&client, len);

    if(file != NULL) {
        do_file_request(fd, file, &server, &len);
    }
    else {
        do_continue_qeuest(fd, &server, &len);
    }


}

void do_tunnel() {
    sockaddr_in server,client,tunnel;
    server.sin_family = client.sin_family = tunnel.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    tunnel.sin_port = htons(TUNNEL_PORT);

    server.sin_addr.s_addr = INADDR_ANY;
    tunnel.sin_addr.s_addr = INADDR_ANY;

    socklen_t len = sizeof(sockaddr_in);

    int fd = Socket(AF_INET, SOCK_DGRAM, 0);
    int on = 1;
    SetSocket(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind_Socket(fd, (SA*)&tunnel, sizeof(tunnel));

    Msg msg;
    size_t nbyte = sizeof(Msg);
    while (1) {
        //recv client
        memset(&msg,0,sizeof(Msg));
        ssize_t n = recvfrom(fd, &msg, nbyte,0, (SA*)&client, &len);
        if(n < 0 ) {
            fprintf(stderr,"[%d,%d] recv error %s \n",__LINE__,__FILE__, strerror(errno));
        }
        else if(n == (sizeof(uint32_t) + msg.len) || true) {
            fprintf(stderr,"recv len:%u , msg: %s\n",msg.len, (char*)msg.data);
            ssize_t s = sendto(fd, &msg, n, 0, (SA*)&server, len );
            timeval t;
            gettimeofday(&t, NULL);
            if(s == n) {
                fprintf(stderr, "reply server success \n");
            }
            else if(s < 0) {
                fprintf(stderr,"[%d,%d] send server error %s \n",__LINE__,__FILE__, strerror(errno));
            }
            else {
                fprintf(stderr,"send server error len %u \n",s);
            }
            //recv server reply
            memset(&msg,0,sizeof(Msg));
            ssize_t r = recvfrom(fd, &msg, n ,0, (SA*)&server, &len );
            if(r < 0 ) {
                fprintf(stderr,"[%d,%d] recv error %s \n",__LINE__,__FILE__, strerror(errno));
            }
            else if(r == (sizeof(uint32_t) + msg.len)) {
                timeval t_2;
                gettimeofday(&t_2, NULL);

                double ms = (double (t_2.tv_sec-t.tv_sec) * 1000.0) + (double (t_2.tv_usec - t.tv_usec) / 1000.0);

                fprintf(stderr,"recv reply, len:%u , msg: %d, use time %lf \n",msg.len, *(int*)(msg.data), ms);


            }
            else {
                fprintf(stderr,"recv  error len %u \n",r);
            }
            //reply client
            s = sendto(fd, &msg, r, 0, (SA*)&client, len );
            if(s == n) {
                fprintf(stderr, "reply client success \n");
            }
            else if(s < 0) {
                fprintf(stderr,"[%d,%d] send client error %s \n",__LINE__,__FILE__, strerror(errno));
            }
            else {
                fprintf(stderr,"send  client error len %u \n",s);
            }
        }
        else {
            fprintf(stderr,"recv  error len %u %u\n",n,msg.len);
        }
    }


}

void do_echo_udp_server(char* server_ip, char* server_port, int mode) {
    sockaddr_in server,client;
    server.sin_family = client.sin_family = AF_INET;
    server.sin_port = htons(atoi(server_port));
    Inet_pton(AF_INET, server_ip, &(server.sin_addr.s_addr));
    socklen_t len = sizeof(sockaddr_in);

    int fd = Socket(AF_INET, SOCK_DGRAM, 0);
    int on = 1;
    SetSocket(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind_Socket(fd, (SA*)&server, sizeof(server));
    Msg msg;
    size_t nbyte = sizeof(Msg);
    while (1) {
        memset(&msg,0,sizeof(Msg));
        ssize_t n = recvfrom(fd, &msg, nbyte, 0, (SA*)&client, &len);
        if(n < 0 ) {
            fprintf(stderr,"Error:[%d,%d] Recv Error %s \n",__LINE__,__FILE__, strerror(errno));
        }
        else if(n == (sizeof(uint32_t) + msg.len)) {
            char buf[128];
            Inet_ntop(AF_INET, &(client.sin_addr.s_addr), buf, sizeof(buf));
            fprintf(stderr,"Info:Recv From %s, Len :%u , Msg: %d\n",buf, msg.len, *(int*)msg.data);
            ssize_t s = sendto(fd, &msg, n, 0, (SA*)&client, len );
            if(s == n) {
                fprintf(stderr, "Info:Reply Success \n");
            }
            else if(s < 0) {
                fprintf(stderr,"Error:[%d,%d] Send Error %s \n",__LINE__,__FILE__, strerror(errno));
            }
            else {
                fprintf(stderr,"Error:Send Error Len %u \n",s);
            }
        }
        else {
            fprintf(stderr,"Error:Recv Error Len %u %u\n",n,msg.len);
        }
    }
}


void do_echo_tcp_server() {
    sockaddr_in server,client;
    server.sin_family = client.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    socklen_t len = sizeof(sockaddr_in);

    int fd = Socket(AF_INET, SOCK_STREAM, 0);
    int on = 1;
    SetSocket(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind_Socket(fd, (SA*)&server, sizeof(server));
    Listen(fd, 4);
    int socket = Accept(fd, (SA*)&client, &len);

    char client_ip[20];
    Inet_ntop(AF_INET, (char*)&client.sin_addr.s_addr, client_ip, sizeof(client_ip));
    fprintf(stderr, "recv connect from %s \n", client_ip);

    Msg msg;
    size_t nbyte = sizeof(Msg);

    while (1) {
        memset(&msg,0,sizeof(Msg));
        ssize_t n = read(socket, &msg, sizeof(Msg));
        if(n < 0 ) {
            fprintf(stderr,"[%d,%d] server recv error %s \n",__LINE__,__FILE__, strerror(errno));
        }
        else if(n == (sizeof(uint32_t) + msg.len) ) {
            fprintf(stderr,"server recv len:%u , msg: %s\n",msg.len, (char*)msg.data);
            ssize_t s = Write_nByte(socket, (char*)&msg, n);
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
        else if(n == 0){
            Close(socket);
            return;
            fprintf(stderr,"recv  error len %u %u\n",n,msg.len);
        }
    }
}

void do_continue_tcp_qeuest(int fd, sockaddr_in* server , socklen_t* len) {
    int n = 1024,count = 0;

    Msg msg;
    while(1) {
        msg.len = n;
        int *data = (int*)(msg.data);
	for(int i = 0 ; i < n; ++i){
		*(msg.data+i) = 1;	
	}
        *data = count;
        count++;

        fprintf(stderr,"Info:Try Send Msg, Len: %u, Content: %d \n",msg.len, *(int*)(msg.data));
        timeval t;
        gettimeofday(&t, NULL);

        ssize_t s = write(fd, &msg, sizeof(uint32_t) + n);
        if(s == sizeof(uint32_t) + n) {
            fprintf(stderr, "Info:Client Send Success \n");
            ssize_t r = read(fd, &msg, sizeof(uint32_t) + n );
            if(r < 0 ) {
                fprintf(stderr,"Error:[%d,%d]Client Recv Error %s !!!!\n",__LINE__,__FILE__, strerror(errno));
            }
            else if(r == (sizeof(uint32_t) + msg.len)) {
                timeval t_2;
                gettimeofday(&t_2, NULL);

                double ms = (double (t_2.tv_sec-t.tv_sec) * 1000.0) + (double (t_2.tv_usec - t.tv_usec) / 1000.0);
                char buf[128];
                Inet_ntop(AF_INET, &(server->sin_addr.s_addr), buf, sizeof(buf));
                fprintf(stderr,"Info:Recv Reply From %s, Len:%u , Msg: %d, Use Time %lf \n",buf, msg.len, *(int*)(msg.data), ms);
            }
            else {

                fprintf(stderr,"Error:Recv Error Len %u \n",r);
                return;
            }

        }
        else if(s < 0) {
            fprintf(stderr,"Error:[%d,%d]Client Send Error, %s \n",__LINE__,__FILE__, strerror(errno));
        }
        else {
            fprintf(stderr,"Error:Client Send Error Len: %u \n",s);
        }
        sleep(2);
    }
}

void do_echo_tcp_client(char* server_ip, char* server_port, char* client_ip, char* client_port, char* file, int mode) {
    sockaddr_in server,client;
    server.sin_family = client.sin_family = AF_INET;
    server.sin_port = htons(atoi(server_port));
    client.sin_port = htons(atoi(client_port));

    Inet_pton(AF_INET, server_ip, &server.sin_addr.s_addr);
    Inet_pton(AF_INET, client_ip, &client.sin_addr.s_addr);
    socklen_t len = sizeof(sockaddr_in);

    int fd = Socket(AF_INET, SOCK_STREAM, 0);
    int on;
    SetSocket(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind_Socket(fd, (SA*)&client, len);
    Socket_Peer_Connect(fd, (SA*)&server, len);

    if(file != NULL) {
        do_file_request(fd, file, &server, &len);
    }
    else {
        do_continue_tcp_qeuest(fd, &server, &len);
    }


}


void do_tcp_tunnel(char* serverport, char* tunnelport) {
    sockaddr_in server,client,tunnel;
    server.sin_family = client.sin_family = tunnel.sin_family = AF_INET;

    server.sin_port = htons(atoi(serverport));
    tunnel.sin_port = htons(atoi(tunnelport));

    server.sin_addr.s_addr = INADDR_ANY;
    tunnel.sin_addr.s_addr = INADDR_ANY;

    socklen_t len = sizeof(sockaddr_in);

    int listenTunnel = Socket(AF_INET, SOCK_STREAM, 0);

    int on = 1;
    SetSocket(listenTunnel, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind_Socket(listenTunnel, (SA*)&tunnel, sizeof(tunnel));
    Listen(listenTunnel, 4);
    while(1) {
        int client_fd =Accept(listenTunnel, (SA*)&client, &len);
        if(client_fd < 0) break;
        char client_ip[20];
        Inet_ntop(AF_INET, (char*)&client.sin_addr.s_addr, client_ip, sizeof(client_ip));
        fprintf(stderr, "tunnel recv connect from %s \n", client_ip);

        if(0) {
            continue;
        }
        else {

            int server_fd = Socket(AF_INET, SOCK_STREAM, 0);
            int on;
            Socket_Peer_Connect(server_fd, (SA*)&server, len);

            Msg msg;
            while (1) {
                memset(&msg,0,sizeof(Msg));
                int cn = read(client_fd, &msg, sizeof(Msg));
                if (cn < 0) {
                    fprintf(stderr, "read client sockfd %d error: %s \n", client_fd, strerror(errno));
                    Close(client_fd);
                    Close(server_fd);
                    break;
                } else if (cn == 0) {
                    fprintf(stderr, "client try to close close sockfd %d \n", client_fd);
                    Close(client_fd);
                    Close(server_fd);
                    break;
                } else {
                    int w = write(server_fd, &msg, cn);
                }

                memset(&msg,0,sizeof(Msg));
                int sn = read(server_fd, &msg, sizeof(Msg));
                if (sn < 0) {
                    fprintf(stderr, "read server sockfd %d error: %s \n", server_fd, strerror(errno));
                    Close(client_fd);
                    Close(server_fd);
                    break;
                } else if (sn == 0) {
                    fprintf(stderr, "server try to close close sockfd %d \n", server_fd);
                    Close(client_fd);
                    Close(server_fd);
                    break;
                } else {
                    int w = write(client_fd, &msg, sn);
                }

            }
            fprintf(stderr, "tunnel close \n", strerror(errno));
            exit(0);
        }
    }


}
