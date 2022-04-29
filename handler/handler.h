/*! \file handler.h
 * \brief Handler class interface.
 *
 * Class description.
 *
 * \authors rene
 * \date April 2022
 */

//=======================================================================================

#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "web_params.h"

//=======================================================================================
/*! \class Handler
 * \brief Some briefing
 */
class Handler
{
public:

    //! \brief default constructor.    
    Handler(const WebParams& web_params);

    ~Handler();

    //! \brief default destructor.

    //-----------------------------------------------------------------------------------

    void run();

    //-----------------------------------------------------------------------------------
    static void _http_handle(int fd);
private:

    int _master_socket_fd{-1};

    std::string _dir;
};
//=======================================================================================
