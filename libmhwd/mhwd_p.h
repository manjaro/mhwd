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
#include <algorithm>
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
    void fillDevices(mhwd::Data *data, mhwd::TYPE type);
    Vita::string from_Hex(uint16_t hexnum, int fill);
    int hexToInt(std::string hex);
    Vita::string from_CharArray(char* c);


    //###############//
    //### Configs ###//
    //###############//
    void fillInstalledConfigs(mhwd::Data *data, mhwd::TYPE type);
    void fillAllConfigs(mhwd::Data *data, mhwd::TYPE type);
    void getAllDevicesOfConfig(mhwd::Data *data, mhwd::Config *config, std::vector<mhwd::Device*>* foundDevices);
    void getAllDevicesOfConfig(std::vector<mhwd::Device*>* devices, mhwd::Config *config, std::vector<mhwd::Device*>* foundDevices);
    void setMatchingConfigs(std::vector<mhwd::Device*>* devices, std::vector<mhwd::Config*>* configs, bool setAsInstalled);
    void setMatchingConfig(mhwd::Config* config, std::vector<mhwd::Device*>* devices, bool setAsInstalled);
    void addConfigSorted(std::vector<mhwd::Config*>* configs, mhwd::Config* config);
    bool readConfigFile(mhwd::Config *config, std::string configPath);
    std::vector<std::string> splitValue(Vita::string str, Vita::string onlyEnding = "");
    Vita::string getRightConfigPath(Vita::string str, Vita::string baseConfigPath);

    void _getAllDependenciesToInstall(mhwd::Data *data, mhwd::Config *config, std::vector<mhwd::Config*>* installedConfigs, std::vector<mhwd::Config*> *depends);



    //#####################################//
    //### Directory and File Operations ###//
    //#####################################//
    std::vector<std::string> getRecursiveDirectoryFileList(const std::string directoryPath, std::string onlyFilename = "");
    bool copyDirectory(const std::string source, const std::string destination);
    bool copyFile(const std::string source, const std::string destination, const mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH);
    bool removeDirectory(const std::string directory);
    bool checkExist(const std::string path);
    bool createDir(const std::string path, const mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH | S_IXGRP | S_IXOTH);


    //###########################//
    //### Script & Operations ###//
    //###########################//
    void emitMessageFunc(mhwd::Data *data, mhwd::MESSAGETYPE type, std::string str);
    mhwd::STATUS installConfig(mhwd::Data *data, mhwd::Config *config);
    mhwd::STATUS uninstallConfig(mhwd::Data *data, mhwd::Config *config);
    bool runScript(mhwd::Data *data, mhwd::Config *config, mhwd::Transaction::TYPE operationType);
}


#endif // MHWD_P_H
