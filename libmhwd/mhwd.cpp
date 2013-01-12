/*
 *  mhwd - Manjaro Hardware Detection
 *  Roland Singer <roland@manjaro.org>
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
#include "mhwd_p.h"




//#########################################//
//### Private - List & Vector Functions ###//
//#########################################//


template< typename T >
struct delete_ptr : public std::unary_function<bool,T> {
    bool operator()(T *ptr) const {
        if (ptr != NULL)
            delete ptr;

        ptr = NULL;
        return true;
    }
};





//########################//
//### Public Functions ###//
//########################//


void mhwd::initData(mhwd::Data *data) {
    data->environment.PMCachePath = MHWD_PM_CACHE_DIR;
    data->environment.PMConfigPath = MHWD_PM_CONFIG;
    data->environment.PMRootPath = MHWD_PM_ROOT;
    data->environment.syncPackageManagerDatabase = true;
    data->environment.messageFunc = NULL;
}



void mhwd::fillData(mhwd::Data *data) {
    freeData(data);

    fillDevices(data, mhwd::TYPE_PCI);
    fillDevices(data, mhwd::TYPE_USB);

    updateConfigData(data);
}



void mhwd::freeData(mhwd::Data *data) {
    std::for_each(data->PCIDevices.begin(), data->PCIDevices.end(), delete_ptr<mhwd::Device>());
    std::for_each(data->USBDevices.begin(), data->USBDevices.end(), delete_ptr<mhwd::Device>());
    std::for_each(data->installedPCIConfigs.begin(), data->installedPCIConfigs.end(), delete_ptr<mhwd::Config>());
    std::for_each(data->installedUSBConfigs.begin(), data->installedUSBConfigs.end(), delete_ptr<mhwd::Config>());
    std::for_each(data->allPCIConfigs.begin(), data->allPCIConfigs.end(), delete_ptr<mhwd::Config>());
    std::for_each(data->allUSBConfigs.begin(), data->allUSBConfigs.end(), delete_ptr<mhwd::Config>());
    std::for_each(data->invalidConfigs.begin(), data->invalidConfigs.end(), delete_ptr<mhwd::Config>());

    data->PCIDevices.clear();
    data->USBDevices.clear();
    data->installedPCIConfigs.clear();
    data->installedUSBConfigs.clear();
    data->allUSBConfigs.clear();
    data->allPCIConfigs.clear();
    data->invalidConfigs.clear();
}



std::string mhwd::checkEnvironment() {
    std::string retValue = "";

    // Check if required directories exists. Otherwise return missing directory...
    if (!checkExist(MHWD_USB_CONFIG_DIR))
        retValue = MHWD_USB_CONFIG_DIR;
    if (!checkExist(MHWD_PCI_CONFIG_DIR))
        retValue = MHWD_PCI_CONFIG_DIR;
    if (!checkExist(MHWD_USB_DATABASE_DIR))
        retValue = MHWD_USB_DATABASE_DIR;
    if (!checkExist(MHWD_PCI_DATABASE_DIR))
        retValue = MHWD_PCI_DATABASE_DIR;

    return retValue;
}



void mhwd::updateConfigData(mhwd::Data *data) {
    // Clear config vectors in each device element
    for (std::vector<mhwd::Device*>::iterator iterator = data->PCIDevices.begin(); iterator != data->PCIDevices.end(); iterator++) {
        (*iterator)->availableConfigs.clear();
    }

    for (std::vector<mhwd::Device*>::iterator iterator = data->USBDevices.begin(); iterator != data->USBDevices.end(); iterator++) {
        (*iterator)->availableConfigs.clear();
    }


    // Clear installed config vectors
    std::for_each(data->allPCIConfigs.begin(), data->allPCIConfigs.end(), delete_ptr<mhwd::Config>());
    std::for_each(data->allUSBConfigs.begin(), data->allUSBConfigs.end(), delete_ptr<mhwd::Config>());

    data->allPCIConfigs.clear();
    data->allUSBConfigs.clear();


    // Refill data
    fillAllConfigs(data, mhwd::TYPE_PCI);
    fillAllConfigs(data, mhwd::TYPE_USB);

    setMatchingConfigs(&data->PCIDevices, &data->allPCIConfigs, false);
    setMatchingConfigs(&data->USBDevices, &data->allUSBConfigs, false);

    // Update also installed config data
    updateInstalledConfigData(data);
}



void mhwd::updateInstalledConfigData(mhwd::Data *data) {
    // Clear config vectors in each device element
    for (std::vector<mhwd::Device*>::iterator iterator = data->PCIDevices.begin(); iterator != data->PCIDevices.end(); iterator++) {
        (*iterator)->installedConfigs.clear();
    }

    for (std::vector<mhwd::Device*>::iterator iterator = data->USBDevices.begin(); iterator != data->USBDevices.end(); iterator++) {
        (*iterator)->installedConfigs.clear();
    }


    // Clear installed config vectors
    std::for_each(data->installedPCIConfigs.begin(), data->installedPCIConfigs.end(), delete_ptr<mhwd::Config>());
    std::for_each(data->installedUSBConfigs.begin(), data->installedUSBConfigs.end(), delete_ptr<mhwd::Config>());

    data->installedPCIConfigs.clear();
    data->installedUSBConfigs.clear();


    // Refill data
    fillInstalledConfigs(data, mhwd::TYPE_PCI);
    fillInstalledConfigs(data, mhwd::TYPE_USB);

    setMatchingConfigs(&data->PCIDevices, &data->installedPCIConfigs, true);
    setMatchingConfigs(&data->USBDevices, &data->installedUSBConfigs, true);
}



bool mhwd::fillConfig(mhwd::Config *config, std::string configPath, mhwd::TYPE type) {
    config->type = type;
    config->priority = 0;
    config->freedriver = true;
    config->basePath = configPath.substr(0, configPath.find_last_of('/'));
    config->configPath = configPath;

    // Add new HardwareIDs group to vector if vector is empty
    if (config->hwdIDs.empty()) {
        mhwd::Config::HardwareIDs hwdID;
        config->hwdIDs.push_back(hwdID);
    }

    return readConfigFile(config, config->configPath);
}



void mhwd::printDeviceDetails(mhwd::TYPE type, FILE *f) {
    hd_data_t *hd_data;
    hd_t *hd;
    hw_item hw;

    if (type == mhwd::TYPE_USB)
        hw = hw_usb;
    else
        hw = hw_pci;

    hd_data = (hd_data_t*)calloc(1, sizeof *hd_data);
    hd = hd_list(hd_data, hw, 1, NULL);

    for(; hd; hd = hd->next) {
        hd_dump_entry(hd_data, hd, f);
    }

    hd_free_hd_list(hd);
    hd_free_hd_data(hd_data);
    free(hd_data);
}



mhwd::Config* mhwd::getInstalledConfig(mhwd::Data *data, const std::string configName, const TYPE configType) {
    std::vector<mhwd::Config*>* installedConfigs;

    // Get the right configs
    if (configType == mhwd::TYPE_USB)
        installedConfigs = &data->installedUSBConfigs;
    else
        installedConfigs = &data->installedPCIConfigs;

    for (std::vector<mhwd::Config*>::iterator iterator = installedConfigs->begin(); iterator != installedConfigs->end(); iterator++) {
        if (configName == (*iterator)->name)
            return (*iterator);
    }

    return NULL;
}



mhwd::Config* mhwd::getDatabaseConfig(mhwd::Data *data, const std::string configName, const TYPE configType) {
    std::vector<mhwd::Config*>* allConfigs;

    // Get the right configs
    if (configType == mhwd::TYPE_USB)
        allConfigs = &data->allUSBConfigs;
    else
        allConfigs = &data->allPCIConfigs;

    for (std::vector<mhwd::Config*>::iterator iterator = allConfigs->begin(); iterator != allConfigs->end(); iterator++) {
        if (configName == (*iterator)->name)
            return (*iterator);
    }

    return NULL;
}



mhwd::Config* mhwd::getAvailableConfig(mhwd::Data *data, const std::string configName, const TYPE configType) {
    std::vector<mhwd::Device*> *devices;

    // Get the right devices
    if (configType == mhwd::TYPE_USB)
        devices = &data->USBDevices;
    else
        devices = &data->PCIDevices;


    for (std::vector<mhwd::Device*>::iterator dev_iter = devices->begin(); dev_iter != devices->end(); dev_iter++) {
        if ((*dev_iter)->availableConfigs.empty())
            continue;

        for (std::vector<mhwd::Config*>::iterator iterator = (*dev_iter)->availableConfigs.begin(); iterator != (*dev_iter)->availableConfigs.end(); iterator++) {
            if (configName == (*iterator)->name)
                return (*iterator);
        }
    }

    return NULL;
}



std::vector<mhwd::Config*> mhwd::getAllDependenciesToInstall(mhwd::Data *data, mhwd::Config *config) {
    std::vector<mhwd::Config*> depends;
    std::vector<mhwd::Config*>* installedConfigs;

    // Get the right configs
    if (config->type == mhwd::TYPE_USB)
        installedConfigs = &data->installedUSBConfigs;
    else
        installedConfigs = &data->installedPCIConfigs;


    // Get all depends
    _getAllDependenciesToInstall(data, config, installedConfigs, &depends);

    return depends;
}



std::vector<mhwd::Config*> mhwd::getAllLocalConflicts(mhwd::Data *data, mhwd::Config *config) {
    std::vector<mhwd::Config*> conflicts;
    std::vector<mhwd::Config*> depends = getAllDependenciesToInstall(data, config);
    std::vector<mhwd::Config*>* installedConfigs;

    // Get the right configs
    if (config->type == mhwd::TYPE_USB)
        installedConfigs = &data->installedUSBConfigs;
    else
        installedConfigs = &data->installedPCIConfigs;


    depends.push_back(config);

    for (std::vector<mhwd::Config*>::const_iterator depend = depends.begin(); depend != depends.end(); depend++) {
        for (std::vector<std::string>::const_iterator conflict = (*depend)->conflicts.begin(); conflict != (*depend)->conflicts.end(); conflict++) {
            for (std::vector<mhwd::Config*>::const_iterator iterator = installedConfigs->begin(); iterator != installedConfigs->end(); iterator++) {
                if ((*conflict) != (*iterator)->name)
                    continue;

                // Check if already in vector
                bool found = false;
                for (std::vector<mhwd::Config*>::const_iterator it = conflicts.begin(); it != conflicts.end(); it++) {
                    if ((*it)->name == (*conflict)) {
                        found = true;
                        break;
                    }
                }

                if (found)
                    continue;

                // Add to vector
                conflicts.push_back((*iterator));
                break;
            }
        }
    }

    return conflicts;
}



std::vector<mhwd::Config*> mhwd::getAllLocalRequirements(mhwd::Data *data, mhwd::Config *config) {
    std::vector<mhwd::Config*> requirements;
    std::vector<mhwd::Config*>* installedConfigs;

    // Get the right configs
    if (config->type == mhwd::TYPE_USB)
        installedConfigs = &data->installedUSBConfigs;
    else
        installedConfigs = &data->installedPCIConfigs;


    // Check if this config is required by another installed config
    for (std::vector<mhwd::Config*>::const_iterator iterator = installedConfigs->begin(); iterator != installedConfigs->end(); iterator++) {
        for (std::vector<std::string>::const_iterator depend = (*iterator)->dependencies.begin(); depend != (*iterator)->dependencies.end(); depend++) {
            if ((*depend) != config->name)
                continue;

            // Check if already in vector
            bool found = false;
            for (std::vector<mhwd::Config*>::const_iterator it = requirements.begin(); it != requirements.end(); it++) {
                if ((*it)->name == (*iterator)->name) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                requirements.push_back((*iterator));
                break;
            }
        }
    }

    return requirements;
}



mhwd::Transaction mhwd::createTransaction(mhwd::Data *data, mhwd::Config* config, mhwd::Transaction::TYPE type, bool allowReinstallation) {
    struct mhwd::Transaction transaction;
    transaction.type = type;
    transaction.allowReinstallation = allowReinstallation;
    transaction.config = config;

    // Fill transaction vectors
    transaction.requiredByConfigs = getAllLocalRequirements(data, config);
    transaction.dependencyConfigs = getAllDependenciesToInstall(data, config);
    transaction.conflictedConfigs = getAllLocalConflicts(data, config);

    return transaction;
}



mhwd::STATUS mhwd::performTransaction(mhwd::Data *data, mhwd::Transaction *transaction) {
    if (transaction->type == mhwd::Transaction::TYPE_INSTALL && !transaction->conflictedConfigs.empty())
        return STATUS_ERROR_CONFLICTS;
    else if (transaction->type == mhwd::Transaction::TYPE_REMOVE && !transaction->requiredByConfigs.empty())
        return STATUS_ERROR_REQUIREMENTS;


    // Check if already installed
    mhwd::Config *installedConfig = getInstalledConfig(data, transaction->config->name, transaction->config->type);
    mhwd::STATUS status = STATUS_SUCCESS;


    if (transaction->type == mhwd::Transaction::TYPE_REMOVE || (installedConfig != NULL && transaction->allowReinstallation)) {
        if (installedConfig == NULL)
                return STATUS_ERROR_NOT_INSTALLED;

        emitMessageFunc(data, mhwd::MESSAGETYPE_REMOVE_START, installedConfig->name);
        if ((status = uninstallConfig(data, installedConfig)) != STATUS_SUCCESS)
            return status;
        emitMessageFunc(data, mhwd::MESSAGETYPE_REMOVE_END, installedConfig->name);
    }

    if (transaction->type == mhwd::Transaction::TYPE_INSTALL) {
        // Check if already installed but not allowed to reinstall
        if (installedConfig != NULL && !transaction->allowReinstallation)
                return STATUS_ERROR_ALREADY_INSTALLED;

        // Install all dependencies first
        for (std::vector<mhwd::Config*>::const_iterator it = transaction->dependencyConfigs.end() - 1; it != transaction->dependencyConfigs.begin() - 1; --it) {
            emitMessageFunc(data, mhwd::MESSAGETYPE_INSTALLDEPENDENCY_START, (*it)->name);
            if ((status = installConfig(data, (*it))) != STATUS_SUCCESS)
                return status;
            emitMessageFunc(data, mhwd::MESSAGETYPE_INSTALLDEPENDENCY_END, (*it)->name);
        }

        emitMessageFunc(data, mhwd::MESSAGETYPE_INSTALL_START, transaction->config->name);
        if ((status = installConfig(data, transaction->config)) != STATUS_SUCCESS)
            return status;
        emitMessageFunc(data, mhwd::MESSAGETYPE_INSTALL_END, transaction->config->name);
    }

    return status;
}







//#########################//
//### Private - Devices ###//
//#########################//



void mhwd::fillDevices(mhwd::Data *data, mhwd::TYPE type) {
    hd_data_t *hd_data;
    hd_t *hd;
    hw_item hw;
    std::vector<mhwd::Device*>* devices;

    if (type == mhwd::TYPE_USB) {
        hw = hw_usb;
        devices = &data->USBDevices;
    }
    else {
        hw = hw_pci;
        devices = &data->PCIDevices;
    }


    // Get the hardware devices
    hd_data = (hd_data_t*)calloc(1, sizeof *hd_data);
    hd = hd_list(hd_data, hw, 1, NULL);

    for(; hd; hd = hd->next) {
        struct mhwd::Device *device = new mhwd::Device();
        device->type = type;
        device->classID = from_Hex(hd->base_class.id, 2) + from_Hex(hd->sub_class.id, 2).toLower();
        device->vendorID = from_Hex(hd->vendor.id, 4).toLower();
        device->deviceID = from_Hex(hd->device.id, 4).toLower();
        device->className = from_CharArray(hd->base_class.name);
        device->vendorName = from_CharArray(hd->vendor.name);
        device->deviceName = from_CharArray(hd->device.name);
        device->sysfsBusID = from_CharArray(hd->sysfs_bus_id);
        device->sysfsID = from_CharArray(hd->sysfs_id);

        devices->push_back(device);
    }

    hd_free_hd_list(hd);
    hd_free_hd_data(hd_data);
    free(hd_data);
}



Vita::string mhwd::from_Hex(uint16_t hexnum, int fill) {
    std::stringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(fill) << hexnum;
    return stream.str();
}



int mhwd::hexToInt(std::string hex) {
    int i;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> i;

    return i;
}



Vita::string mhwd::from_CharArray(char* c) {
    if (c == NULL)
        return "";

    return Vita::string(c);
}





//#########################//
//### Private - Configs ###//
//#########################//


void mhwd::fillInstalledConfigs(mhwd::Data *data, mhwd::TYPE type) {
    std::vector<std::string> configPaths;
    std::vector<mhwd::Config*>* configs;

    if (type == mhwd::TYPE_USB) {
        configs = &data->installedUSBConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_USB_DATABASE_DIR, MHWD_CONFIG_NAME);
    }
    else {
        configs = &data->installedPCIConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_PCI_DATABASE_DIR, MHWD_CONFIG_NAME);
    }


    for (std::vector<std::string>::const_iterator iterator = configPaths.begin(); iterator != configPaths.end(); iterator++) {
        struct mhwd::Config *config = new mhwd::Config();

        if (fillConfig(config, (*iterator), type))
            configs->push_back(config);
        else
            data->invalidConfigs.push_back(config);
    }
}



void mhwd::fillAllConfigs(mhwd::Data *data, mhwd::TYPE type) {
    std::vector<std::string> configPaths;
    std::vector<mhwd::Config*>* configs;

    if (type == mhwd::TYPE_USB) {
        configs = &data->allUSBConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_USB_CONFIG_DIR, MHWD_CONFIG_NAME);
    }
    else {
        configs = &data->allPCIConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_PCI_CONFIG_DIR, MHWD_CONFIG_NAME);
    }


    for (std::vector<std::string>::const_iterator iterator = configPaths.begin(); iterator != configPaths.end(); iterator++) {
        struct mhwd::Config *config = new mhwd::Config();

        if (fillConfig(config, (*iterator), type))
            configs->push_back(config);
        else
            data->invalidConfigs.push_back(config);
    }
}



void mhwd::getAllDevicesOfConfig(mhwd::Data *data, mhwd::Config *config, std::vector<mhwd::Device*>* foundDevices) {
    std::vector<mhwd::Device*>* devices;

    if (config->type == mhwd::TYPE_USB)
        devices = &data->USBDevices;
    else
        devices = &data->PCIDevices;

    getAllDevicesOfConfig(devices, config, foundDevices);
}



void mhwd::getAllDevicesOfConfig(std::vector<mhwd::Device*>* devices, mhwd::Config *config, std::vector<mhwd::Device*>* foundDevices) {
    foundDevices->clear();

    for (std::vector<mhwd::Config::HardwareIDs>::const_iterator i_hwdIDs = config->hwdIDs.begin(); i_hwdIDs != config->hwdIDs.end(); i_hwdIDs++) {
        bool foundDevice = false;

        // Check all devices
        for (std::vector<mhwd::Device*>::iterator i_device = devices->begin(); i_device != devices->end(); i_device++) {
            bool found = false;

            // Check class ids
            for (std::vector<std::string>::const_iterator iterator = (*i_hwdIDs).classIDs.begin(); iterator != (*i_hwdIDs).classIDs.end(); iterator++) {
                if (*iterator == "*" || *iterator == (*i_device)->classID) {
                    found = true;
                    break;
                }
            }

            if (!found)
                continue;


            // Check blacklisted class ids
            found = false;

            for (std::vector<std::string>::const_iterator iterator = (*i_hwdIDs).blacklistedClassIDs.begin(); iterator != (*i_hwdIDs).blacklistedClassIDs.end(); iterator++) {
                if (*iterator == (*i_device)->classID) {
                    found = true;
                    break;
                }
            }

            if (found)
                continue;


            // Check vendor ids
            found = false;

            for (std::vector<std::string>::const_iterator iterator = (*i_hwdIDs).vendorIDs.begin(); iterator != (*i_hwdIDs).vendorIDs.end(); iterator++) {
                if (*iterator == "*" || *iterator == (*i_device)->vendorID) {
                    found = true;
                    break;
                }
            }

            if (!found)
                continue;


            // Check blacklisted vendor ids
            found = false;

            for (std::vector<std::string>::const_iterator iterator = (*i_hwdIDs).blacklistedVendorIDs.begin(); iterator != (*i_hwdIDs).blacklistedVendorIDs.end(); iterator++) {
                if (*iterator == (*i_device)->vendorID) {
                    found = true;
                    break;
                }
            }

            if (found)
                continue;


            // Check device ids
            found = false;

            for (std::vector<std::string>::const_iterator iterator = (*i_hwdIDs).deviceIDs.begin(); iterator != (*i_hwdIDs).deviceIDs.end(); iterator++) {
                if (*iterator == "*" || *iterator == (*i_device)->deviceID) {
                    found = true;
                    break;
                }
            }

            if (!found)
                continue;


            // Check blacklisted device ids
            found = false;

            for (std::vector<std::string>::const_iterator iterator = (*i_hwdIDs).blacklistedDeviceIDs.begin(); iterator != (*i_hwdIDs).blacklistedDeviceIDs.end(); iterator++) {
                if (*iterator == (*i_device)->deviceID) {
                    found = true;
                    break;
                }
            }

            if (found)
                continue;


            foundDevice = true;
            foundDevices->push_back((*i_device));
        }

        if (!foundDevice) {
            foundDevices->clear();
            return;
        }
    }
}



void mhwd::setMatchingConfigs(std::vector<mhwd::Device*>* devices, std::vector<mhwd::Config*>* configs, bool setAsInstalled) {
    for (std::vector<mhwd::Config*>::iterator iterator = configs->begin(); iterator != configs->end(); iterator++) {
        setMatchingConfig((*iterator), devices, setAsInstalled);
    }
}



void mhwd::setMatchingConfig(mhwd::Config* config, std::vector<mhwd::Device*>* devices, bool setAsInstalled) {
    std::vector<mhwd::Device*> foundDevices;

    getAllDevicesOfConfig(devices, config, &foundDevices);


    // Set config to all matching devices
    for (std::vector<mhwd::Device*>::iterator iterator = foundDevices.begin(); iterator != foundDevices.end(); iterator++) {
        if (setAsInstalled)
            addConfigSorted(&(*iterator)->installedConfigs, config);
        else
            addConfigSorted(&(*iterator)->availableConfigs, config);
    }
}



bool mhwd::readConfigFile(mhwd::Config *config, std::string configPath) {
    std::ifstream file(configPath.c_str(), std::ios::in);

    if (!file.is_open())
        return false;

    Vita::string line, key, value;
    std::vector<Vita::string> parts;

    while (!file.eof()) {
        getline(file, line);

        size_t pos = line.find_first_of('#');
        if (pos != std::string::npos)
            line.erase(pos);

        if (line.trim().empty())
            continue;

        parts = line.explode("=");
        key = parts.front().trim().toLower();
        value = parts.back().trim("\"").trim();


        // Read in extern file
        if (value.size() > 1 && value.substr(0, 1) == ">") {
            std::ifstream file(getRightConfigPath(value.substr(1), config->basePath).c_str(), std::ios::in);
            if (!file.is_open())
                return false;

            Vita::string line;
            value.clear();

            while (!file.eof()) {
                getline(file, line);

                size_t pos = line.find_first_of('#');
                if (pos != std::string::npos)
                    line.erase(pos);

                if (line.trim().empty())
                    continue;

                value += " " + line.trim();
            }

            file.close();

            value = value.trim();

            // remove all multiple spaces
            while (value.find("  ") != std::string::npos) {
                value = value.replace("  ", " ");
            }
        }


        if (key == "include") {
            readConfigFile(config, getRightConfigPath(value, config->basePath));
        }
        else if (key == "name") {
            config->name = value.toLower();
        }
        else if (key == "version") {
            config->version = value;
        }
        else if (key == "info") {
            config->info = value;
        }
        else if (key == "priority") {
            config->priority = value.convert<int>();
        }
        else if (key == "freedriver") {
            value = value.toLower();

            if (value == "false")
                config->freedriver = false;
            else if (value == "true")
                config->freedriver = true;
        }
        else if (key == "classids") {
            // Add new HardwareIDs group to vector if vector is not empty
            if (!config->hwdIDs.back().classIDs.empty()) {
                mhwd::Config::HardwareIDs hwdID;
                config->hwdIDs.push_back(hwdID);
            }

            config->hwdIDs.back().classIDs = splitValue(value);
        }
        else if (key == "vendorids") {
            // Add new HardwareIDs group to vector if vector is not empty
            if (!config->hwdIDs.back().vendorIDs.empty()) {
                mhwd::Config::HardwareIDs hwdID;
                config->hwdIDs.push_back(hwdID);
            }

            config->hwdIDs.back().vendorIDs = splitValue(value);
        }
        else if (key == "deviceids") {
            // Add new HardwareIDs group to vector if vector is not empty
            if (!config->hwdIDs.back().deviceIDs.empty()) {
                mhwd::Config::HardwareIDs hwdID;
                config->hwdIDs.push_back(hwdID);
            }

            config->hwdIDs.back().deviceIDs = splitValue(value);
        }
        else if (key == "blacklistedclassids") {
            config->hwdIDs.back().blacklistedClassIDs = splitValue(value);
        }
        else if (key == "blacklistedvendorids") {
            config->hwdIDs.back().blacklistedVendorIDs = splitValue(value);
        }
        else if (key == "blacklisteddeviceids") {
            config->hwdIDs.back().blacklistedDeviceIDs = splitValue(value);
        }
        else if (key == "mhwddepends") {
            config->dependencies = splitValue(value);
        }
        else if (key == "mhwdconflicts") {
            config->conflicts = splitValue(value);
        }
    }

    file.close();

    // Append * to all empty vectors
    for (std::vector<mhwd::Config::HardwareIDs>::iterator iterator = config->hwdIDs.begin(); iterator != config->hwdIDs.end(); iterator++) {
        if ((*iterator).classIDs.empty())
            (*iterator).classIDs.push_back("*");

        if ((*iterator).vendorIDs.empty())
            (*iterator).vendorIDs.push_back("*");

        if ((*iterator).deviceIDs.empty())
            (*iterator).deviceIDs.push_back("*");
    }

    if (config->name.empty())
        return false;

    return true;
}



void mhwd::addConfigSorted(std::vector<mhwd::Config*>* configs, mhwd::Config* config) {
    for (std::vector<mhwd::Config*>::const_iterator iterator = configs->begin(); iterator != configs->end(); iterator++) {
        if (config->name == (*iterator)->name)
            return;
    }


    for (std::vector<mhwd::Config*>::iterator iterator = configs->begin(); iterator != configs->end(); iterator++) {
        if (config->priority > (*iterator)->priority) {
            configs->insert(iterator, config);
            return;
        }
    }

    configs->push_back(config);
}



std::vector<std::string> mhwd::splitValue(Vita::string str, Vita::string onlyEnding) {
    std::vector<Vita::string> work = str.toLower().explode(" ");
    std::vector<std::string> final;

    for (std::vector<Vita::string>::const_iterator iterator = work.begin(); iterator != work.end(); iterator++) {
        if (*iterator != "" && onlyEnding.empty())
            final.push_back(*iterator);
        else if (*iterator != "" && Vita::string(*iterator).explode(".").back() == onlyEnding && (*iterator).size() > 5)
            final.push_back(Vita::string(*iterator).substr(0, (*iterator).size() - 5));
    }

    return final;
}



Vita::string mhwd::getRightConfigPath(Vita::string str, Vita::string baseConfigPath) {
    str = str.trim();

    if (str.size() <= 0 || str.substr(0, 1) == "/")
        return str;

    return baseConfigPath + "/" + str;
}



void mhwd::_getAllDependenciesToInstall(mhwd::Data *data, mhwd::Config *config, std::vector<mhwd::Config*>* installedConfigs, std::vector<mhwd::Config*> *depends) {
    for (std::vector<std::string>::const_iterator depend = config->dependencies.begin(); depend != config->dependencies.end(); depend++) {
        bool found = false;

        for (std::vector<mhwd::Config*>::const_iterator iterator = installedConfigs->begin(); iterator != installedConfigs->end(); iterator++) {
            if ((*depend) == (*iterator)->name) {
                found = true;
                break;
            }
        }

        if (found)
            continue;

        // Check if already in vector
        for (std::vector<mhwd::Config*>::const_iterator it = depends->begin(); it != depends->end(); it++) {
            if ((*it)->name == (*depend)) {
                found = true;
                break;
            }
        }

        if (found)
            continue;

        // Add to vector and check for further subdepends...
        mhwd::Config *dependconfig = mhwd::getDatabaseConfig(data, (*depend), config->type);
        if (dependconfig == NULL)
            continue;

        depends->push_back(dependconfig);
        _getAllDependenciesToInstall(data, dependconfig, installedConfigs, depends);
    }
}






//###############################################//
//### Private - Directory and File Operations ###//
//###############################################//



std::vector<std::string> mhwd::getRecursiveDirectoryFileList(const std::string directoryPath, std::string onlyFilename) {
    std::vector<std::string> list;
    struct dirent *dir;
    DIR *d = opendir(directoryPath.c_str());

    if (!d)
        return list;

    while ((dir = readdir(d)) != NULL)
    {
        std::string filename = std::string(dir->d_name);
        std::string filepath = directoryPath + "/" + filename;

        if(filename == "." || filename == ".." || filename == "")
            continue;

        struct stat filestatus;
        lstat(filepath.c_str(), &filestatus);

        if (S_ISREG(filestatus.st_mode) && (onlyFilename.empty() || onlyFilename == filename)) {
            list.push_back(filepath);
        }
        else if (S_ISDIR(filestatus.st_mode)) {
            std::vector<std::string> templist = getRecursiveDirectoryFileList(filepath, onlyFilename);

            for (std::vector<std::string>::const_iterator iterator = templist.begin(); iterator != templist.end(); iterator++) {
                list.push_back((*iterator));
            }
        }
    }

    closedir(d);

    return list;
}



bool mhwd::copyDirectory(const std::string source, const std::string destination) {
    struct stat filestatus;

    if (lstat(destination.c_str(), &filestatus) != 0) {
        if (!createDir(destination))
            return false;
    }
    else if (S_ISREG(filestatus.st_mode)) {
        return false;
    }
    else if (S_ISDIR(filestatus.st_mode)) {
        if (!removeDirectory(destination))
            return false;

        if (!createDir(destination))
            return false;
    }


    bool success = true;
    struct dirent *dir;
    DIR *d = opendir(source.c_str());

    if (!d)
        return false;

    while ((dir = readdir(d)) != NULL)
    {
        std::string filename = std::string(dir->d_name);
        std::string sourcepath = source + "/" + filename;
        std::string destinationpath = destination + "/" + filename;

        if(filename == "." || filename == ".." || filename == "")
            continue;

        lstat(sourcepath.c_str(), &filestatus);

        if (S_ISREG(filestatus.st_mode)) {
            if (!copyFile(sourcepath, destinationpath))
                success = false;
        }
        else if (S_ISDIR(filestatus.st_mode)) {
            if (!copyDirectory(sourcepath, destinationpath))
                success = false;
        }
    }

    closedir(d);
    return success;
}



bool mhwd::copyFile(const std::string source, const std::string destination, const mode_t mode) {
    int c;
    FILE *in,*out;

    in = fopen(source.c_str(), "r");
    out = fopen(destination.c_str(), "w");

    if(in==NULL || !in)
        return false;
    else if(out==NULL || !out) {
        fclose(in);
        return false;
    }

    while((c=getc(in))!=EOF)
        putc(c,out);

    fclose(in);
    fclose(out);

    // Set right permission
    mode_t process_mask = umask(0);
    chmod(destination.c_str(), mode);
    umask(process_mask);

    return true;
}



bool mhwd::removeDirectory(const std::string directory) {
    bool success = true;
    struct dirent *dir;
    DIR *d = opendir(directory.c_str());

    if (!d)
        return false;

    while ((dir = readdir(d)) != NULL)
    {
        std::string filename = std::string(dir->d_name);
        std::string filepath = directory + "/" + filename;

        if(filename == "." || filename == ".." || filename == "")
            continue;

        struct stat filestatus;
        lstat(filepath.c_str(), &filestatus);

        if (S_ISREG(filestatus.st_mode)) {
            if (unlink(filepath.c_str()) != 0)
                success = false;
        }
        else if (S_ISDIR(filestatus.st_mode)) {
            if (!removeDirectory(filepath))
                success = false;
        }
    }

    closedir(d);

    if (rmdir(directory.c_str()) != 0)
        success = false;

    return success;
}



bool mhwd::checkExist(const std::string path) {
    struct stat filestatus;
    if (stat(path.c_str(), &filestatus) != 0)
        return false;
    else
        return true;
}



bool mhwd::createDir(const std::string path, const mode_t mode) {
    mode_t process_mask = umask(0);
    int ret = mkdir(path.c_str(), mode);
    umask(process_mask);

    return (ret == 0);
}



//#####################################//
//### Private - Script & Operations ###//
//#####################################//


void mhwd::emitMessageFunc(mhwd::Data *data, mhwd::MESSAGETYPE type, std::string str) {
    if (data->environment.messageFunc == NULL)
        return;

    data->environment.messageFunc(type, str);
}



mhwd::STATUS mhwd::installConfig(mhwd::Data *data, mhwd::Config *config) {
    std::string databaseDir;

    // Get the right configs
    if (config->type == mhwd::TYPE_USB)
        databaseDir = MHWD_USB_DATABASE_DIR;
    else
        databaseDir = MHWD_PCI_DATABASE_DIR;

    // Run script
    if (!runScript(data, config, mhwd::Transaction::TYPE_INSTALL))
        return STATUS_ERROR_SCRIPT_FAILED;

    if (!copyDirectory(config->basePath, databaseDir + "/" + config->name))
        return STATUS_ERROR_SET_DATABASE;

    // Installed config vectors have to be updated manual with updateInstalledConfigData(mhwd::Data*)

    return STATUS_SUCCESS;
}



mhwd::STATUS mhwd::uninstallConfig(mhwd::Data *data, mhwd::Config *config) {
    mhwd::Config *installedConfig = getInstalledConfig(data, config->name, config->type);

    // Check if installed
    if (installedConfig == NULL)
        return STATUS_ERROR_NOT_INSTALLED;
    else if (installedConfig->basePath != config->basePath)
        return STATUS_ERROR_NO_MATCH_LOCAL_CONFIG;


    // TODO: Should we check for local requirements here?

    // Run script
    if (!runScript(data, installedConfig, mhwd::Transaction::TYPE_REMOVE))
        return STATUS_ERROR_SCRIPT_FAILED;

    if (!removeDirectory(installedConfig->basePath))
        return STATUS_ERROR_SET_DATABASE;

    // Installed config vectors have to be updated manual with updateInstalledConfigData(mhwd::Data*)

    return STATUS_SUCCESS;
}



bool mhwd::runScript(mhwd::Data *data, mhwd::Config *config, mhwd::Transaction::TYPE operationType) {
    std::string cmd = "exec " + std::string(MHWD_SCRIPT_PATH);

    if (operationType == mhwd::Transaction::TYPE_REMOVE)
        cmd += " --remove";
    else
        cmd += " --install";

    if (data->environment.syncPackageManagerDatabase)
        cmd += " --sync";

    cmd += " --cachedir \"" + data->environment.PMCachePath + "\"";
    cmd += " --pmconfig \"" + data->environment.PMConfigPath + "\"";
    cmd += " --pmroot \"" + data->environment.PMRootPath + "\"";
    cmd += " --config \"" + config->configPath + "\"";


    // Set all config devices as argument
    std::vector<mhwd::Device*> foundDevices, devices;
    getAllDevicesOfConfig(data, config, &foundDevices);

    for (std::vector<mhwd::Device*>::iterator iterator = foundDevices.begin(); iterator != foundDevices.end(); iterator++) {
        bool found = false;

        // Check if already in list
        for (std::vector<mhwd::Device*>::iterator dev = devices.begin(); dev != devices.end(); dev++) {
            if ((*iterator)->sysfsBusID == (*dev)->sysfsBusID && (*iterator)->sysfsID == (*dev)->sysfsID) {
                found = true;
                break;
            }
        }

        if (!found)
            devices.push_back((*iterator));
    }

    for (std::vector<mhwd::Device*>::iterator dev = devices.begin(); dev != devices.end(); dev++) {
        Vita::string busID = (*dev)->sysfsBusID;

        if (config->type == mhwd::TYPE_PCI) {
            std::vector<Vita::string> split = Vita::string(busID).replace(".", ":").explode(":");
            const int size = split.size();

            if (size >= 3) {
                // Convert to int to remove leading 0
                busID = Vita::string::toStr<int>(hexToInt(split[size-3]));
                busID += ":" + Vita::string::toStr<int>(hexToInt(split[size-2]));
                busID += ":" + Vita::string::toStr<int>(hexToInt(split[size-1]));
            }
        }

        cmd += " --device \"" + (*dev)->classID + "|" + (*dev)->vendorID + "|" + (*dev)->deviceID + "|" + busID + "\"";
    }

    cmd += " 2>&1";


    FILE *in;
    char buff[512];

    if(!(in = popen(cmd.c_str(), "r")))
        return false;

    while(fgets(buff, sizeof(buff), in) != NULL)
        emitMessageFunc(data, mhwd::MESSAGETYPE_CONSOLE_OUTPUT, std::string(buff));

    int stat = pclose(in);

    if(WEXITSTATUS(stat) != 0)
        return false;

    // Only one database sync is required
    if (operationType == mhwd::Transaction::TYPE_INSTALL)
        data->environment.syncPackageManagerDatabase = false;

    return true;
}
