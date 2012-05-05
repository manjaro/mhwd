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

#ifndef MHWD_H
#define MHWD_H

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>


namespace mhwd {
    //#############//
    //### Enums ###//
    //#############//
    enum TYPE { TYPE_PCI, TYPE_USB };


    //####################//
    //### Data Structs ###//
    //####################//
    struct HardwareIDs {
        std::vector<std::string> classIDs, vendorIDs, deviceIDs;
    };

    struct Config {
        TYPE type;
        std::string basePath, configPath, name, info, version;
        bool freedriver;
        int priority;
        std::vector<HardwareIDs> hwdIDs;
    };

    struct Device {
        TYPE type;
        std::string className, deviceName, vendorName, classID, deviceID, vendorID;
        std::vector<mhwd::Config> availableConfigs, installedConfigs;
    };

    struct Data {
        std::vector<mhwd::Device> USBDevices, PCIDevices;
        std::vector<mhwd::Config> installedUSBConfigs, installedPCIConfigs;
    };


    //#################//
    //### Functions ###//
    //#################//
    void fillData(mhwd::Data *data);
    void cleanupData(mhwd::Data *data);

    void printDeviceDetails(mhwd::TYPE type, FILE *f = stdout);

    /*bool installConfig(mhwd::Config *config);*/
}


#else


//!!!!!!!!!!!!!!!!!!!! TODO


#endif // MHWD_H
