/*
 *  This file is part of the mhwd - Manjaro Hardware Detection project
 *  
 *  mhwd - Manjaro Hardware Detection
 *  Roland Singer <roland@manjaro.org>
 *  Łukasz Matysiak <december0123@gmail.com>
 * 	Filipe Marques <eagle.software3@gmail.com>
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

#ifndef DATA_HPP_
#define DATA_HPP_

#include <hd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <memory>
#include <string>
#include <vector>

#include "Config.hpp"
#include "Device.hpp"
#include "const.h"
#include "vita/string.hpp"

class Data {
public:
    Data();
    ~Data();

    struct Environment {
            std::string PMCachePath {MHWD_PM_CACHE_DIR};
            std::string PMConfigPath {MHWD_PM_CONFIG};
            std::string PMRootPath {MHWD_PM_ROOT};
            bool syncPackageManagerDatabase = true;
    };

    Environment environment;
    std::vector<std::shared_ptr<Device>> USBDevices;
    std::vector<std::shared_ptr<Device>> PCIDevices;
    std::vector<std::shared_ptr<Config>> installedUSBConfigs;
    std::vector<std::shared_ptr<Config>> installedPCIConfigs;
    std::vector<std::shared_ptr<Config>> allUSBConfigs;
    std::vector<std::shared_ptr<Config>> allPCIConfigs;
    std::vector<std::shared_ptr<Config>> invalidConfigs;

    void updateInstalledConfigData();
    void getAllDevicesOfConfig(std::shared_ptr<Config> config, std::vector<std::shared_ptr<Device>>& foundDevices);
    bool fillConfig(std::shared_ptr<Config> config, std::string configPath, std::string type);

    std::vector<std::shared_ptr<Config>> getAllDependenciesToInstall(std::shared_ptr<Config> config);
    void getAllDependenciesToInstall(std::shared_ptr<Config> config,
            std::vector<std::shared_ptr<Config>>& installedConfigs,
            std::vector<std::shared_ptr<Config>> *depends);
    std::shared_ptr<Config> getDatabaseConfig(const std::string configName,
            const std::string configType);
    std::vector<std::shared_ptr<Config>> getAllLocalConflicts(std::shared_ptr<Config> config);
    std::vector<std::shared_ptr<Config>> getAllLocalRequirements(std::shared_ptr<Config> config);

private:
    void getAllDevicesOfConfig(const std::vector<std::shared_ptr<Device>>& devices,
            std::shared_ptr<Config> config, std::vector<std::shared_ptr<Device>>& foundDevices);
    void fillInstalledConfigs(std::string type);
    void fillDevices(std::string type);
    void fillAllConfigs(std::string type);
    void setMatchingConfigs(const std::vector<std::shared_ptr<Device>>& devices,
            std::vector<std::shared_ptr<Config>>& configs, bool setAsInstalled);
    void setMatchingConfig(std::shared_ptr<Config> config, const std::vector<std::shared_ptr<Device>>& devices,
            bool setAsInstalled);
    void addConfigSorted(std::vector<std::shared_ptr<Config>>& configs, std::shared_ptr<Config> config);
    std::vector<std::string> getRecursiveDirectoryFileList(const std::string& directoryPath,
            std::string onlyFilename = "");

    Vita::string getRightConfigPath(Vita::string str, Vita::string baseConfigPath);
    std::vector<std::string> splitValue(Vita::string str, Vita::string onlyEnding = "");
    void updateConfigData();

    Vita::string from_Hex(uint16_t hexnum, int fill);
    std::string from_CharArray(char* c);
};

#endif /* DATA_HPP_ */
