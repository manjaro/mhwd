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

    ClassID = from_Hex(hd->base_class.id, 2) + from_Hex(hd->sub_class.id, 2);
    VendorID = from_Hex(hd->vendor.id, 4);
    DeviceID = from_Hex(hd->device.id, 4);
    ClassName = from_CharArray(hd->base_class.name);
    VendorName = from_CharArray(hd->vendor.name);
    DeviceName = from_CharArray(hd->device.name);
}




// Private



string mhwd::Device::from_Hex(uint16_t hexnum, int fill) {
    stringstream stream;
    stream << hex << setfill('0') << setw(fill) << hexnum;
    return stream.str();
}



string mhwd::Device::from_CharArray(char* c) {
    if (c == NULL)
        return "";

    return string(c);
}
