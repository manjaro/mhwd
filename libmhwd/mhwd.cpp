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

#include "mhwd.h"


vector<mhwd::Device*> mhwd::getDevices(HW hw) {
    vector<Device*> devices;
    hd_data_t *hd_data;
    hd_t *hd;

    hd_data = (hd_data_t*)calloc(1, sizeof *hd_data);

    hd = hd_list(hd_data, (hw_item)hw, 1, NULL);

    for(; hd; hd = hd->next) {
        Device::TYPE type = Device::TYPE_PCI;
        if (hw == HW_USB)
            type = Device::TYPE_USB;

        devices.push_back(new Device(hd, type));
    }

    hd_free_hd_list(hd);
    hd_free_hd_data(hd_data);
    free(hd_data);

    return devices;
}



void mhwd::printDetails(HW hw) {
    hd_data_t *hd_data;
    hd_t *hd;

    hd_data = (hd_data_t*)calloc(1, sizeof *hd_data);

    hd = hd_list(hd_data, (hw_item)hw, 1, NULL);

    for(; hd; hd = hd->next) {
        hd_dump_entry(hd_data, hd, stdout);
    }

    hd_free_hd_list(hd);
    hd_free_hd_data(hd_data);
    free(hd_data);
}
