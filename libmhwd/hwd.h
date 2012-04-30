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

#ifndef HWD_H
#define HWD_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <hd.h>
#include "const.h"
#include "device.h"
#include "config.h"
#include "string.hpp"


namespace mhwd {
    class HWD
    {
    public:
        HWD();

        std::vector<mhwd::Device>* getUSBDevices() { return &USBDevices; }
        std::vector<mhwd::Device>* getPCIDevices() { return &PCIDevices; }

        void printUSBDetails();
        void printPCIDetails();
        void update();

        bool installConfig(mhwd::Config& config);

    private:
        std::vector<mhwd::Device> USBDevices, PCIDevices;
        std::vector<mhwd::Config> InstalledUSBConfigs, InstalledPCIConfigs;

        void setDevices(std::vector<mhwd::Device>* devices, Device::TYPE type);
        void setInstalledConfigs(std::vector<mhwd::Config>* configs, std::string databaseDir);

        void setMatchingConfigs(std::vector<mhwd::Device>* devices, const std::string configDir, bool setAsInstalled = false);
        void setMatchingConfigs(std::vector<mhwd::Device>* devices, std::vector<mhwd::Config>* configs, bool setAsInstalled = false);
        void setMatchingConfig(mhwd::Config& config, std::vector<mhwd::Device>* devices, bool setAsInstalled = false);

        void printDetails(hw_item hw);

    };
}

#endif // HWD_H
