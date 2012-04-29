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

#include "hwd.h"


// Static variables


vector<mhwd::Device*> mhwd::hwd::USBDevices, mhwd::hwd::PCIDevices;



// Static methods



vector<mhwd::Device*> mhwd::hwd::getUSBDevices() {
    if (USBDevices.empty())
        update();

    return USBDevices;
}



vector<mhwd::Device*> mhwd::hwd::getPCIDevices() {
    if (PCIDevices.empty())
        update();

    return PCIDevices;
}



void mhwd::hwd::update() {
    PCIDevices.clear();
    USBDevices.clear();

    PCIDevices = getDevices(Device::TYPE_PCI);
    USBDevices = getDevices(Device::TYPE_USB);
}



void mhwd::hwd::printUSBDetails() {
    printDetails(hw_usb);
}



void mhwd::hwd::printPCIDetails() {
    printDetails(hw_pci);
}



// Private



vector<mhwd::Device*> mhwd::hwd::getDevices(Device::TYPE type) {
    vector<Device*> devices;
    hd_data_t *hd_data;
    hd_t *hd;
    hw_item hw;
    string configDir;

    if (type == Device::TYPE_USB) {
        hw = hw_usb;
        configDir = MHWD_USB_CONFIG_DIR;
    }
    else {
        hw = hw_pci;
        configDir = MHWD_PCI_CONFIG_DIR;
    }

    hd_data = (hd_data_t*)calloc(1, sizeof *hd_data);


    hd = hd_list(hd_data, hw, 1, NULL);

    for(; hd; hd = hd->next) {
        devices.push_back(new Device(hd, type));
    }

    setMatchingConfigs(&devices, configDir);

    hd_free_hd_list(hd);
    hd_free_hd_data(hd_data);
    free(hd_data);

    return devices;
}



void mhwd::hwd::setMatchingConfigs(vector<mhwd::Device*>* devices, const string configDir) {
    struct dirent *dir;
    DIR *d = opendir(configDir.c_str());

    if (!d)
        return;

    while ((dir = readdir(d)) != NULL)
    {
        Vita::string filename = Vita::string(dir->d_name);
        Vita::string filepath = configDir + "/" + filename;

        if(filename == "." || filename == ".." || filename == "")
            continue;

        struct stat filestatus;
        lstat(filepath.c_str(), &filestatus);

        if (S_ISREG(filestatus.st_mode) && filename.explode(".").back() == MHWD_CONFIG_END)
            setMatchingConfig(devices, filepath);
        else if (S_ISDIR(filestatus.st_mode))
            setMatchingConfigs(devices, filepath);
    }

    closedir(d);
}



void mhwd::hwd::setMatchingConfig(vector<mhwd::Device*>* devices, const string configPath) {
    mhwd::Config config(configPath);
    vector<mhwd::Device*> foundDevices;
    vector<mhwd::Config::IDsGroup> IDsGroups = config.getIDsGroups();

    for (vector<mhwd::Config::IDsGroup>::const_iterator i_idsgroup = IDsGroups.begin(); i_idsgroup != IDsGroups.end(); i_idsgroup++) {
        bool foundDevice = false;

        // Check all devices
        for (vector<mhwd::Device*>::iterator i_device = devices->begin(); i_device != devices->end(); i_device++) {
            bool found = false;

            // Check class ids
            for (vector<string>::const_iterator iterator = (*i_idsgroup).classIDs.begin(); iterator != (*i_idsgroup).classIDs.end(); iterator++) {
                if (*iterator == "*" || *iterator == (*i_device)->ClassID) {
                    found = true;
                    break;
                }
            }

            if (!found)
                continue;

            // Check vendor ids
            found = false;

            for (vector<string>::const_iterator iterator = (*i_idsgroup).vendorIDs.begin(); iterator != (*i_idsgroup).vendorIDs.end(); iterator++) {
                if (*iterator == "*" || *iterator == (*i_device)->VendorID) {
                    found = true;
                    break;
                }
            }

            if (!found)
                continue;

            // Check device ids
            found = false;

            for (vector<string>::const_iterator iterator = (*i_idsgroup).deviceIDs.begin(); iterator != (*i_idsgroup).deviceIDs.end(); iterator++) {
                if (*iterator == "*" || *iterator == (*i_device)->DeviceID) {
                    found = true;
                    break;
                }
            }

            if (!found)
                continue;

            foundDevices.push_back((*i_device));
            foundDevice = true;
        }

        if (!foundDevice)
            return;
    }


    // Set config to all matching devices
    for (vector<mhwd::Device*>::iterator iterator = foundDevices.begin(); iterator != foundDevices.end(); iterator++) {
        (*iterator)->addConfig(config);
    }
}



void mhwd::hwd::printDetails(hw_item hw) {
    hd_data_t *hd_data;
    hd_t *hd;

    hd_data = (hd_data_t*)calloc(1, sizeof *hd_data);

    hd = hd_list(hd_data, hw, 1, NULL);

    for(; hd; hd = hd->next) {
        hd_dump_entry(hd_data, hd, stdout);
    }

    hd_free_hd_list(hd);
    hd_free_hd_data(hd_data);
    free(hd_data);
}
