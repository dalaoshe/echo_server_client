#include <iostream>
#include "unp.h"
#include "LOG.h"
#include <getopt.h>

#define ARGS "hscC:S:P:p:k:R:m:"
struct Config{
    char* server_ip;
    char* client_ip;
    char* server_port;
    char* client_port;
    char* key;
    char* route_file;
    char* protocol;
    char type;

}config;
void usage() {
    fprintf(stderr, "-S <Server IP>\n"
            "-P <Server Port>\n"
            "-C <Client IP>\n"
            "-p <Client Port>\n"
            "-s Be A Server Mode\n"
            "-c Be A Client Mode\n"
            "-m <TCP/UDP> Use Tcp/Udp Mode\n"
            "-h Help\n");
}
void init_config() {
    config.client_port = config.server_port = config.server_ip = config.key = NULL;
    config.route_file = config.protocol = config.client_ip = NULL;
    config.type = 0;
}
void showConf(Config* conf) {
    fprintf(stderr, "SERVER IP:%s \n"
            "CLIENT IP:%s \n"
            "SERVER PORT:%s\n"
            "LOCAL PORT:%s\n"
            "PROTOCOL %s\n"
            "KEY:%s\n"
            "TYPE:%c\n", conf->server_ip,conf->client_ip, conf->server_port, conf->client_port, conf->protocol, conf->key, conf->type);
}

#define SERVER_PORT 6666
#define TUNNEL_PORT 6668


void do_echo_udp_client(char* server_ip, char* server_port, char* client_ip, char* client_port, char* file);
void do_echo_udp_server(char* server_ip, char* server_port);
void do_tunnel();

void do_echo_tcp_server(char* server_ip, char* server_port);
void do_tcp_tunnel(char* serverport, char* tunnelport);
void do_echo_tcp_client(char* server_ip, char* server_port, char* client_ip, char* client_port, char* file);
void do_help() {
    const char* info = "Help Info\n";
    const char* options = "";
}


int main(int argc , char** argv) {
    std::cout << "Hello, World!" << std::endl;
    LOG::init(stderr);
    char ch = 0;
    while( (ch = getopt(argc, argv, ARGS)) != -1)
        switch (ch) {
            case 'S':
                config.server_ip = optarg;
                break;
            case 'P':
                config.server_port = optarg;
                break;
            case 'p':
                config.client_port = optarg;
                break;
            case 'k':
                config.key = optarg;
                break;
            case 's':
                config.type = 's';
                break;
            case 'c':
                config.type = 'c';
                break;
            case 'C':
                config.client_ip = optarg;
                break;
            case 'm':
                config.protocol = optarg;
                break;
            case 'h':
                usage();
                exit(0);
            default:
                break;
        }
    showConf(&config);

    switch (config.type) {
        case 's':
            if(strcmp(config.protocol, "udp") == 0) {
                LOG_INFO("BE UDP SERVER\n");
                do_echo_udp_server(config.server_ip, config.server_port);
            }
            else if(strcmp(config.protocol, "tcp") == 0) {
                LOG_INFO("BE TCP SERVER\n");
                do_echo_tcp_server(config.server_ip, config.server_port);
            }
            else {
                LOG_INFO("UNKNOWN TRANSPORT PROTOCOL:%s\n", config.protocol);
            }
            break;
        case 'c':
            if(strcmp(config.protocol, "udp") == 0) {
                LOG_INFO("BE UDP CLIENT\n");
                do_echo_udp_client(config.server_ip, config.server_port, config.client_ip, config.client_port, NULL);
            }
            else if(strcmp(config.protocol, "tcp") == 0) {
                LOG_INFO("BE TCP CLIENT\n");
                do_echo_tcp_client(config.server_ip, config. server_port, config.client_ip, config.client_port, NULL);
            }
            else {
                LOG_INFO("UNKNOWN TRANSPORT PROTOCOL:%s\n", config.protocol);
            }
            break;
        default:
            break;
    }

//    if(strcmp(argv[1],"1") == 0) {
//        do_echo_udp_server(argv[2], argv[3], 0);
//    }
//    else if(strcmp(argv[1],"2") == 0){
//        do_echo_udp_client(argv[2],argv[3],argv[4],argv[5], NULL, 0);
//    }
//    else if(strcmp(argv[1],"3") == 0){
//        do_tunnel();
//    }
//    else if(strcmp(argv[1],"4") == 0) {
//        do_echo_tcp_server();
//    }
//    else if(strcmp(argv[1],"5") == 0) {
//        do_echo_tcp_client(argv[2],argv[3],argv[4],argv[5], NULL, 0);
//    }
//    else if(strcmp(argv[1],"6") == 0) {
//        do_tcp_tunnel(argv[2], argv[3]);
//    }
    return 0;
}


struct Msg{
    uint32_t len;
    uint8_t data[MAX_LEN];
};


void do_file_request(int fd, char* file, sockaddr_in* server , socklen_t* len) {

    FILE* f = fopen(file,"r");
    size_t n = 0;
    Msg msg;
    while((n = fread(msg.data,1,1,f) )!= 0) {
        msg.len = n;
        fprintf(stderr,"send Msg, len: %u, content: %s \n",msg.len, (char*)msg.data);
        timeval t;
        gettimeofday(&t, NULL);

        ssize_t s = sendto(fd, &msg, sizeof(uint32_t) + n, 0, (SA*)server, *len );
        if(s == sizeof(uint32_t) + n) {
            LOG_INFO("Client Send Success \n");
            ssize_t r = recvfrom(fd, &msg, sizeof(uint32_t) + n ,0, (SA*)server, len );
            if(r < 0 ) {
                LOG_ERROR("Client Recv Error %s !!!!\n", strerror(errno));
            }
            else if(r == (sizeof(uint32_t) + msg.len)) {
                timeval t_2;
                gettimeofday(&t_2, NULL);

                double ms = (double (t_2.tv_sec-t.tv_sec) * 1000.0) + (double (t_2.tv_usec - t.tv_usec) / 1000.0);
                char buf[128];
                Inet_ntop(AF_INET, &(server->sin_addr.s_addr), buf, sizeof(buf));
                LOG_INFO("Recv Reply From %s, Len:%u , Msg: %d, Use Time %lf \n",buf, msg.len, *(int*)(msg.data), ms);


            }
            else {
                LOG_ERROR("Recv Error Len %ld \n",r);
            }

        }
        else if(s < 0) {
            LOG_ERROR("Client Send Error, %s \n", strerror(errno));
        }
        else {
            LOG_ERROR("Client Send Error Len: %ld \n",s);
        }
        sleep(1);
    }
}

void do_continue_qeuest(int fd, sockaddr_in* server , socklen_t* len) {
    uint32_t n = 4; int count = 0;
    Msg msg;
    while(1) {
        msg.len = n;
        int *data = (int*)(msg.data);
        *data = count;
    	count++;
        LOG_INFO("Try Send Msg, Len: %u, Content: %d\n", msg.len, *(int*)(msg.data));
        timeval t;
        gettimeofday(&t, NULL);

        ssize_t s = sendto(fd, &msg, sizeof(uint32_t) + n, 0, (SA*)server, *len );
        if(s == sizeof(uint32_t) + n) {
            LOG_INFO("Client Send Success \n");
            ssize_t r = recvfrom(fd, &msg, sizeof(uint32_t) + n ,0, (SA*)server, len );
            if(r < 0 ) {
                LOG_ERROR("Client Recv Error %s !!!!\n", strerror(errno));
            }
            else if(r == (sizeof(uint32_t) + msg.len)) {
                timeval t_2;
                gettimeofday(&t_2, NULL);
                double ms = (double (t_2.tv_sec-t.tv_sec) * 1000.0) + (double (t_2.tv_usec - t.tv_usec) / 1000.0);
                char buf[128];
                Inet_ntop(AF_INET, &(server->sin_addr.s_addr), buf, sizeof(buf));
                LOG_INFO("Recv Reply From %s, Len:%u , Msg: %d, Use Time %lf \n",buf, msg.len, *(int*)(msg.data), ms);

            }
            else {
                LOG_ERROR("Recv Error Len %ld \n",r);
            }

        }
        else if(s < 0) {
            LOG_ERROR("Client Send Error, %s \n", strerror(errno));
        }
        else {
            LOG_ERROR("Client Send Error Len: %ld \n",s);
        }
        sleep(3);
    }
}

void do_echo_udp_client(char* server_ip, char* server_port, char* client_ip, char* client_port, char* file) {
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
            LOG_ERROR("When Tun Recv Client, %s\n", strerror(errno));
        }
        else if(n == (sizeof(uint32_t) + msg.len) || true) {
            char buf[128];
            Inet_ntop(AF_INET, &(client.sin_addr.s_addr), buf, sizeof(buf));
            LOG_INFO("Recv From Client:%s, Len :%u , Msg: %d\n",buf, msg.len, *(int*)msg.data);
            ssize_t s = sendto(fd, &msg, n, 0, (SA*)&server, len );
            timeval t;
            gettimeofday(&t, NULL);
            if(s == n) {
                LOG_INFO("Send To Sever Success \n");
            }
            else if(s < 0) {
                LOG_ERROR("When Send To Server, %s\n", strerror(errno));
            }
            else {
                LOG_ERROR("Send To Server Error Len %ld \n",s);
            }
            //recv server reply
            memset(&msg,0,sizeof(Msg));
            ssize_t r = recvfrom(fd, &msg, n ,0, (SA*)&server, &len );
            if(r < 0 ) {
                LOG_ERROR("When Tun Recv Server, %s\n", strerror(errno));
            }
            else if(r == (sizeof(uint32_t) + msg.len)) {
                timeval t_2;
                gettimeofday(&t_2, NULL);
                double ms = (double (t_2.tv_sec-t.tv_sec) * 1000.0) + (double (t_2.tv_usec - t.tv_usec) / 1000.0);
                LOG_INFO("Tun Recv Reply From Sever:%s, Len:%u , Msg: %d, Use Time %lf \n",buf, msg.len, *(int*)(msg.data), ms);
            }
            else {
                LOG_ERROR("Tun Recv Error Len:%ld From Sever\n",r);
            }
            //reply client
            s = sendto(fd, &msg, r, 0, (SA*)&client, len );
            if(s == n) {
                LOG_INFO("Tun Reply Client Success \n");
            }
            else if(s < 0) {
                LOG_ERROR("When Tun Reply Client, %s\n", strerror(errno));
            }
            else {
                LOG_ERROR("Tun Send Error Len %ld \n",s);
            }
        }
        else {
            LOG_ERROR("Recv Error Len From Client, n:%ld, msg.len:%u\n",n, msg.len);
        }
    }


}

void do_echo_udp_server(char* server_ip, char* server_port) {
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
            LOG_ERROR("When Server Recv, %s\n", strerror(errno));
        }
        else if(n == (sizeof(uint32_t) + msg.len)) {
            char buf[128];
            Inet_ntop(AF_INET, &(client.sin_addr.s_addr), buf, sizeof(buf));
            LOG_INFO("Recv From %s, Len :%u , Msg: %d\n",buf, msg.len, *(int*)msg.data);
            ssize_t s = sendto(fd, &msg, n, 0, (SA*)&client, len );
            if(s == n) {
                LOG_INFO("Sever Reply Success \n");
            }
            else if(s < 0) {
                LOG_ERROR("When Server Reply, %s\n", strerror(errno));
            }
            else {
                LOG_ERROR("Sever Send Error Len %ld \n",s);
            }
        }
        else {
            LOG_ERROR("Recv Error Len %ld %u\n",n, msg.len);
        }
    }
}


void do_echo_tcp_server(char* server_ip, char* server_port) {
    sockaddr_in server,client;
    server.sin_family = client.sin_family = AF_INET;
    server.sin_port = htons(atoi(server_port));
    Inet_pton(AF_INET, server_ip, &(server.sin_addr.s_addr));
    socklen_t len = sizeof(sockaddr_in);

    int fd = Socket(AF_INET, SOCK_STREAM, 0);
    int on = 1;
    SetSocket(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind_Socket(fd, (SA*)&server, sizeof(server));
    Listen(fd, 4);
    int socket = Accept(fd, (SA*)&client, &len);

    char client_ip[20];
    Inet_ntop(AF_INET, (char*)&client.sin_addr.s_addr, client_ip, sizeof(client_ip));
    LOG_INFO("Recv Connect From %s\n", client_ip);

    Msg msg;

    while (1) {
        memset(&msg,0,sizeof(Msg));
        ssize_t n = read(socket, (char*)&msg, sizeof(uint32_t));
        if(n < 0 ) {
            LOG_ERROR("When Server Recv Msg.len, %s\n", strerror(errno));
        }
        else if(n == sizeof(uint32_t) ) {
            n = read(socket, msg.data, msg.len);
            if(n < 0 ) {
                LOG_ERROR("When Server Recv Msg.data, %s\n", strerror(errno));
            }
            else if(n == msg.len) {
                LOG_INFO("Recv From %s, Len :%u , Msg: %d\n", client_ip, msg.len, *(int *) msg.data);
                ssize_t s = Write_nByte(socket, (char *) &msg, n);
                if (s == n) {
                    LOG_INFO("Sever Reply Success \n");
                } else if (s < 0) {
                    LOG_ERROR("When Server Reply, %s\n", strerror(errno));
                } else {
                    LOG_ERROR("Sever Send Error Len %ld \n", s);
                }
            }
            else if(n == 0){
                Close(socket);
                LOG_INFO("Recv Disconnect From %s\n", client_ip);
                return;
            }
        }
        else if(n == 0){
            Close(socket);
            LOG_INFO("Recv Disconnect From %s\n", client_ip);
            return;
        }
    }
}

void do_continue_tcp_qeuest(int fd, sockaddr_in* server , socklen_t* len) {
    uint32_t n = 1024; int count = 0;

    Msg msg;
    while(1) {
        msg.len = n;
        int *data = (int*)(msg.data);
        for(int i = 0 ; i < n; ++i){
            *(msg.data+i) = 1;
        }
        *data = count;
        count++;
        LOG_INFO("Try Send Msg, Len: %u, Content: %d\n", msg.len, *(int*)(msg.data));
        timeval t;
        gettimeofday(&t, NULL);
        ssize_t s = Write_nByte(fd, (char*)&msg, sizeof(uint32_t) + n);
        if(s == sizeof(uint32_t) + n) {
            LOG_INFO("Client Send Success \n");
            ssize_t r = read(fd, (char*)&msg.len, sizeof(uint32_t));
            if(r < 0 ) {
                LOG_ERROR("Client Recv Msg.len Error, %s !!!!\n", strerror(errno));
            }
            else if(r == sizeof(uint32_t)) {
                timeval t_2;
                gettimeofday(&t_2, NULL);
                r = read(fd, (char*)&msg.data, msg.len);
                if(r < 0 ) {
                    LOG_ERROR("Client Recv Msg.data Error, %s !!!!\n", strerror(errno));
                }
                else if(r == msg.len) {
                    double ms = (double (t_2.tv_sec-t.tv_sec) * 1000.0) + (double (t_2.tv_usec - t.tv_usec) / 1000.0);
                    char buf[128];
                    Inet_ntop(AF_INET, &(server->sin_addr.s_addr), buf, sizeof(buf));
                    LOG_INFO("Recv Reply From %s, Len:%u , Msg: %d, Use Time %lf \n",buf, msg.len, *(int*)(msg.data), ms);
                }
                else if(r == 0) {
                    Close(fd);
                    char buf[128];
                    Inet_ntop(AF_INET, &(server->sin_addr.s_addr), buf, sizeof(buf));
                    LOG_INFO("Recv Disconnect From %s\n", buf);
                    return;
                }
                else {
                    LOG_ERROR("Recv Error Msg.data len %ld %u\n",n, msg.len);
                }
            }
            else if(r == 0){
                Close(fd);
                char buf[128];
                Inet_ntop(AF_INET, &(server->sin_addr.s_addr), buf, sizeof(buf));
                LOG_INFO("Recv Disconnect From %s\n", buf);
                return;
            }
            else {
                LOG_ERROR("Recv Error Msg.len %ld\n",n);
            }
        }
        else if(s < 0) {
            LOG_ERROR("Client Send Error, %s \n", strerror(errno));
        }
        else {
            LOG_ERROR("Client Send Error Len: %ld \n",s);
        }
        sleep(2);
    }
}

void do_echo_tcp_client(char* server_ip, char* server_port, char* client_ip, char* client_port, char* file) {
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
            fprintf(stderr, "tunnel close \n");
            exit(0);
        }
    }


}
