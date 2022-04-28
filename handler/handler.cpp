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
    event.events = EPOLLIN;// access to read | edge trigger regime
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
                event.events = EPOLLIN;
                epoll_ctl(e_poll, EPOLL_CTL_ADD, new_slave_socket, &event);
            } else
            {
                int fd = static_cast<int>(events[i].data.fd);
                epoll_ctl(e_poll, EPOLL_CTL_DEL, fd, 0);
                thread t([&fd, this]()
                {
                    std::stringstream ss;
                    static char sock_buf[1024];
                    int recv_sz = recv(fd, sock_buf, 1024, MSG_NOSIGNAL);

                    size_t size = 0;

                    if (recv_sz > 0)
                    {
//                        cout << sock_buf << endl;

                        Request request;
                        HttpRequestParser parser;

                        HttpRequestParser::ParseResult res = parser.parse(request, sock_buf, sock_buf + strlen(sock_buf));

                        FILE *file_in = NULL;
                        char buff[255] = {0};

                        std::string file_in_name = _dir;

//                        std::cout << request.inspect() << std::endl;
                        file_in_name += request.uri;
                        file_in = fopen(file_in_name.c_str(), "r");
                        if (file_in)
                        {
                            std::string tmp;
                            fgets(buff, 255, file_in);

                            tmp += buff;

                            fclose(file_in);

                            ss << "HTTP/1.0 200 OK";
                            ss << "\r\n";
                            ss << "Content-length: ";
                            ss << tmp.size();
                            ss << "\r\n";
                            ss << "Content-Type: text/html";
                            ss << "\r\n\r\n";
                            ss << tmp;

                            printf("ss = %s", ss.str().c_str());

                            size = ss.str().size();

                            strncpy(sock_buf, ss.str().c_str(), size);
                        } else {
                            ss << "HTTP/1.0 404 NOT FOUND";
                            ss << "\r\n";
                            ss << "Content-length: ";
                            ss << 0;
                            ss << "\r\n";
                            ss << "Content-Type: text/html";
                            ss << "\r\n\r\n";

                            printf("ss = %s", ss.str().c_str());

                            size = ss.str().size();

                            strncpy(sock_buf, ss.str().c_str(), size);
                        }

                        send(fd, sock_buf, size, MSG_NOSIGNAL);
                    }

                    if (recv_sz == 0 && errno != EAGAIN)
                    {
                        shutdown(fd, SHUT_RDWR);
                        close(fd);
                    }

                });

                t.detach();

//                shutdown(fd, SHUT_RDWR);
//                close(fd);
            }
    }
}

void Handler::_http_handle(int fd)
{
    static char buf[1024];
    int recv_sz = recv(fd, buf, 1024, MSG_NOSIGNAL);

    if (recv_sz == 0 && errno != EAGAIN)
    {
        shutdown(fd, SHUT_RDWR);
        close(fd);
    }
    else if (recv_sz > 0)
    {
        Request request;
        HttpRequestParser parser;

        HttpRequestParser::ParseResult res = parser.parse(request, buf, buf + strlen(buf));

        if (res == HttpRequestParser::ParsingCompleted)
        {
            std::cout << request.inspect() << std::endl;
        } else
        {
            std::cerr << "Parsing failed" << std::endl;
        }
    }
}
//=======================================================================================

