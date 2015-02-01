/*
 *  mhwd - Manjaro Hardware Detection
 *  Roland Singer <roland@manjaro.org>
 *  Łukasz Matysiak <december0123@gmail.com>
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

#ifndef DEVICE_HPP_
#define DEVICE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "Config.hpp"

struct Device
{
    std::string type_;
    std::string className_;
    std::string deviceName_;
    std::string vendorName_;
    std::string classID_;
    std::string deviceID_;
    std::string vendorID_;
    std::string sysfsBusID_;
    std::string sysfsID_;
    std::vector<std::shared_ptr<Config>> availableConfigs_;
    std::vector<std::shared_ptr<Config>> installedConfigs_;
};

#endif /* DEVICE_HPP_ */
