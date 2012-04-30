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

#ifndef DEVICE_H
#define DEVICE_H

#include <stdlib.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <hd.h>
#include "string.hpp"
#include "config.h"


namespace mhwd {
    class Device
    {
    public:
        friend class HWD;

        enum TYPE { TYPE_PCI, TYPE_USB };

        Device(hd_t *hd, TYPE type);

        TYPE getType() { return type; }
        std::string getClassName() { return ClassName; }
        std::string getDeviceName() { return DeviceName; }
        std::string getVendorName() { return VendorName; }
        std::string getClassID() { return ClassID; }
        std::string getDeviceID() { return DeviceID; }
        std::string getVendorID() { return VendorID; }
        std::vector<mhwd::Config> getConfigs() { return configs; }
        std::vector<mhwd::Config> getInstalledConfigs() { return installedConfigs; }

    private:
        std::string ClassName, DeviceName, VendorName, ClassID, DeviceID, VendorID;
        TYPE type;
        std::vector<mhwd::Config> configs, installedConfigs;

        void addConfig(Config& config);
        void addInstalledConfig(Config& config);
        Vita::string from_Hex(uint16_t hexnum, int fill = 4);
        Vita::string from_CharArray(char* c);
    };
}

#endif // DEVICE_H
