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

using namespace std;


namespace mhwd {
    class Device
    {
    public:
        friend class hwd;

        enum TYPE { TYPE_PCI, TYPE_USB };

        Device(hd_t *hd, TYPE type);

        TYPE getType() { return type; }
        string getClassName() { return ClassName; }
        string getDeviceName() { return DeviceName; }
        string getVendorName() { return VendorName; }
        string getClassID() { return ClassID; }
        string getDeviceID() { return DeviceID; }
        string getVendorID() { return VendorID; }
        vector<mhwd::Config> getConfigs() { return configs; }

    private:
        string ClassName, DeviceName, VendorName, ClassID, DeviceID, VendorID;
        TYPE type;
        vector<mhwd::Config> configs;

        void addConfig(Config& config);
        Vita::string from_Hex(uint16_t hexnum, int fill = 4);
        Vita::string from_CharArray(char* c);
    };
}

#endif // DEVICE_H
