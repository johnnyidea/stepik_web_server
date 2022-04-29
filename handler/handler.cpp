/*! \file handler.cpp
 * \brief Handler class implementation.
 *
 * \authors rene
 * \date April 2022
 */

//=======================================================================================
#include <iostream>
#include <algorithm>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "handler.h"

#include "../third_party/httpparser/src/httpparser/request.h"
#include "../third_party/httpparser/src/httpparser/httprequestparser.h"

#define MAX_EVENTS 32

using namespace std;
using namespace httpparser;

//=======================================================================================
int set_nonblock(int fd)
{
    int flags;

#if defined(O_NONBLOCK)
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;

    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1;
    return fcntl(fd, FIOBIO, &flags);
#endif
}
//=======================================================================================
Handler::Handler(const WebParams &web_params)
        :_master_socket_fd(socket(AF_INET, SOCK_STREAM, 0)),
         _dir(web_params.get_dir())
{
    int opt = 1;

    if (_master_socket_fd < 0)
    {
        cerr << "Negative socket fd";
        exit(EXIT_FAILURE);
    }

    if (setsockopt(_master_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        cerr << "Couldn't set setsockopt()";
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(web_params.get_port());
    sock_addr.sin_addr.s_addr = inet_addr(web_params.get_ip().c_str());

    if (bind(_master_socket_fd, (struct sockaddr*)(&sock_addr), sizeof(sock_addr)) < 0)
    {
        cerr << "Couldn't bind";
        exit(EXIT_FAILURE);
    }

    set_nonblock(_master_socket_fd);
}
//=======================================================================================
void Handler::run()
{
    if (!fork())
    {
        //daemon
        setsid();
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        chdir(_dir.c_str());

        if (listen(_master_socket_fd, SOMAXCONN) < 0)
        {
            cerr << "Couldn't listen";
            exit(EXIT_FAILURE);
        }

        while (true)
        {
            int slave_socket = accept(_master_socket_fd, 0, 0);
            set_nonblock(slave_socket);

            thread t(_http_handle, slave_socket);

            t.detach();
        }
    }
}

bool check_get(std::string request)
{
    return ( (request.size() > 3) && ( "GET" == request.substr(0, 3) ) );
}

bool do_get(int socket, std::string request)
{
    if (std::string::npos != request.find("?"))
        request = request.substr(0, request.find("?"));

    if (std::string::npos != request.find("HTTP"))
        request = request.substr(0, request.find("HTTP"));

    std::string fullpath = request.substr(request.find("/"), request.size() - request.find("/"));
    fullpath = "." + fullpath;
    fullpath.erase(std::remove(fullpath.begin(), fullpath.end(), ' '), fullpath.end());
    int fd = open(fullpath.c_str(), O_RDONLY);
    if (-1 == fd) //if cannot open file
    {
        std::string responce = "HTTP/1.0 404 NOT FOUND\r\nContent-Length: 0\r\nContent-Type: text/html\r\n\r\n";
        send(socket, responce.c_str(), responce.size(), 0);
        return false;
    }

    std::string responce = "HTTP/1.0 200 OK\r\n\r\n";
    send(socket, responce.c_str(), responce.size(), 0);
    char readBuf[2048];

    while (int cntRead = read(fd, readBuf, 2048))
        send(socket, readBuf, cntRead, 0);

    close(fd);
    return true;
}

void Handler::_http_handle(int fd)
{
    char buf[2048];
    int recv_sz = recv(fd, buf, 2048, MSG_NOSIGNAL);

    if (recv_sz == -1 || recv_sz == 0)
    {
        shutdown(fd, SHUT_RDWR);
        close(fd);
        return;
    }
    std::string request_str(buf, (unsigned long)recv_sz);

    if (check_get(request_str))
        do_get(fd, request_str);
    else
    {
        std::string responce = "HTTP/1.0 400 Bad Request\r\nContent-Length: 0\r\nContent-Type: text/html\r\n\r\n";
        send(fd, responce.c_str(), responce.size(), 0);
    }

    shutdown(fd, SHUT_RDWR);
    close(fd);
}
//=======================================================================================

