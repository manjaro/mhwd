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

#ifndef MHWD_H
#define MHWD_H

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>


namespace mhwd {
    //#############//
    //### Enums ###//
    //#############//
    enum TYPE { TYPE_PCI, TYPE_USB };
    enum STATUS { STATUS_SUCCESS, STATUS_ERROR_CONFLICTS, STATUS_ERROR_REQUIREMENTS, STATUS_ERROR_NOT_INSTALLED, STATUS_ERROR_ALREADY_INSTALLED, STATUS_ERROR_NO_MATCH_LOCAL_CONFIG, STATUS_ERROR_SCRIPT_FAILED, STATUS_ERROR_SET_DATABASE };
    enum MESSAGETYPE { MESSAGETYPE_CONSOLE_OUTPUT, MESSAGETYPE_INSTALLDEPENDENCY_START, MESSAGETYPE_INSTALLDEPENDENCY_END, MESSAGETYPE_INSTALL_START, MESSAGETYPE_INSTALL_END, MESSAGETYPE_REMOVE_START, MESSAGETYPE_REMOVE_END };


    //####################//
    //### Data Structs ###//
    //####################//
    struct Config {
        struct HardwareIDs {
            std::vector<std::string> classIDs, vendorIDs, deviceIDs;
            std::vector<std::string> blacklistedClassIDs, blacklistedVendorIDs, blacklistedDeviceIDs;
        };

        TYPE type;
        std::string basePath, configPath, name, info, version;
        bool freedriver;
        int priority;
        std::vector<HardwareIDs> hwdIDs;
        std::vector<std::string> conflicts, dependencies;
    };

    struct Device {
        TYPE type;
        std::string className, deviceName, vendorName, classID, deviceID, vendorID, sysfsBusID, sysfsID;
        std::vector<mhwd::Config*> availableConfigs, installedConfigs;
    };

    struct Data {
        struct Environment {
            std::string PMCachePath, PMConfigPath, PMRootPath;
            bool syncPackageManagerDatabase;
            void (*messageFunc)(mhwd::MESSAGETYPE, std::string);
        };

        std::vector<mhwd::Device*> USBDevices, PCIDevices;
        std::vector<mhwd::Config*> installedUSBConfigs, installedPCIConfigs, allUSBConfigs, allPCIConfigs, invalidConfigs;
        Environment environment;
    };

    struct Transaction {
        enum TYPE { TYPE_INSTALL, TYPE_REMOVE };

        TYPE type;
        bool allowReinstallation;
        mhwd::Config* config;
        std::vector<mhwd::Config*> dependencyConfigs, conflictedConfigs, requiredByConfigs;
    };


    //#################//
    //### Functions ###//
    //#################//
    void initData(mhwd::Data *data);
    void fillData(mhwd::Data *data);
    void freeData(mhwd::Data *data);
    std::string checkEnvironment();
    void updateConfigData(mhwd::Data *data);
    void updateInstalledConfigData(mhwd::Data *data);
    bool fillConfig(mhwd::Config *config, std::string configPath, mhwd::TYPE type);

    void printDeviceDetails(mhwd::TYPE type, FILE *f = stdout);

    mhwd::Config* getInstalledConfig(mhwd::Data *data, const std::string configName, const TYPE configType);
    mhwd::Config* getDatabaseConfig(mhwd::Data *data, const std::string configName, const TYPE configType);
    mhwd::Config* getAvailableConfig(mhwd::Data *data, const std::string configName, const TYPE configType);

    std::vector<mhwd::Config*> getAllDependenciesToInstall(mhwd::Data *data, mhwd::Config *config);
    std::vector<mhwd::Config*> getAllLocalConflicts(mhwd::Data *data, mhwd::Config *config);
    std::vector<mhwd::Config*> getAllLocalRequirements(mhwd::Data *data, mhwd::Config *config);

    mhwd::Transaction createTransaction(mhwd::Data *data, mhwd::Config* config, mhwd::Transaction::TYPE type, bool allowReinstallation = false);
    mhwd::STATUS performTransaction(mhwd::Data *data, mhwd::Transaction *transaction);
}


#else


namespace mhwd {
    extern void initData(mhwd::Data *data);
    extern void fillData(mhwd::Data *data);
    extern void freeData(mhwd::Data *data);
    extern std::string checkEnvironment();
    extern void updateConfigData(mhwd::Data *data);
    extern void updateInstalledConfigData(mhwd::Data *data);

    extern void printDeviceDetails(mhwd::TYPE type, FILE *f = stdout);

    extern mhwd::Config* getInstalledConfig(mhwd::Data *data, const std::string configName, const TYPE configType);
    extern mhwd::Config* getDatabaseConfig(mhwd::Data *data, const std::string configName, const TYPE configType);
    extern mhwd::Config* getAvailableConfig(mhwd::Data *data, const std::string configName, const TYPE configType);

    extern std::vector<mhwd::Config*> getAllDependenciesToInstall(mhwd::Data *data, mhwd::Config *config);
    extern std::vector<mhwd::Config*> getAllLocalConflicts(mhwd::Data *data, mhwd::Config *config);
    extern std::vector<mhwd::Config*> getAllLocalRequirements(mhwd::Data *data, mhwd::Config *config);

    extern mhwd::Transaction createTransaction(mhwd::Data *data, mhwd::Config* config, mhwd::Transaction::TYPE type, bool allowReinstallation = false);
    extern mhwd::STATUS performTransaction(mhwd::Data *data, mhwd::Transaction *transaction);
}


#endif // MHWD_H
