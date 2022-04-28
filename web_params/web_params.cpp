/*! \file web_params.cpp
 * \brief WebParams class implementation.
 *
 * \authors rene
 * \date April 2022
 */

//=======================================================================================
#include <unistd.h>

#include "web_params.h"

using namespace std;

void WebParams::parse(int argc, char** argv)
{
    int c{0};
    while ( (c = getopt(argc, argv, "h:p:d:")) != -1)
        switch (c)
        {
            case 'h':
                _ip = optarg;
                break;
            case 'p':
                _port = atoi(optarg);
                break;
            case 'd':
                _dir = optarg;
                break;
            default:
                cout << "wrong arg:" << optarg << endl;
                break;
        }


    _log_params();
}
//=======================================================================================
void WebParams::_log_params()
{
    cout << "Read params:" << endl;
    cout << "ip: " << _ip << endl;
    cout << "port: " << _port << endl;
    cout << "directory: " << _dir << endl;
}
//=======================================================================================
int WebParams::get_port() const
{
    return _port;
}

std::string WebParams::get_ip() const
{
    return _ip;
}

std::string WebParams::get_dir() const
{
    return _dir;
}
//=======================================================================================