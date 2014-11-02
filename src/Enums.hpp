/*
 * Enums.hpp
 *
 *  Created on: 28 sie 2014
 *      Author: dec
 */

#ifndef ENUMS_HPP_
#define ENUMS_HPP_

namespace MHWD
{

struct Arguments
{
    bool SHOWPCI = false;
    bool SHOWUSB = false;
    bool INSTALL = false;
    bool REMOVE = false;
    bool DETAIL = false;
    bool FORCE = false;
    bool LISTALL = false;
    bool LISTINSTALLED = false;
    bool LISTAVAILABLE = false;
    bool LISTHARDWARE = false;
    bool CUSTOMINSTALL = false;
    bool AUTOCONFIGURE = false;
};

enum class STATUS
{
    SUCCESS,
    ERROR_CONFLICTS,
    ERROR_REQUIREMENTS,
    ERROR_NOT_INSTALLED,
    ERROR_ALREADY_INSTALLED,
    ERROR_NO_MATCH_LOCAL_CONFIG,
    ERROR_SCRIPT_FAILED,
    ERROR_SET_DATABASE
};

enum class MESSAGETYPE
{
    CONSOLE_OUTPUT,
    INSTALLDEPENDENCY_START,
    INSTALLDEPENDENCY_END,
    INSTALL_START,
    INSTALL_END,
    REMOVE_START,
    REMOVE_END
};

enum class TRANSACTIONTYPE
{
    INSTALL, REMOVE
};

}  // namespace MHWD

#endif /* ENUMS_HPP_ */
