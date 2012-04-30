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

#include "device.h"



mhwd::Device::Device(hd_t *hd, TYPE type) {
    Device::type = type;

    ClassID = from_Hex(hd->base_class.id, 2) + from_Hex(hd->sub_class.id, 2).toLower();
    VendorID = from_Hex(hd->vendor.id, 4).toLower();
    DeviceID = from_Hex(hd->device.id, 4).toLower();
    ClassName = from_CharArray(hd->base_class.name);
    VendorName = from_CharArray(hd->vendor.name);
    DeviceName = from_CharArray(hd->device.name);
}




// Private



void mhwd::Device::addConfig(mhwd::Config& config) {
    for (std::vector<mhwd::Config>::const_iterator iterator = configs.begin(); iterator != configs.end(); iterator++) {
        if (config == *iterator)
            return;
    }

    for (std::vector<mhwd::Config>::iterator iterator = configs.begin(); iterator != configs.end(); iterator++) {
        if (config.getPriority() > (*iterator).getPriority()) {
            configs.insert(iterator, config);
            return;
        }
    }

    configs.push_back(config);
}



void mhwd::Device::addInstalledConfig(mhwd::Config& config) {
    for (std::vector<mhwd::Config>::const_iterator iterator = installedConfigs.begin(); iterator != installedConfigs.end(); iterator++) {
        if (config == *iterator)
            return;
    }

    for (std::vector<mhwd::Config>::iterator iterator = installedConfigs.begin(); iterator != installedConfigs.end(); iterator++) {
        if (config.getPriority() > (*iterator).getPriority()) {
            installedConfigs.insert(iterator, config);
            return;
        }
    }

    installedConfigs.push_back(config);
}



Vita::string mhwd::Device::from_Hex(uint16_t hexnum, int fill) {
    std::stringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(fill) << hexnum;
    return stream.str();
}



Vita::string mhwd::Device::from_CharArray(char* c) {
    if (c == NULL)
        return "";

    return std::string(c);
}

