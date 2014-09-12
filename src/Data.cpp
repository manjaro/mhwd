/*
 * Data.cpp
 *
 *  Created on: 28 sie 2014
 *      Author: dec
 */

#include <dirent.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "Data.hpp"

Data::Data()
{
    fillDevices("PCI");
    fillDevices("USB");

    updateConfigData();
}

Data::~Data()
{
    for (auto& PCIDevice : PCIDevices)
    {
        delete PCIDevice;
        PCIDevice = nullptr;
    }

    for (auto& USBDevice : USBDevices)
    {
        delete USBDevice;
        USBDevice = nullptr;
    }

    for (auto& installedPCIConfig : installedPCIConfigs)
    {
        delete installedPCIConfig;
        installedPCIConfig = nullptr;
    }

    for (auto& installedUSBConfig : installedUSBConfigs)
    {
        delete installedUSBConfig;
        installedUSBConfig = nullptr;
    }

    for (auto& PCIConfig : allPCIConfigs)
    {
        delete PCIConfig;
        PCIConfig = nullptr;
    }

    for (auto& USBConfig : allUSBConfigs)
    {
        delete USBConfig;
        USBConfig = nullptr;
    }

    for (auto& invalidConfig : invalidConfigs)
    {
        delete invalidConfig;
        invalidConfig = nullptr;
    }

    PCIDevices.clear();
    USBDevices.clear();
    installedPCIConfigs.clear();
    installedUSBConfigs.clear();
    allUSBConfigs.clear();
    allPCIConfigs.clear();
    invalidConfigs.clear();
}

void Data::updateInstalledConfigData()
{
    // Clear config vectors in each device element
    for (auto PCIDevice = PCIDevices.begin();
            PCIDevice != PCIDevices.end(); PCIDevice++)
    {
        (*PCIDevice)->installedConfigs.clear();
    }

    for (auto USBDevice = USBDevices.begin();
            USBDevice != USBDevices.end(); USBDevice++)
    {
        (*USBDevice)->installedConfigs.clear();
    }

    // Clear installed config vectors
    for (auto& PCIConfig : installedPCIConfigs)
    {
        delete PCIConfig;
        PCIConfig = nullptr;
    }

    for (auto& USBConfig : installedUSBConfigs)
    {
        delete USBConfig;
        USBConfig = nullptr;
    }

    installedPCIConfigs.clear();
    installedUSBConfigs.clear();

    // Refill data
    fillInstalledConfigs("PCI");
    fillInstalledConfigs("USB");

    setMatchingConfigs(&PCIDevices, &installedPCIConfigs, true);
    setMatchingConfigs(&USBDevices, &installedUSBConfigs, true);
}

void Data::fillInstalledConfigs(std::string type)
{
    std::vector<std::string> configPaths;
    std::vector<Config*>* configs;

    if (type == "USB")
    {
        configs = &installedUSBConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_USB_DATABASE_DIR, MHWD_CONFIG_NAME);
    }
    else
    {
        configs = &installedPCIConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_PCI_DATABASE_DIR, MHWD_CONFIG_NAME);
    }

    for (auto configPath = configPaths.begin();
            configPath != configPaths.end(); ++configPath)
    {
        Config *config = new Config((*configPath), type);

        if (config->readConfigFile((*configPath)))
        {
            configs->push_back(config);
        }
        else
        {
            invalidConfigs.push_back(config);
        }
    }
}

void Data::getAllDevicesOfConfig(Config *config, std::vector<Device*>* foundDevices)
{
    std::vector<Device*> devices;

    if (config->type_ == "USB")
    {
        devices = USBDevices;
    }
    else
    {
        devices = PCIDevices;
    }

    getAllDevicesOfConfig(&devices, config, foundDevices);
}


void Data::getAllDevicesOfConfig(std::vector<Device*>* devices, Config *config,
        std::vector<Device*>* foundDevices)
{
    foundDevices->clear();

    for (auto hwdID = config->hwdIDs_.begin();
            hwdID != config->hwdIDs_.end(); ++hwdID)
    {
        bool foundDevice = false;
        // Check all devices
        for (auto i_device = devices->begin(); i_device != devices->end();
                ++i_device)
        {
            bool found = false;
            // Check class ids
            for (auto classID = hwdID->classIDs.begin();
                    classID != hwdID->classIDs.end(); ++classID)
            {
                if (*classID == "*" || *classID == (*i_device)->classID)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                continue;
            }
            else
            {
                // Check blacklisted class ids
                found = false;

                for (auto blacklistedClassID =
                        (*hwdID).blacklistedClassIDs.begin();
                        blacklistedClassID != (*hwdID).blacklistedClassIDs.end();
                        ++blacklistedClassID)
                {
                    if (*blacklistedClassID == (*i_device)->classID)
                    {
                        found = true;
                        break;
                    }
                }

                if (found)
                {
                    continue;
                }
                else
                {
                    // Check vendor ids
                    found = false;

                    for (auto vendorID = hwdID->vendorIDs.begin();
                            vendorID != hwdID->vendorIDs.end(); ++vendorID)
                    {
                        if (*vendorID == "*" || *vendorID == (*i_device)->vendorID)
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        continue;
                    }
                    else
                    {
                        // Check blacklisted vendor ids
                        found = false;

                        for (auto blacklistedVendorID =
                                hwdID->blacklistedVendorIDs.begin();
                                blacklistedVendorID != hwdID->blacklistedVendorIDs.end();
                                ++blacklistedVendorID)
                        {
                            if (*blacklistedVendorID == (*i_device)->vendorID)
                            {
                                found = true;
                                break;
                            }
                        }

                        if (found)
                        {
                            continue;
                        }
                        else
                        {
                            // Check device ids
                            found = false;

                            for (auto deviceID = hwdID->deviceIDs.begin();
                                    deviceID != hwdID->deviceIDs.end(); ++deviceID)
                            {
                                if (*deviceID == "*" || *deviceID == (*i_device)->deviceID)
                                {
                                    found = true;
                                    break;
                                }
                            }

                            if (!found)
                            {
                                continue;
                            }
                            else
                            {
                                // Check blacklisted device ids
                                found = false;

                                for (auto blacklistedDeviceID =
                                        hwdID->blacklistedDeviceIDs.begin();
                                        blacklistedDeviceID != hwdID->blacklistedDeviceIDs.end();
                                        ++blacklistedDeviceID)
                                {
                                    if (*blacklistedDeviceID == (*i_device)->deviceID)
                                    {
                                        found = true;
                                        break;
                                    }
                                }

                                if (found)
                                {
                                    continue;
                                }
                                else
                                {
                                    foundDevice = true;
                                    foundDevices->push_back((*i_device));
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!foundDevice)
        {
            foundDevices->clear();
            return;
        }
    }
}

std::vector<Config*> Data::getAllDependenciesToInstall(Config *config)
{
    std::vector<Config*> depends;
    std::vector<Config*> installedConfigs;

    // Get the right configs
    if (config->type_ == "USB")
    {
        installedConfigs = installedUSBConfigs;
    }
    else
    {
        installedConfigs = installedPCIConfigs;
    }

    // Get all depends
    getAllDependenciesToInstall(config, &installedConfigs, &depends);

    return depends;
}

void Data::getAllDependenciesToInstall(Config *config,
        std::vector<Config*>* installedConfigs, std::vector<Config*> *dependencies)
{
    for (auto configDependency = config->dependencies_.begin();
            configDependency != config->dependencies_.end(); configDependency++)
    {
        bool found = false;

        for (auto installedConfig = installedConfigs->begin();
                installedConfig != installedConfigs->end(); installedConfig++)
        {
            if ((*configDependency) == (*installedConfig)->name_)
            {
                found = true;
                break;
            }
        }

        if (found)
            continue;

        // Check if already in vector
        for (auto dependency = dependencies->begin();
                dependency != dependencies->end(); dependency++)
        {
            if ((*dependency)->name_ == (*configDependency))
            {
                found = true;
                break;
            }
        }

        if (found)
        {
            continue;
        }
        else
        {
            // Add to vector and check for further subdepends...
            Config *dependconfig = getDatabaseConfig((*configDependency), config->type_);
            if (dependconfig == nullptr)
            {
                continue;
            }
            else
            {
                dependencies->push_back(dependconfig);
                getAllDependenciesToInstall(dependconfig, installedConfigs, dependencies);
            }
        }
    }
}

Config* Data::getDatabaseConfig(const std::string configName,
        const std::string configType)
{
    std::vector<Config*> allConfigs;

    // Get the right configs
    if (configType == "USB")
    {
        allConfigs = allUSBConfigs;
    }
    else
    {
        allConfigs = allPCIConfigs;
    }

    for (auto config = allConfigs.begin(); config != allConfigs.end();
            config++)
    {
        if (configName == (*config)->name_)
        {
            return (*config);
        }
    }

    return nullptr;
}

std::vector<Config*> Data::getAllLocalConflicts(Config *config)
{
    std::vector<Config*> conflicts;
    std::vector<Config*> dependencies = getAllDependenciesToInstall(config);
    std::vector<Config*> installedConfigs;

    // Get the right configs
    if (config->type_ == "USB")
    {
        installedConfigs = installedUSBConfigs;
    }
    else
    {
        installedConfigs = installedPCIConfigs;
    }

    dependencies.push_back(config);

    for (auto dependency = dependencies.begin();
            dependency != dependencies.end(); dependency++)
    {
        for (auto dependencyConflict =
                (*dependency)->conflicts_.begin();
                dependencyConflict != (*dependency)->conflicts_.end(); dependencyConflict++)
        {
            for (auto installedConfig = installedConfigs.begin();
                    installedConfig != installedConfigs.end(); installedConfig++)
            {
                if ((*dependencyConflict) != (*installedConfig)->name_)
                {
                    continue;
                }
                else
                {
                    // Check if already in vector
                    bool found = false;
                    for (auto conflict = conflicts.begin();
                            conflict != conflicts.end(); conflict++)
                    {
                        if ((*conflict)->name_ == (*dependencyConflict))
                        {
                            found = true;
                            break;
                        }
                    }

                    if (found)
                    {
                        continue;
                    }
                    else
                    {
                        conflicts.push_back((*installedConfig));
                        break;
                    }
                }
            }
        }
    }

    return conflicts;
}

std::vector<Config*> Data::getAllLocalRequirements(Config *config)
{
    std::vector<Config*> requirements;
    std::vector<Config*> installedConfigs;

    // Get the right configs
    if (config->type_ == "USB")
    {
        installedConfigs = installedUSBConfigs;
    }
    else
    {
        installedConfigs = installedPCIConfigs;
    }

    // Check if this config is required by another installed config
    for (auto installedConfig = installedConfigs.begin();
            installedConfig != installedConfigs.end(); installedConfig++)
    {
        for (auto dependency = (*installedConfig)->dependencies_.begin();
                dependency != (*installedConfig)->dependencies_.end(); dependency++)
        {
            if ((*dependency) != config->name_)
            {
                continue;
            }
            else
            {
                // Check if already in vector
                bool found = false;
                for (auto requirement = requirements.begin();
                        requirement != requirements.end(); requirement++)
                {
                    if ((*requirement)->name_ == (*installedConfig)->name_)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    requirements.push_back((*installedConfig));
                    break;
                }
            }
        }
    }

    return requirements;
}

void Data::fillDevices(std::string type)
{
    hw_item hw;
    std::vector<Device*>* devices;

    if (type == "USB")
    {
        hw = hw_usb;
        devices = &USBDevices;
    }
    else
    {
        hw = hw_pci;
        devices = &PCIDevices;
    }

    // Get the hardware devices
    hd_data_t *hd_data = new hd_data_t();

    hd_t *hd;
    hd = hd_list(hd_data, hw, 1, nullptr);
    hd_t *beginningOfhd = hd;

    Device *device;
    for (; hd; hd = hd->next)
    {
        device = new Device();
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

    hd_free_hd_list(beginningOfhd);
    hd_free_hd_data(hd_data);
    delete hd_data;
}

void Data::fillAllConfigs(std::string type)
{
    std::vector<std::string> configPaths;
    std::vector<Config*>* configs;

    if (type == "USB")
    {
        configs = &allUSBConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_USB_CONFIG_DIR, MHWD_CONFIG_NAME);
    }
    else
    {
        configs = &allPCIConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_PCI_CONFIG_DIR, MHWD_CONFIG_NAME);
    }

    for (auto configPath = configPaths.begin();
            configPath != configPaths.end(); ++configPath)
    {
        Config *config = new Config((*configPath), type);

        if (config->readConfigFile((*configPath)))
        {
            configs->push_back(config);
        }
        else
        {
            invalidConfigs.push_back(config);
        }
    }
}

bool Data::fillConfig(Config *config, std::string configPath, std::string type)
{
    config->type_ = type;
    config->priority_ = 0;
    config->freedriver_ = true;
    config->basePath_ = configPath.substr(0, configPath.find_last_of('/'));
    config->configPath_ = configPath;

    // Add new HardwareIDs group to vector if vector is empty
    if (config->hwdIDs_.empty())
    {
        Config::HardwareID hwdID;
        config->hwdIDs_.push_back(hwdID);
    }

    return config->readConfigFile(config->configPath_);
}

std::vector<std::string> Data::getRecursiveDirectoryFileList(const std::string directoryPath,
        std::string onlyFilename)
{
    std::vector<std::string> list;
    struct dirent *dir;
    DIR *d = opendir(directoryPath.c_str());

    if (d)
    {
        while ((dir = readdir(d)) != nullptr)
        {
            std::string filename = std::string(dir->d_name);
            std::string filepath = directoryPath + "/" + filename;

            if (filename == "." || filename == ".." || filename == "")
            {
                continue;
            }
            else
            {
                struct stat filestatus;
                lstat(filepath.c_str(), &filestatus);

                if (S_ISREG(filestatus.st_mode) &&
                        (onlyFilename.empty() || (onlyFilename == filename)))
                {
                    list.push_back(filepath);
                }
                else if (S_ISDIR(filestatus.st_mode))
                {
                    std::vector<std::string> templist = getRecursiveDirectoryFileList(filepath,
                            onlyFilename);

                    for (auto iterator = templist.begin();
                            iterator != templist.end(); iterator++)
                    {
                        list.push_back((*iterator));
                    }
                }
            }
        }

        closedir(d);
    }

    return list;
}

Vita::string Data::getRightConfigPath(Vita::string str, Vita::string baseConfigPath)
{
    str = str.trim();
    if ((str.size() <= 0) || (str.substr(0, 1) == "/"))
    {
        return str;
    }
    return baseConfigPath + "/" + str;
}

std::vector<std::string> Data::splitValue(Vita::string str, Vita::string onlyEnding)
{
    std::vector<Vita::string> work = str.toLower().explode(" ");
    std::vector<std::string> final;

    for (auto iterator = work.begin(); iterator != work.end();
            iterator++)
    {
        if (*iterator != "" && onlyEnding.empty())
        {
            final.push_back(*iterator);
        }
        else if (*iterator != "" && Vita::string(*iterator).explode(".").back() == onlyEnding
                && (*iterator).size() > 5)
        {
            final.push_back(Vita::string(*iterator).substr(0, (*iterator).size() - 5));
        }
    }

    return final;
}

void Data::updateConfigData()
{
    // Clear config vectors in each device element
    for (auto PCIDevice = PCIDevices.begin();
            PCIDevice != PCIDevices.end(); PCIDevice++)
    {
        (*PCIDevice)->availableConfigs.clear();
    }

    for (auto USBDevice = USBDevices.begin();
            USBDevice != USBDevices.end(); USBDevice++)
    {
        (*USBDevice)->availableConfigs.clear();
    }

    // Clear installed config vectors
    for (auto& PCIConfig : allPCIConfigs)
    {
        delete PCIConfig;
        PCIConfig = nullptr;
    }

    for (auto& USBConfig : allUSBConfigs)
    {
        delete USBConfig;
        USBConfig = nullptr;
    }

    allPCIConfigs.clear();
    allUSBConfigs.clear();

    // Refill data
    fillAllConfigs("PCI");
    fillAllConfigs("USB");

    setMatchingConfigs(&PCIDevices, &allPCIConfigs, false);
    setMatchingConfigs(&USBDevices, &allUSBConfigs, false);

    // Update also installed config data
    updateInstalledConfigData();
}

void Data::setMatchingConfigs(std::vector<Device*>* devices, std::vector<Config*>* configs,
        bool setAsInstalled)
{
    for (auto config = configs->begin(); config != configs->end();
            ++config)
    {
        setMatchingConfig((*config), devices, setAsInstalled);
    }
}

void Data::setMatchingConfig(Config* config, std::vector<Device*>* devices, bool setAsInstalled)
{
    std::vector<Device*> foundDevices;

    getAllDevicesOfConfig(devices, config, &foundDevices);

    // Set config to all matching devices
    for (auto foundDevice = foundDevices.begin();
            foundDevice != foundDevices.end(); ++foundDevice)
    {
        if (setAsInstalled)
        {
            addConfigSorted(&(*foundDevice)->installedConfigs, config);
        }
        else
        {
            addConfigSorted(&(*foundDevice)->availableConfigs, config);
        }
    }
}

void Data::addConfigSorted(std::vector<Config*>* configs, Config* config)
{
    for (auto iterator = configs->begin();
            iterator != configs->end(); iterator++)
    {
        if (config->name_ == (*iterator)->name_)
        {
            return;
        }
    }

    for (auto iterator = configs->begin(); iterator != configs->end();
            iterator++)
    {
        if (config->priority_ > (*iterator)->priority_)
        {
            configs->insert(iterator, config);
            return;
        }
    }

    configs->push_back(config);
}

Vita::string Data::from_Hex(uint16_t hexnum, int fill)
{
    std::stringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(fill) << hexnum;
    return stream.str();
}

Vita::string Data::from_CharArray(char* c)
{
    if (c == nullptr)
    {
        return "";
    }

    return Vita::string(c);
}
