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

#ifndef MHWD_HPP_
#define MHWD_HPP_

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "Config.hpp"
#include "const.h"
#include "Data.hpp"
#include "Device.hpp"
#include "Enums.hpp"
#include "Printer.hpp"
#include "vita/string.hpp"
#include "Transaction.hpp"

class Mhwd
{
public:
    Mhwd();
    ~Mhwd() = default;
	void set_version_mhwd(std::string version_of_software, std::string year_copyright);
    int launch(int argc, char *argv[]);

private:
    struct Arguments
    {
        bool SHOW_PCI = false;
        bool SHOW_USB = false;
        bool INSTALL = false;
        bool REMOVE = false;
        bool DETAIL = false;
        bool FORCE = false;
        bool LIST_ALL = false;
        bool LIST_INSTALLED = false;
        bool LIST_AVAILABLE = false;
        bool LIST_HARDWARE = false;
        bool CUSTOM_INSTALL = false;
        bool AUTOCONFIGURE = false;
    } arguments_;
    std::shared_ptr<Config> config_;
    Data data_;
    Printer printer_;
    std::vector<std::string> configs_;

	std::string version_, year_;
	
    bool performTransaction(std::shared_ptr<Config> config, MHWD::TRANSACTIONTYPE type);
    bool isUserRoot() const;
    std::string checkEnvironment();

    void printDeviceDetails(std::string type, FILE *f = stdout);

    std::shared_ptr<Config> getInstalledConfig(const std::string& configName, const std::string& configType);
    std::shared_ptr<Config> getDatabaseConfig(const std::string& configName, const std::string& configType);
    std::shared_ptr<Config> getAvailableConfig(const std::string& configName, const std::string& configType);

    std::vector<Config*> getAllLocalRequirements(Config *config);

    MHWD::STATUS performTransaction(const Transaction& transaction);
    bool proceedWithInstallation(const std::string& input) const;

    bool copyDirectory(const std::string& source, const std::string& destination);
    bool copyFile(const std::string& source, const std::string destination, const mode_t mode =
            S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH);
    bool removeDirectory(const std::string& directory);
    bool dirExists(const std::string& path);
    bool createDir(const std::string& path, const mode_t mode =
            S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH | S_IXGRP | S_IXOTH);

    MHWD::STATUS installConfig(std::shared_ptr<Config> config);
    MHWD::STATUS uninstallConfig(Config *config);
    bool runScript(std::shared_ptr<Config> config, MHWD::TRANSACTIONTYPE operationType);
	void tryToParseCmdLineOptions(int argc, char* argv[], bool& autoConfigureNonFreeDriver,
			std::string& operationType, std::string& autoConfigureClassID);
	bool optionsDontInterfereWithEachOther() const;
};

#endif /* MHWD_HPP_ */
