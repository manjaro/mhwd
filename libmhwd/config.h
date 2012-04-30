/*
 *  mhwd - Manjaro Hardware Detection
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

#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "const.h"
#include "string.hpp"

namespace mhwd {
    class Config
    {
    public:
        Config(std::string basePath);
        bool operator==(const Config& compare);

        bool isValid() { return configValid; }
        std::string getName() { return name; }
        std::string getVersion() { return version; }
        std::string getInfo() { return info; }
        bool getIsFreeDriver() { return freedriver; }
        int getPriority() { return priority; }

        struct IDsGroup {
            std::vector<std::string> classIDs, vendorIDs, deviceIDs;
        };

        std::vector<IDsGroup> getIDsGroups() { return IDs; }

    private:
        std::string basePath, name, info, version;
        std::vector<IDsGroup> IDs;
        bool configValid, freedriver;
        int priority;

        bool readConfig(const Vita::string path);
        std::vector<std::string> getIDs(Vita::string str);
        inline void addNewIDsGroup();
        Vita::string getRightPath(Vita::string str);
    };
}

#endif // CONFIG_H
