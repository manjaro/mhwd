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



mhwd::HWD::HWD()
{
    update();
}



void mhwd::HWD::update() {
    PCIDevices.clear();
    USBDevices.clear();
    InstalledUSBConfigs.clear();
    InstalledPCIConfigs.clear();

    setInstalledConfigs(&InstalledUSBConfigs, MHWD_USB_DATABASE_DIR);
    setInstalledConfigs(&InstalledPCIConfigs, MHWD_PCI_DATABASE_DIR);

    setDevices(&PCIDevices, Device::TYPE_PCI);
    setDevices(&USBDevices, Device::TYPE_USB);
}



void mhwd::HWD::printUSBDetails() {
    printDetails(hw_usb);
}



void mhwd::HWD::printPCIDetails() {
    printDetails(hw_pci);
}



bool mhwd::HWD::installConfig(mhwd::Config& config) {

    return true;
}



//###
//### Private
//###


void mhwd::HWD::setDevices(std::vector<mhwd::Device>* devices, Device::TYPE type) {
    hd_data_t *hd_data;
    hd_t *hd;
    hw_item hw;
    std::string configDir;
    std::vector<mhwd::Config>* configs;

    if (type == Device::TYPE_USB) {
        hw = hw_usb;
        configDir = MHWD_USB_CONFIG_DIR;
        configs = &InstalledUSBConfigs;
    }
    else {
        hw = hw_pci;
        configDir = MHWD_PCI_CONFIG_DIR;
        configs = &InstalledPCIConfigs;
    }


    hd_data = (hd_data_t*)calloc(1, sizeof *hd_data);
    hd = hd_list(hd_data, hw, 1, NULL);

    for(; hd; hd = hd->next) {
        Device device(hd, type);
        devices->push_back(device);
    }

    setMatchingConfigs(devices, configDir, false);
    setMatchingConfigs(devices, configs, true);

    hd_free_hd_list(hd);
    hd_free_hd_data(hd_data);
    free(hd_data);
}



void mhwd::HWD::setInstalledConfigs(std::vector<mhwd::Config>* configs, std::string databaseDir) {
    struct dirent *dir;
    DIR *d = opendir(databaseDir.c_str());

    if (!d)
        return;

    while ((dir = readdir(d)) != NULL)
    {
        Vita::string filename = Vita::string(dir->d_name);
        Vita::string filepath = databaseDir + "/" + filename;

        if(filename == "." || filename == ".." || filename == "")
            continue;

        struct stat filestatus;
        lstat(filepath.c_str(), &filestatus);

        if (S_ISREG(filestatus.st_mode) && filename == MHWD_CONFIG_NAME) {
            mhwd::Config config(databaseDir);

            if (config.isValid())
                configs->push_back(config);
            // TODO: Show error message!
        }
        else if (S_ISDIR(filestatus.st_mode)) {
            setInstalledConfigs(configs, filepath);
        }
    }

    closedir(d);
}



void mhwd::HWD::setMatchingConfigs(std::vector<mhwd::Device>* devices, const std::string configDir, bool setAsInstalled) {
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

        if (S_ISREG(filestatus.st_mode) && filename == MHWD_CONFIG_NAME) {
            mhwd::Config config(configDir);
            setMatchingConfig(config, devices, setAsInstalled);
        }
        else if (S_ISDIR(filestatus.st_mode)) {
            setMatchingConfigs(devices, filepath, setAsInstalled);
        }
    }

    closedir(d);
}



void mhwd::HWD::setMatchingConfigs(std::vector<mhwd::Device>* devices, std::vector<mhwd::Config>* configs, bool setAsInstalled) {
    for (std::vector<mhwd::Config>::iterator iterator = configs->begin(); iterator != configs->end(); iterator++) {
        setMatchingConfig((*iterator), devices, setAsInstalled);
    }
}



void mhwd::HWD::setMatchingConfig(mhwd::Config& config, std::vector<mhwd::Device>* devices, bool setAsInstalled) {
    // Check if config is valid
    if (!config.isValid())
        return;
    // TODO: print warning!

    std::vector<mhwd::Device*> foundDevices;
    std::vector<mhwd::Config::IDsGroup> IDsGroups = config.getIDsGroups();

    for (std::vector<mhwd::Config::IDsGroup>::const_iterator i_idsgroup = IDsGroups.begin(); i_idsgroup != IDsGroups.end(); i_idsgroup++) {
        bool foundDevice = false;

        // Check all devices
        for (std::vector<mhwd::Device>::iterator i_device = devices->begin(); i_device != devices->end(); i_device++) {
            bool found = false;

            // Check class ids
            for (std::vector<std::string>::const_iterator iterator = (*i_idsgroup).classIDs.begin(); iterator != (*i_idsgroup).classIDs.end(); iterator++) {
                if (*iterator == "*" || *iterator == (*i_device).ClassID) {
                    found = true;
                    break;
                }
            }

            if (!found)
                continue;

            // Check vendor ids
            found = false;

            for (std::vector<std::string>::const_iterator iterator = (*i_idsgroup).vendorIDs.begin(); iterator != (*i_idsgroup).vendorIDs.end(); iterator++) {
                if (*iterator == "*" || *iterator == (*i_device).VendorID) {
                    found = true;
                    break;
                }
            }

            if (!found)
                continue;

            // Check device ids
            found = false;

            for (std::vector<std::string>::const_iterator iterator = (*i_idsgroup).deviceIDs.begin(); iterator != (*i_idsgroup).deviceIDs.end(); iterator++) {
                if (*iterator == "*" || *iterator == (*i_device).DeviceID) {
                    found = true;
                    break;
                }
            }

            if (!found)
                continue;

            foundDevices.push_back(&(*i_device));
            foundDevice = true;
        }

        if (!foundDevice)
            return;
    }


    // Set config to all matching devices
    for (std::vector<mhwd::Device*>::iterator iterator = foundDevices.begin(); iterator != foundDevices.end(); iterator++) {
        if (setAsInstalled)
            (*iterator)->addInstalledConfig(config);
        else
            (*iterator)->addConfig(config);
    }
}



void mhwd::HWD::printDetails(hw_item hw) {
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
