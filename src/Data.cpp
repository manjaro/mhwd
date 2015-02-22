/*
 *  mhwd - Manjaro Hardware Detection
 *  Roland Singer <roland@manjaro.org>
 *  Łukasz Matysiak <december0123@gmail.com>
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

#include <dirent.h>

#include <algorithm>
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
}

void Data::updateInstalledConfigData()
{
    // Clear config vectors in each device element
    for (auto&& PCIDevice = PCIDevices.begin();
            PCIDevice != PCIDevices.end(); ++PCIDevice)
    {
        (*PCIDevice)->installedConfigs_.clear();
    }

    for (auto&& USBDevice = USBDevices.begin();
            USBDevice != USBDevices.end(); ++USBDevice)
    {
        (*USBDevice)->installedConfigs_.clear();
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

    for (auto&& configPath = configPaths.begin();
            configPath != configPaths.end(); ++configPath)
    {
        Config *config = new Config((*configPath), type);

        if (config->readConfigFile((*configPath)))
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
            bool found = false;
            // Check class ids
            for (auto&& classID = hwdID->classIDs.begin();
                    classID != hwdID->classIDs.end(); ++classID)
            {
                if (*classID == "*" || *classID == (*i_device)->classID_)
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

                for (auto&& blacklistedClassID =
                        (*hwdID).blacklistedClassIDs.begin();
                        blacklistedClassID != (*hwdID).blacklistedClassIDs.end();
                        ++blacklistedClassID)
                {
                    if (*blacklistedClassID == (*i_device)->classID_)
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

                    for (auto&& vendorID = hwdID->vendorIDs.begin();
                            vendorID != hwdID->vendorIDs.end(); ++vendorID)
                    {
                        if (("*" == *vendorID) || (*vendorID == (*i_device)->vendorID_))
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

                        for (auto&& blacklistedVendorID =
                                hwdID->blacklistedVendorIDs.begin();
                                blacklistedVendorID != hwdID->blacklistedVendorIDs.end();
                                ++blacklistedVendorID)
                        {
                            if (*blacklistedVendorID == (*i_device)->vendorID_)
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

                            for (auto&& deviceID = hwdID->deviceIDs.begin();
                                    deviceID != hwdID->deviceIDs.end(); ++deviceID)
                            {
                                if (("*" == *deviceID) || (*deviceID == (*i_device)->deviceID_))
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

                                for (auto&& blacklistedDeviceID =
                                        hwdID->blacklistedDeviceIDs.begin();
                                        blacklistedDeviceID != hwdID->blacklistedDeviceIDs.end();
                                        ++blacklistedDeviceID)
                                {
                                    if (*blacklistedDeviceID == (*i_device)->deviceID_)
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

    // Get the right configs
    if ("USB" == config->type_)
    {
        installedConfigs = installedUSBConfigs;
    }
    else
    {
        installedConfigs = installedPCIConfigs;
    }

    // Get all depends
    getAllDependenciesToInstall(config, installedConfigs, &depends);

    return depends;
}

void Data::getAllDependenciesToInstall(std::shared_ptr<Config> config,
        std::vector<std::shared_ptr<Config>>& installedConfigs,
        std::vector<std::shared_ptr<Config>> *dependencies)
{
    for (auto&& configDependency = config->dependencies_.begin();
            configDependency != config->dependencies_.end(); ++configDependency)
    {
        auto found = std::find_if(installedConfigs.begin(), installedConfigs.end(),
                [configDependency](const std::shared_ptr<Config>& rhs) -> bool {
                    return (rhs->name_ == *configDependency);
                });

        if (found != installedConfigs.end())
        {
            continue;
        }
        else
        {
            found = std::find_if(dependencies->begin(), dependencies->end(),
                    [configDependency](const std::shared_ptr<Config>& rhs) -> bool {
                        return (rhs->name_ == *configDependency);
                    });

            if (found != dependencies->end())
            {
                continue;
            }
            else
            {
                // Add to vector and check for further subdepends...
                std::shared_ptr<Config> dependconfig {
                	getDatabaseConfig((*configDependency), config->type_)};
                if (nullptr == dependconfig)
                {
                    continue;
                }
                else
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

    // Get the right configs
    if ("USB" == configType)
    {
        allConfigs = allUSBConfigs;
    }
    else
    {
        allConfigs = allPCIConfigs;
    }

    for (auto&& config = allConfigs.begin(); config != allConfigs.end();
            ++config)
    {
        if (configName == (*config)->name_)
        {
            return (*config);
        }
    }

    return nullptr;
}

std::vector<std::shared_ptr<Config>> Data::getAllLocalConflicts(std::shared_ptr<Config> config)
{
    std::vector<std::shared_ptr<Config>> conflicts;
    std::vector<std::shared_ptr<Config>> dependencies = getAllDependenciesToInstall(config);
    std::vector<std::shared_ptr<Config>> installedConfigs;

    // Get the right configs
    if ("USB" == config->type_)
    {
        installedConfigs = installedUSBConfigs;
    }
    else
    {
        installedConfigs = installedPCIConfigs;
    }

    dependencies.emplace_back(config);

    for (auto&& dependency = dependencies.begin();
            dependency != dependencies.end(); ++dependency)
    {
        for (auto&& dependencyConflict = (*dependency)->conflicts_.begin();
                dependencyConflict != (*dependency)->conflicts_.end(); ++dependencyConflict)
        {
            for (auto&& installedConfig = installedConfigs.begin();
                    installedConfig != installedConfigs.end(); ++installedConfig)
            {
                if ((*dependencyConflict) != (*installedConfig)->name_)
                {
                    continue;
                }
                else
                {
                    // Check if already in vector
                    bool found = false;
                    for (auto&& conflict = conflicts.begin();
                            conflict != conflicts.end(); ++conflict)
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
                        conflicts.emplace_back(*installedConfig);
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

    // Get the right configs
    if ("USB" == config->type_)
    {
        installedConfigs = installedUSBConfigs;
    }
    else
    {
        installedConfigs = installedPCIConfigs;
    }

    // Check if this config is required by another installed config
    for (auto&& installedConfig = installedConfigs.begin();
            installedConfig != installedConfigs.end(); ++installedConfig)
    {
        for (auto&& dependency = (*installedConfig)->dependencies_.begin();
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
                for (auto&& requirement = requirements.begin();
                        requirement != requirements.end(); ++requirement)
                {
                    if ((*requirement)->name_ == (*installedConfig)->name_)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    requirements.emplace_back(*installedConfig);
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
    std::vector<std::shared_ptr<Device>>* devices;

    if ("USB" == type)
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
    std::unique_ptr<hd_data_t> hd_data{new hd_data_t()};
    hd_t *hd = hd_list(hd_data.get(), hw, 1, nullptr);

    std::unique_ptr<Device> device;
    for (hd_t *hdIter = hd; hdIter; hdIter = hdIter->next)
    {
        device.reset(new Device());
        device->type_ = type;
        device->classID_ = from_Hex(hdIter->base_class.id, 2) + from_Hex(hdIter->sub_class.id, 2).toLower();
        device->vendorID_ = from_Hex(hdIter->vendor.id, 4).toLower();
        device->deviceID_ = from_Hex(hdIter->device.id, 4).toLower();
        device->className_ = from_CharArray(hdIter->base_class.name);
        device->vendorName_ = from_CharArray(hdIter->vendor.name);
        device->deviceName_ = from_CharArray(hdIter->device.name);
        device->sysfsBusID_ = from_CharArray(hdIter->sysfs_bus_id);
        device->sysfsID_ = from_CharArray(hdIter->sysfs_id);
        devices->emplace_back(device.release());
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

bool Data::fillConfig(std::shared_ptr<Config> config, std::string configPath, std::string type)
{
    config->type_ = type;
    config->priority_ = 0;
    config->freedriver_ = true;
    config->basePath_ = configPath.substr(0, configPath.find_last_of('/'));
    config->configPath_ = configPath;

    // Add new HardwareIDs group to vector if vector is empty
    if (config->hwdIDs_.empty())
    {
        config->hwdIDs_.emplace_back();
    }

    return config->readConfigFile(config->configPath_);
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
            std::string filename = dir->d_name;
            std::string filepath = directoryPath + "/" + filename;

            if (("." == filename) || (".." == filename) || ("" == filename))
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

std::vector<std::string> Data::splitValue(Vita::string str, Vita::string onlyEnding)
{
    std::vector<Vita::string> work = str.toLower().explode(" ");
    std::vector<std::string> final;

    for (auto&& iterator = work.begin(); iterator != work.end();
            iterator++)
    {
        if (("" != *iterator) && onlyEnding.empty())
        {
            final.push_back(*iterator);
        }
        else if (("" != *iterator) && (Vita::string(*iterator).explode(".").back() == onlyEnding)
                && ((*iterator).size() > 5))
        {
            final.push_back(Vita::string(*iterator).substr(0, (*iterator).size() - 5));
        }
    }

    return final;
}

void Data::updateConfigData()
{
    // Clear config vectors in each device element
    for (auto&& PCIDevice = PCIDevices.begin();
            PCIDevice != PCIDevices.end(); PCIDevice++)
    {
        (*PCIDevice)->availableConfigs_.clear();
    }

    for (auto&& USBDevice = USBDevices.begin();
            USBDevice != USBDevices.end(); USBDevice++)
    {
        (*USBDevice)->availableConfigs_.clear();
    }
    allPCIConfigs.clear();
    allUSBConfigs.clear();

    // Refill data
    fillAllConfigs("PCI");
    fillAllConfigs("USB");

    setMatchingConfigs(PCIDevices, allPCIConfigs, false);
    setMatchingConfigs(USBDevices, allUSBConfigs, false);

    // Update also installed config data
    updateInstalledConfigData();
}

void Data::setMatchingConfigs(const std::vector<std::shared_ptr<Device>>& devices,
        std::vector<std::shared_ptr<Config>>& configs, bool setAsInstalled)
{
    for (auto&& config = configs.begin(); config != configs.end();
            ++config)
    {
        setMatchingConfig((*config), devices, setAsInstalled);
    }
}

void Data::setMatchingConfig(std::shared_ptr<Config> config,
		const std::vector<std::shared_ptr<Device>>& devices, bool setAsInstalled)
{
    std::vector<std::shared_ptr<Device>> foundDevices;

    getAllDevicesOfConfig(devices, config, foundDevices);

    // Set config to all matching devices
    for (auto&& foundDevice = foundDevices.begin();
            foundDevice != foundDevices.end(); ++foundDevice)
    {
        if (setAsInstalled)
        {
            addConfigSorted((*foundDevice)->installedConfigs_, config);
        }
        else
        {
            addConfigSorted((*foundDevice)->availableConfigs_, config);
        }
    }
}

void Data::addConfigSorted(std::vector<std::shared_ptr<Config>>& configs,
		std::shared_ptr<Config> config)
{
    for (auto&& iterator = configs.begin();
            iterator != configs.end(); iterator++)
    {
        if (config->name_ == (*iterator)->name_)
        {
            return;
        }
    }

    for (auto&& iterator = configs.begin(); iterator != configs.end();
            iterator++)
    {
        if (config->priority_ > (*iterator)->priority_)
        {
            configs.insert(iterator, std::shared_ptr<Config>(config));
            return;
        }
    }

    configs.emplace_back(config);
}

Vita::string Data::from_Hex(std::uint16_t hexnum, int fill)
{
    std::stringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(fill) << hexnum;
    return stream.str();
}

Vita::string Data::from_CharArray(char* c)
{
    if (nullptr == c)
    {
        return "";
    }

    return Vita::string(c);
}
