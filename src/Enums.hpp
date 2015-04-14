/*
 *  This file is part of the mhwd - Manjaro Hardware Detection project
 *  
 *  mhwd - Manjaro Hardware Detection
 *  Roland Singer <roland@manjaro.org>
 *  Łukasz Matysiak <december0123@gmail.com>
 *  Filipe Marques <eagle.software3@gmail.com>
 *
 *  Copyright (C) 2007 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ENUMS_HPP_
#define ENUMS_HPP_

namespace MHWD
{

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
