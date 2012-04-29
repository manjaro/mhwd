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

using namespace std;


namespace mhwd {
    class hwd
    {
    public:
        static vector<Device*> getUSBDevices();
        static vector<Device*> getPCIDevices();
        static void printUSBDetails();
        static void printPCIDetails();
        static void update();

    private:
        static vector<Device*> USBDevices, PCIDevices;

        static vector<mhwd::Device*> getDevices(Device::TYPE type);
        static void setMatchingConfigs(vector<mhwd::Device*>* devices, const string configDir);
        static void setMatchingConfig(vector<mhwd::Device*>* devices, const string configPath);
        static void printDetails(hw_item hw);

    };
}

#endif // HWD_H
