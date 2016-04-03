/*
 *  This file is part of the mhwd - Manjaro Hardware Detection project
 *
 *  mhwd - Manjaro Hardware Detection
 *  Roland Singer <roland@manjaro.org>
 *  Łukasz Matysiak <december0123@gmail.com>
 *  Filipe Marques <eagle.software3@gmail.com>
 *
 *  Copyright (C) 2012 - 2016 Manjaro (http://manjaro.org)
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

#include "Data.hpp"

#include <dirent.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

Data::Data()
{
    fillDevices(hw_pci, PCIDevices);
    fillDevices(hw_usb, USBDevices);

    updateConfigData();
}

void Data::updateInstalledConfigData()
{
    // Clear config vectors in each device element

    for (auto& PCIDevice : PCIDevices)
    {
        PCIDevice->installedConfigs_.clear();
    }

    for (auto& USBDevice : USBDevices)
    {
        USBDevice->installedConfigs_.clear();
    }

    installedPCIConfigs.clear();
    installedUSBConfigs.clear();

    // Refill data
    fillInstalledConfigs("PCI");
    fillInstalledConfigs("USB");

    setMatchingConfigs(PCIDevices, installedPCIConfigs, true);
    setMatchingConfigs(USBDevices, installedUSBConfigs, true);
}

void Data::fillInstalledConfigs(std::string type)
{
    std::vector<std::string> configPaths;
    std::vector<std::shared_ptr<Config>>* configs;

    if ("USB" == type)
    {
        configs = &installedUSBConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_USB_DATABASE_DIR, MHWD_CONFIG_NAME);
    }
    else
    {
        configs = &installedPCIConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_PCI_DATABASE_DIR, MHWD_CONFIG_NAME);
    }

    for (const auto& configPath : configPaths)
    {
        Config *config = new Config(configPath, type);

        if (config->readConfigFile(configPath))
        {
            configs->push_back(std::shared_ptr<Config>{config});
        }
        else
        {
            invalidConfigs.push_back(std::shared_ptr<Config>{config});
        }
    }
}

void Data::getAllDevicesOfConfig(std::shared_ptr<Config> config, std::vector<std::shared_ptr<Device>>& foundDevices)
{
    std::vector<std::shared_ptr<Device>> devices;

    if ("USB" == config->type_)
    {
        devices = USBDevices;
    }
    else
    {
        devices = PCIDevices;
    }

    getAllDevicesOfConfig(devices, config, foundDevices);
}

void Data::getAllDevicesOfConfig(const std::vector<std::shared_ptr<Device>>& devices,
        std::shared_ptr<Config> config,
        std::vector<std::shared_ptr<Device>>& foundDevices)
{
    foundDevices.clear();

    for (auto&& hwdID = config->hwdIDs_.begin();
            hwdID != config->hwdIDs_.end(); ++hwdID)
    {
        bool foundDevice = false;
        // Check all devices
        for (auto&& i_device = devices.begin(); i_device != devices.end();
                ++i_device)
        {
            // Check class ids
            bool found = std::find_if(hwdID->classIDs.begin(), hwdID->classIDs.end(), [i_device](const std::string& classID){
                                return (("*" == classID) || (classID == (*i_device)->classID_));
                            }) != hwdID->classIDs.end();

            if (found)
            {
                // Check blacklisted class ids
                found = std::find_if(hwdID->blacklistedClassIDs.begin(), hwdID->blacklistedClassIDs.end(), [i_device](const std::string& blacklistedClassID){
                                return (blacklistedClassID == (*i_device)->classID_);
                            }) != hwdID->blacklistedClassIDs.end();

                if (!found)
                {
                    // Check vendor ids
                    found = std::find_if(hwdID->vendorIDs.begin(), hwdID->vendorIDs.end(), [i_device](const std::string& vendorID){
                                    return (("*" == vendorID) || (vendorID == (*i_device)->vendorID_));
                                }) != hwdID->vendorIDs.end();

                    if (found)
                    {
                        // Check blacklisted vendor ids
                        found = std::find_if(hwdID->blacklistedVendorIDs.begin(), hwdID->blacklistedVendorIDs.end(), [i_device](const std::string& blacklistedVendorID){
                                        return (blacklistedVendorID == (*i_device)->vendorID_);
                                    }) != hwdID->blacklistedVendorIDs.end();

                        if (!found)
                        {
                            // Check device ids
                            found = std::find_if(hwdID->deviceIDs.begin(), hwdID->deviceIDs.end(), [i_device](const std::string& deviceID){
                                            return (("*" == deviceID) || (deviceID == (*i_device)->deviceID_));
                                        }) != hwdID->deviceIDs.end();

                            if (found)
                            {
                                // Check blacklisted device ids
                                found = std::find_if(hwdID->blacklistedDeviceIDs.begin(), hwdID->blacklistedDeviceIDs.end(), [i_device](const std::string& blacklistedDeviceID){
                                                return (blacklistedDeviceID == (*i_device)->deviceID_);
                                            }) != hwdID->blacklistedDeviceIDs.end();
                                if (!found)
                                {
                                    foundDevice = true;
                                    foundDevices.push_back(*i_device);
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!foundDevice)
        {
            foundDevices.clear();
            return;
        }
    }
}

std::vector<std::shared_ptr<Config>> Data::getAllDependenciesToInstall(
        std::shared_ptr<Config> config)
{
    std::vector<std::shared_ptr<Config>> depends;
    std::vector<std::shared_ptr<Config>> installedConfigs;

    if ("USB" == config->type_)
    {
        installedConfigs = installedUSBConfigs;
    }
    else
    {
        installedConfigs = installedPCIConfigs;
    }

    getAllDependenciesToInstall(config, installedConfigs, &depends);

    return depends;
}

void Data::getAllDependenciesToInstall(std::shared_ptr<Config> config,
        std::vector<std::shared_ptr<Config>>& installedConfigs,
        std::vector<std::shared_ptr<Config>> *dependencies)
{
    for (const auto& configDependency : config->dependencies_)
    {
        auto found = std::find_if(installedConfigs.begin(), installedConfigs.end(),
                [configDependency](const std::shared_ptr<Config>& config) -> bool {
                    return (config->name_ == configDependency);
                }) != installedConfigs.end();

        if (!found)
        {
            found = std::find_if(dependencies->begin(), dependencies->end(),
                    [configDependency](const std::shared_ptr<Config>& config) -> bool {
                        return (config->name_ == configDependency);
                    }) != dependencies->end();

            if (!found)
            {
                // Add to vector and check for further subdepends...
                std::shared_ptr<Config> dependconfig {
                    getDatabaseConfig(configDependency, config->type_)};
                if (nullptr != dependconfig)
                {
                    dependencies->emplace_back(dependconfig);
                    getAllDependenciesToInstall(dependconfig, installedConfigs, dependencies);
                }
            }
        }
    }
}

std::shared_ptr<Config> Data::getDatabaseConfig(const std::string configName,
        const std::string configType)
{
    std::vector<std::shared_ptr<Config>> allConfigs;

    if ("USB" == configType)
    {
        allConfigs = allUSBConfigs;
    }
    else
    {
        allConfigs = allPCIConfigs;
    }

    for (auto& config : allConfigs)
    {

        if (configName == config->name_)
        {

            return config;
        }
    }

    return nullptr;
}

std::vector<std::shared_ptr<Config>> Data::getAllLocalConflicts(std::shared_ptr<Config> config)
{
    std::vector<std::shared_ptr<Config>> conflicts;
    std::vector<std::shared_ptr<Config>> dependencies = getAllDependenciesToInstall(config);
    std::vector<std::shared_ptr<Config>> installedConfigs;

    if ("USB" == config->type_)
    {
        installedConfigs = installedUSBConfigs;
    }
    else
    {
        installedConfigs = installedPCIConfigs;
    }

    dependencies.emplace_back(config);

    for (const auto& dependency : dependencies)
    {
        for (const auto& dependencyConflict : dependency->conflicts_)
        {
            for (auto& installedConfig : installedConfigs)
            {
                if (dependencyConflict == installedConfig->name_)
                {
                    bool found = std::find_if(conflicts.begin(), conflicts.end(),
                            [&dependencyConflict](const std::shared_ptr<Config>& conflict)
                            {
                                return conflict->name_ == dependencyConflict;
                            }) != conflicts.end();

                    if (!found)
                    {
                        conflicts.emplace_back(installedConfig);
                        break;
                    }
                }
            }
        }
    }

    return conflicts;
}

std::vector<std::shared_ptr<Config>> Data::getAllLocalRequirements(std::shared_ptr<Config> config)
{
    std::vector<std::shared_ptr<Config>> requirements;
    std::vector<std::shared_ptr<Config>> installedConfigs;

    if ("USB" == config->type_)
    {
        installedConfigs = installedUSBConfigs;
    }
    else
    {
        installedConfigs = installedPCIConfigs;
    }

    // Check if this config is required by another installed config
    for (auto& installedConfig : installedConfigs)
    {
        for (const auto& dependency : installedConfig->dependencies_)
        {
            if (dependency == config->name_)
            {
                bool found = std::find_if(requirements.begin(), requirements.end(),
                        [&installedConfig](const std::shared_ptr<Config>& req)
                        {
                            return req->name_ == installedConfig->name_;
                        }) != requirements.end();

                if (!found)
                {
                    requirements.emplace_back(installedConfig);
                    break;
                }
            }
        }
    }

    return requirements;
}

void Data::fillDevices(hw_item hw, std::vector<std::shared_ptr<Device>>& devices)
{
    // Get the hardware devices
    std::unique_ptr<hd_data_t> hd_data{new hd_data_t()};
    hd_t *hd = hd_list(hd_data.get(), hw, 1, nullptr);

    std::unique_ptr<Device> device;
    for (hd_t *hdIter = hd; hdIter; hdIter = hdIter->next)
    {
        device.reset(new Device());
        device->type_ = (hw == hw_usb ? "USB" : "PCI");
        device->classID_ = from_Hex(hdIter->base_class.id, 2) + from_Hex(hdIter->sub_class.id, 2).toLower();
        device->vendorID_ = from_Hex(hdIter->vendor.id, 4).toLower();
        device->deviceID_ = from_Hex(hdIter->device.id, 4).toLower();
        device->className_ = from_CharArray(hdIter->base_class.name);
        device->vendorName_ = from_CharArray(hdIter->vendor.name);
        device->deviceName_ = from_CharArray(hdIter->device.name);
        device->sysfsBusID_ = from_CharArray(hdIter->sysfs_bus_id);
        device->sysfsID_ = from_CharArray(hdIter->sysfs_id);
        devices.emplace_back(device.release());
    }

    hd_free_hd_list(hd);
    hd_free_hd_data(hd_data.get());
}

void Data::fillAllConfigs(std::string type)
{
    std::vector<std::string> configPaths;
    std::vector<std::shared_ptr<Config>>* configs;

    if ("USB" == type)
    {
        configs = &allUSBConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_USB_CONFIG_DIR, MHWD_CONFIG_NAME);
    }
    else
    {
        configs = &allPCIConfigs;
        configPaths = getRecursiveDirectoryFileList(MHWD_PCI_CONFIG_DIR, MHWD_CONFIG_NAME);
    }

    for (auto&& configPath = configPaths.begin();
            configPath != configPaths.end(); ++configPath)
    {
        std::unique_ptr<Config> config{new Config((*configPath), type)};

        if (config->readConfigFile((*configPath)))
        {
            configs->emplace_back(config.release());
        }
        else
        {
            invalidConfigs.emplace_back(config.release());
        }
    }
}

std::vector<std::string> Data::getRecursiveDirectoryFileList(const std::string& directoryPath,
        std::string onlyFilename)
{
    std::vector<std::string> list;
    struct dirent *dir = nullptr;
    DIR* d = opendir(directoryPath.c_str());
    if (d)
    {
        while (nullptr != (dir = readdir(d)))
        {
            const std::string filename {dir->d_name};
            if (("." != filename) && (".." != filename) && ("" != filename))
            {
                std::string filepath {directoryPath + "/" + filename};
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

                    for (auto&& iterator = templist.begin();
                            iterator != templist.end(); iterator++)
                    {
                        list.push_back((*iterator));
                    }
                }
            }
        }

        closedir(d);
    }
    delete dir;
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

void Data::updateConfigData()
{
    for (auto& PCIDevice : PCIDevices)
    {
        PCIDevice->availableConfigs_.clear();
    }

    for (auto& USBDevice : USBDevices)
    {
        USBDevice->availableConfigs_.clear();
    }

    allPCIConfigs.clear();
    allUSBConfigs.clear();

    fillAllConfigs("PCI");
    fillAllConfigs("USB");

    setMatchingConfigs(PCIDevices, allPCIConfigs, false);
    setMatchingConfigs(USBDevices, allUSBConfigs, false);

    updateInstalledConfigData();
}

void Data::setMatchingConfigs(const std::vector<std::shared_ptr<Device>>& devices,
        std::vector<std::shared_ptr<Config>>& configs, bool setAsInstalled)
{
    for (auto& config : configs)
    {
        setMatchingConfig(config, devices, setAsInstalled);
    }
}

void Data::setMatchingConfig(std::shared_ptr<Config> config,
        const std::vector<std::shared_ptr<Device>>& devices, bool setAsInstalled)
{
    std::vector<std::shared_ptr<Device>> foundDevices;

    getAllDevicesOfConfig(devices, config, foundDevices);

    // Set config to all matching devices
    for (auto& foundDevice : foundDevices)
    {
        if (setAsInstalled)
        {
            addConfigSorted(foundDevice->installedConfigs_, config);
        }
        else
        {
            addConfigSorted(foundDevice->availableConfigs_, config);
        }
    }
}

void Data::addConfigSorted(std::vector<std::shared_ptr<Config>>& configs,
        std::shared_ptr<Config> newConfig)
{
    bool found = std::find_if(configs.begin(), configs.end(),
            [&newConfig](const std::shared_ptr<Config>& config)
            {
                return newConfig->name_ == config->name_;
            }) != configs.end();

    if (!found)
    {
        for (auto&& config = configs.begin(); config != configs.end(); ++config)
        {
            if (newConfig->priority_ > (*config)->priority_)
            {
                configs.insert(config, std::shared_ptr<Config>(newConfig));
                return;
            }
        }
        configs.emplace_back(newConfig);
    }
}

Vita::string Data::from_Hex(std::uint16_t hexnum, int fill)
{
    std::stringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(fill) << hexnum;
    return stream.str();
}

std::string Data::from_CharArray(char* c)
{
    if (nullptr == c)
    {
        return "";
    }

    return std::string(c);
}
