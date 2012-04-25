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

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <hd.h>
#include "device.h"

using namespace std;


namespace mhwd {
    enum HW { HW_PCI = hw_pci, HW_USB = hw_usb };

    vector<Device*> getDevices(HW hw);
    void printDetails(HW hw);
}

#else

namespace mhwd {
    extern vector<Device*> getDevices(hw_item hw);
    extern void printDetails(hw_item hw);
}

#endif // LIBMHWD_H
