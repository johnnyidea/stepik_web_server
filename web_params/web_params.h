/*! \file web_params.h
 * \brief WebParams class interface.
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

//=======================================================================================
/*! \class WebParams
 * \brief Some briefing
 */
class WebParams
{
public:

    //! \brief default constructor.    
    WebParams(){};

    //! \brief default destructor.

    //-----------------------------------------------------------------------------------

    void parse(int argc, char** argv);

    //-----------------------------------------------------------------------------------

    int get_port() const;
    std::string get_ip() const;
    std::string get_dir() const;

private:

    void _log_params();

    int _port;

    std::string _ip;

    std::string _dir;
};
//=======================================================================================
