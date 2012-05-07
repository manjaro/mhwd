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



//########################//
//### Public Functions ###//
//########################//



void mhwd::initData(mhwd::Data *data) {
    data->PCIDevices.clear();
    data->USBDevices.clear();
    data->installedPCIConfigs.clear();
    data->installedUSBConfigs.clear();
    data->invalidConfigs.clear();
    data->lastError.clear();

    data->environment.cachePath = MHWD_CACHE_DIR;
    data->environment.messageFunc = NULL;
}


void mhwd::fillData(mhwd::Data *data) {
    data->PCIDevices.clear();
    data->USBDevices.clear();
    data->installedPCIConfigs.clear();
    data->installedUSBConfigs.clear();
    data->invalidConfigs.clear();
    data->lastError.clear();

    setInstalledConfigs(data, mhwd::TYPE_PCI);
    setInstalledConfigs(data, mhwd::TYPE_USB);

    setDevices(data, mhwd::TYPE_PCI);
    setDevices(data, mhwd::TYPE_USB);
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



bool mhwd::installConfig(mhwd::Data *data, mhwd::Config *config) {
    std::string databaseDir;

    // Get the right configs
    if (config->type == mhwd::TYPE_USB)
        databaseDir = MHWD_USB_DATABASE_DIR;
    else
        databaseDir = MHWD_PCI_DATABASE_DIR;

    // Check if already installed
    if (getInstalledConfig(data, config->name, config->type) != NULL) {
        data->lastError = "a version of config " + config->name + " is already installed";
        return false;
    }

    // Check for mhwd config dependendcies and conflicts
    if (!checkDependenciesConflicts(data, config))
        return false;

    // Run script
    if (!runScript(data, config, SCRIPTOPERATION_INSTALL)) {
        data->lastError = "failed to run script or script failed";
        return false;
    }

    if (!copyDirectory(config->basePath, databaseDir + "/" + config->name)) {
        data->lastError = "failed to set local database:\nfailed to copy '" + config->basePath + "' to '" + databaseDir + "/" + config->name + "'";
        return false;
    }

    // Update installed config vectors
    updateInstalledConfigData(data);

    return true;
}



bool mhwd::uninstallConfig(mhwd::Data *data, mhwd::Config *config) {
    mhwd::Config *installedConfig = getInstalledConfig(data, config->name, config->type);
    std::vector<mhwd::Config>* installedConfigs;

    if (config->type == mhwd::TYPE_USB)
        installedConfigs = &data->installedUSBConfigs;
    else
        installedConfigs = &data->installedPCIConfigs;


    // Check if installed
    if (installedConfig == NULL) {
        data->lastError = "config is not installed";
        return false;
    }
    else if (installedConfig->basePath != config->basePath) {
        data->lastError = "passed config does not match with local config";
        return false;
    }

    // Check if this config is required by another installed config
    for (std::vector<mhwd::Config>::const_iterator iterator = installedConfigs->begin(); iterator != installedConfigs->end(); iterator++) {
        for (std::vector<std::string>::const_iterator depend = (*iterator).dependencies.begin(); depend != (*iterator).dependencies.end(); depend++) {
            if ((*depend) == config->name) {
                data->lastError = (*iterator).name + " requires " + config->name;
                return false;
            }
        }
    }


    // Run script
    if (!runScript(data, installedConfig, SCRIPTOPERATION_REMOVE)) {
        data->lastError = "failed to run script or script failed";
        return false;
    }

    if (!removeDirectory(installedConfig->basePath)) {
        data->lastError = "failed to set local database:\nfailed to remove '" + installedConfig->basePath + "'";
        return false;
    }

    // Update installed config vectors
    updateInstalledConfigData(data);

    return true;
}



mhwd::Config* mhwd::getInstalledConfig(mhwd::Data *data, const std::string configName, const TYPE configType) {
    std::vector<mhwd::Config>* installedConfigs;

    // Get the right configs
    if (configType == mhwd::TYPE_USB)
        installedConfigs = &data->installedUSBConfigs;
    else
        installedConfigs = &data->installedPCIConfigs;

    for (std::vector<mhwd::Config>::iterator iterator = installedConfigs->begin(); iterator != installedConfigs->end(); iterator++) {
        if (configName == (*iterator).name)
            return &(*iterator);
    }

    return NULL;
}



mhwd::Config* mhwd::getAvailableConfig(mhwd::Data *data, const std::string configName, const TYPE configType) {
    std::vector<mhwd::Device> *devices;

    // Get the right devices
    if (configType == mhwd::TYPE_USB)
        devices = &data->USBDevices;
    else
        devices = &data->PCIDevices;


    for (std::vector<mhwd::Device>::iterator dev_iter = devices->begin(); dev_iter != devices->end(); dev_iter++) {
        if ((*dev_iter).availableConfigs.empty())
            continue;

        for (std::vector<mhwd::Config>::iterator iterator = (*dev_iter).availableConfigs.begin(); iterator != (*dev_iter).availableConfigs.end(); iterator++) {
            if (configName == (*iterator).name)
                return &(*iterator);
        }
    }

    return NULL;
}





//#########################//
//### Private - Devices ###//
//#########################//



void mhwd::setDevices(mhwd::Data *data, mhwd::TYPE type) {
    hd_data_t *hd_data;
    hd_t *hd;
    hw_item hw;
    std::vector<mhwd::Config>* installedConfigs;
    std::vector<mhwd::Device>* devices;

    if (type == mhwd::TYPE_USB) {
        hw = hw_usb;
        installedConfigs = &data->installedUSBConfigs;
        devices = &data->USBDevices;
    }
    else {
        hw = hw_pci;
        installedConfigs = &data->installedPCIConfigs;
        devices = &data->PCIDevices;
    }


    hd_data = (hd_data_t*)calloc(1, sizeof *hd_data);
    hd = hd_list(hd_data, hw, 1, NULL);

    for(; hd; hd = hd->next) {
        struct mhwd::Device device;
        device.type = type;
        device.classID = from_Hex(hd->base_class.id, 2) + from_Hex(hd->sub_class.id, 2).toLower();
        device.vendorID = from_Hex(hd->vendor.id, 4).toLower();
        device.deviceID = from_Hex(hd->device.id, 4).toLower();
        device.className = from_CharArray(hd->base_class.name);
        device.vendorName = from_CharArray(hd->vendor.name);
        device.deviceName = from_CharArray(hd->device.name);

        devices->push_back(device);
    }

    hd_free_hd_list(hd);
    hd_free_hd_data(hd_data);
    free(hd_data);

    // Fill the config vectors
    setMatchingConfigs(data, devices, type, false);
    setMatchingConfigs(devices, installedConfigs, true);
}



Vita::string mhwd::from_Hex(uint16_t hexnum, int fill) {
    std::stringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(fill) << hexnum;
    return stream.str();
}



Vita::string mhwd::from_CharArray(char* c) {
    if (c == NULL)
        return "";

    return Vita::string(c);
}



void mhwd::addConfigSorted(std::vector<mhwd::Config>* configs, mhwd::Config* config) {
    for (std::vector<mhwd::Config>::const_iterator iterator = configs->begin(); iterator != configs->end(); iterator++) {
        if (config->name == (*iterator).name)
            return;
    }

    for (std::vector<mhwd::Config>::iterator iterator = configs->begin(); iterator != configs->end(); iterator++) {
        if (config->priority > (*iterator).priority) {
            configs->insert(iterator, *config);
            return;
        }
    }

    configs->push_back(*config);
}




//#########################//
//### Private - Configs ###//
//#########################//



bool mhwd::checkDependenciesConflicts(mhwd::Data *data, mhwd::Config *config) {
    std::vector<mhwd::Config>* installedConfigs;

    // Get the right configs
    if (config->type == mhwd::TYPE_USB)
        installedConfigs = &data->installedUSBConfigs;
    else
        installedConfigs = &data->installedPCIConfigs;


    for (std::vector<std::string>::const_iterator conflict = config->conflicts.begin(); conflict != config->conflicts.end(); conflict++) {
        for (std::vector<mhwd::Config>::const_iterator iterator = installedConfigs->begin(); iterator != installedConfigs->end(); iterator++) {
            if ((*conflict) == (*iterator).name) {
                data->lastError = config->name + " conflicts with " + (*conflict);
                return false;
            }
        }
    }

    for (std::vector<std::string>::const_iterator depend = config->dependencies.begin(); depend != config->dependencies.end(); depend++) {
        bool found = false;

        for (std::vector<mhwd::Config>::const_iterator iterator = installedConfigs->begin(); iterator != installedConfigs->end(); iterator++) {
            if ((*depend) == (*iterator).name) {
                found = true;
                break;
            }
        }

        if (!found) {
            data->lastError = config->name + " depends on " + (*depend) + " but " + (*depend) + " is not installed";
            return false;
        }
    }

    return true;
}



void mhwd::updateInstalledConfigData(mhwd::Data *data) {
    // Clear and refill the installed config vectors
    data->installedPCIConfigs.clear();
    data->installedUSBConfigs.clear();
    setInstalledConfigs(data, mhwd::TYPE_PCI);
    setInstalledConfigs(data, mhwd::TYPE_USB);

    // Clear installed config vector in each device element
    for (std::vector<mhwd::Device>::iterator iterator = data->PCIDevices.begin(); iterator != data->PCIDevices.end(); iterator++) {
        (*iterator).installedConfigs.clear();
    }

    for (std::vector<mhwd::Device>::iterator iterator = data->USBDevices.begin(); iterator != data->USBDevices.end(); iterator++) {
        (*iterator).installedConfigs.clear();
    }

    // Refill it again
    setMatchingConfigs(&data->PCIDevices, &data->installedPCIConfigs, true);
    setMatchingConfigs(&data->USBDevices, &data->installedUSBConfigs, true);
}



void mhwd::setInstalledConfigs(mhwd::Data *data, mhwd::TYPE type) {
    std::vector<std::string> configPaths;
    std::vector<mhwd::Config>* configs;

    if (type == mhwd::TYPE_USB) {
        configs = &data->installedUSBConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_USB_DATABASE_DIR, MHWD_CONFIG_NAME);
    }
    else {
        configs = &data->installedPCIConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_PCI_DATABASE_DIR, MHWD_CONFIG_NAME);
    }


    for (std::vector<std::string>::const_iterator iterator = configPaths.begin(); iterator != configPaths.end(); iterator++) {
        struct mhwd::Config config;

        if (fillConfig(&config, (*iterator), type))
            configs->push_back(config);
        else
            data->invalidConfigs.push_back(config);
    }
}



void mhwd::setMatchingConfigs(mhwd::Data *data, std::vector<mhwd::Device>* devices, mhwd::TYPE type, bool setAsInstalled) {
    std::vector<std::string> configPaths;

    if (type == mhwd::TYPE_USB)
        configPaths = getRecursiveDirectoryFileList(MHWD_USB_CONFIG_DIR, MHWD_CONFIG_NAME);
    else
        configPaths = getRecursiveDirectoryFileList(MHWD_PCI_CONFIG_DIR, MHWD_CONFIG_NAME);


    for (std::vector<std::string>::const_iterator iterator = configPaths.begin(); iterator != configPaths.end(); iterator++) {
        mhwd::Config config;

        if (fillConfig(&config, (*iterator), type))
            setMatchingConfig(&config, devices, setAsInstalled);
        else
            data->invalidConfigs.push_back(config);
    }
}



void mhwd::setMatchingConfigs(std::vector<mhwd::Device>* devices, std::vector<mhwd::Config>* configs, bool setAsInstalled) {
    for (std::vector<mhwd::Config>::iterator iterator = configs->begin(); iterator != configs->end(); iterator++) {
        setMatchingConfig(&(*iterator), devices, setAsInstalled);
    }
}



void mhwd::setMatchingConfig(mhwd::Config* config, std::vector<mhwd::Device>* devices, bool setAsInstalled) {
    std::vector<mhwd::Device*> foundDevices;

    for (std::vector<mhwd::Config::HardwareIDs>::const_iterator i_hwdIDs = config->hwdIDs.begin(); i_hwdIDs != config->hwdIDs.end(); i_hwdIDs++) {
        bool foundDevice = false;

        // Check all devices
        for (std::vector<mhwd::Device>::iterator i_device = devices->begin(); i_device != devices->end(); i_device++) {
            bool found = false;

            // Check class ids
            for (std::vector<std::string>::const_iterator iterator = (*i_hwdIDs).classIDs.begin(); iterator != (*i_hwdIDs).classIDs.end(); iterator++) {
                if (*iterator == "*" || *iterator == (*i_device).classID) {
                    found = true;
                    break;
                }
            }

            if (!found)
                continue;

            // Check vendor ids
            found = false;

            for (std::vector<std::string>::const_iterator iterator = (*i_hwdIDs).vendorIDs.begin(); iterator != (*i_hwdIDs).vendorIDs.end(); iterator++) {
                if (*iterator == "*" || *iterator == (*i_device).vendorID) {
                    found = true;
                    break;
                }
            }

            if (!found)
                continue;

            // Check device ids
            found = false;

            for (std::vector<std::string>::const_iterator iterator = (*i_hwdIDs).deviceIDs.begin(); iterator != (*i_hwdIDs).deviceIDs.end(); iterator++) {
                if (*iterator == "*" || *iterator == (*i_device).deviceID) {
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
            addConfigSorted(&(*iterator)->installedConfigs, config);
        else
            addConfigSorted(&(*iterator)->availableConfigs, config);
    }
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
            // Add new HardwareIDs group to vector if vector is empty
            if (!config->hwdIDs.back().classIDs.empty()) {
                mhwd::Config::HardwareIDs hwdID;
                config->hwdIDs.push_back(hwdID);
            }

            config->hwdIDs.back().classIDs = splitValue(value);
        }
        else if (key == "vendorids") {
            // Add new HardwareIDs group to vector if vector is empty
            if (!config->hwdIDs.back().vendorIDs.empty()) {
                mhwd::Config::HardwareIDs hwdID;
                config->hwdIDs.push_back(hwdID);
            }

            config->hwdIDs.back().vendorIDs = splitValue(value);
        }
        else if (key == "deviceids") {
            // Add new HardwareIDs group to vector if vector is empty
            if (!config->hwdIDs.back().deviceIDs.empty()) {
                mhwd::Config::HardwareIDs hwdID;
                config->hwdIDs.push_back(hwdID);
            }

            config->hwdIDs.back().deviceIDs = splitValue(value);
        }
        else if (key == "dependencies" || key == "depends") {
            config->dependencies = splitValue(value, "mhwd");
        }
        else if (key == "conflicts") {
            config->conflicts = splitValue(value, "mhwd");
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
        if (mkdir(destination.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH) != 0)
            return false;
    }
    else if (S_ISREG(filestatus.st_mode)) {
        return false;
    }
    else if (S_ISDIR(filestatus.st_mode)) {
        if (!removeDirectory(destination))
            return false;

        if (mkdir(destination.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH) != 0)
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



bool mhwd::copyFile(const std::string source, const std::string destination) {
    int c;
    FILE *in,*out;

    in = fopen(source.c_str(), "r");
    out = fopen(destination.c_str(), "w");

    if(in==NULL || !in)
        return false;
    else if(out==NULL || !out)
        return false;

    while((c=getc(in))!=EOF)
        putc(c,out);

    fclose(in);
    fclose(out);

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
            if (remove(filepath.c_str()) != 0)
                success = false;
        }
        else if (S_ISDIR(filestatus.st_mode)) {
            if (!removeDirectory(filepath)) {
                std::cout << "bb" << std::endl;
                success = false;
            }
        }
    }

    closedir(d);

    if (remove(directory.c_str()) != 0)
        success = false;

    return success;
}



//########################//
//### Private - Script ###//
//########################//



bool mhwd::runScript(mhwd::Data *data, mhwd::Config *config, SCRIPTOPERATION scriptOperation) {
    std::string cmd = "exec " + std::string(MHWD_SCRIPT_PATH);

    if (scriptOperation == SCRIPTOPERATION_REMOVE)
        cmd += " --remove";
    else
        cmd += " --install";

    cmd += " --cachedir \"" + data->environment.cachePath + "\"";
    cmd += " --config \"" + config->configPath + "\"";
    cmd += " 2>&1";


    FILE *in;
    char buff[512];

    if(!(in = popen(cmd.c_str(), "r")))
        return false;

    while(fgets(buff, sizeof(buff), in) != NULL){
        if (data->environment.messageFunc != NULL)
            data->environment.messageFunc(std::string(buff));
    }

    int stat = pclose(in);

    if(WEXITSTATUS(stat) != 0)
        return false;

    return true;
}

