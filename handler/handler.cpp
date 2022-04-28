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

#define MAX_EVENTS 32

using namespace std;

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
    :_master_socket_fd(socket(AF_INET, SOCK_STREAM, 0))
{
    int opt{1};

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

    if (listen(_master_socket_fd, SOMAXCONN) < 0)
    {
        cerr << "Couldn't listen";
        exit(EXIT_FAILURE);
    }

}
//=======================================================================================
void Handler::run()
{
    int e_poll = epoll_create1(0);
    struct epoll_event event;
    event.data.fd = _master_socket_fd;
    event.events = EPOLLIN | EPOLLET;// access to read | edge trigger regime
    epoll_ctl(e_poll, EPOLL_CTL_ADD, _master_socket_fd, &event); //register events

    while (true)
    {
        struct epoll_event events[MAX_EVENTS];
        int n = epoll_wait(e_poll, events, MAX_EVENTS, -1); //-1 inf wait

        for (uint i = 0; i < n; i++)
            if (events[i].data.fd == _master_socket_fd)
            {
                int new_slave_socket = accept(_master_socket_fd, 0, 0);
                set_nonblock(new_slave_socket);
                struct epoll_event event;
                event.data.fd = new_slave_socket;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(e_poll, EPOLL_CTL_ADD, new_slave_socket, &event);
            } else
            {
                static char buf[1024];
                int recv_sz = recv(events[i].data.fd, buf, 1024, MSG_NOSIGNAL);
                if (recv_sz == 0 && errno != EAGAIN)
                {
                    shutdown(events[i].data.fd, SHUT_RDWR);
                    close(events[i].data.fd);
                } else if (recv_sz > 0)
                    send(events[i].data.fd, buf, recv_sz, MSG_NOSIGNAL);
            }

    }
}
//=======================================================================================

