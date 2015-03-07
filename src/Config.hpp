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

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <string>
#include <vector>

#include "Enums.hpp"
#include "vita/string.hpp"

struct Config
{
    Config(std::string configPath, std::string type);
    bool readConfigFile(std::string configPath);

    struct HardwareID
    {
        std::vector<std::string> classIDs;
        std::vector<std::string> vendorIDs;
        std::vector<std::string> deviceIDs;
        std::vector<std::string> blacklistedClassIDs;
        std::vector<std::string> blacklistedVendorIDs;
        std::vector<std::string> blacklistedDeviceIDs;
    };

    std::string type_;
    std::string basePath_;
    std::string configPath_;
    std::string name_;
    std::string info_;
    std::string version_;
    bool freedriver_ = true;
    int priority_ = 0;
    std::vector<HardwareID> hwdIDs_;
    std::vector<std::string> conflicts_;
    std::vector<std::string> dependencies_;

private:
    std::vector<std::string> splitValue(Vita::string str, Vita::string onlyEnding = "");
    Vita::string getRightConfigPath(Vita::string str, Vita::string baseConfigPath);
};

#endif /* CONFIG_HPP_ */
