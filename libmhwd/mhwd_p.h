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

#ifndef MHWD_P_H
#define MHWD_P_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <hd.h>
#include "const.h"
#include "vita/string.hpp"


namespace mhwd {
    //###############//
    //### Devices ###//
    //###############//
    void setDevices(mhwd::Data *data, mhwd::TYPE type);
    Vita::string from_Hex(uint16_t hexnum, int fill);
    Vita::string from_CharArray(char* c);
    void addConfigSorted(std::vector<mhwd::Config>* configs, mhwd::Config* config);


    //###############//
    //### Configs ###//
    //###############//
    enum SCRIPTOPERATION { SCRIPTOPERATION_INSTALL, SCRIPTOPERATION_REMOVE };

    bool checkDependenciesConflicts(mhwd::Data *data, mhwd::Config *config);
    void updateInstalledConfigData(mhwd::Data *data);
    void setInstalledConfigs(mhwd::Data *data, mhwd::TYPE type);
    void setMatchingConfigs(mhwd::Data *data, std::vector<mhwd::Device>* devices, mhwd::TYPE type, bool setAsInstalled = false);
    void setMatchingConfigs(std::vector<mhwd::Device>* devices, std::vector<mhwd::Config>* configs, bool setAsInstalled = false);
    void setMatchingConfig(mhwd::Config* config, std::vector<mhwd::Device>* devices, bool setAsInstalled = false);
    bool fillConfig(mhwd::Config *config, std::string configPath, mhwd::TYPE type);
    bool readConfigFile(mhwd::Config *config, std::string configPath);
    std::vector<std::string> splitValue(Vita::string str, Vita::string onlyEnding = "");
    Vita::string getRightConfigPath(Vita::string str, Vita::string baseConfigPath);


    //#####################################//
    //### Directory and File Operations ###//
    //#####################################//
    std::vector<std::string> getRecursiveDirectoryFileList(const std::string directoryPath, std::string onlyFilename = "");
    bool copyDirectory(const std::string source, const std::string destination);
    bool copyFile(const std::string source, const std::string destination);
    bool removeDirectory(const std::string directory);


    //##############//
    //### Script ###//
    //##############//
    bool runScript(mhwd::Data *data, mhwd::Config *config, SCRIPTOPERATION scriptOperation);
}


#endif // MHWD_P_H
