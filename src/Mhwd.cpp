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

#include "Mhwd.hpp"
#include "vita/string.hpp"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

Mhwd::Mhwd() : arguments_(), data_(), printer_()
{
}

bool Mhwd::performTransaction(std::shared_ptr<Config> config, MHWD::TRANSACTIONTYPE transactionType)
{
    Transaction transaction (data_, config, transactionType,
            arguments_.FORCE);

    // Print things to do
    if (MHWD::TRANSACTIONTYPE::INSTALL == transactionType)
    {
        // Print conflicts
        if (!transaction.conflictedConfigs_.empty())
        {
            std::string conflicts;

            for (auto&& conflictedConfig : transaction.conflictedConfigs_)
            {
                conflicts += " " + conflictedConfig->name_;
            }

            printer_.printError("config '" + config->name_ + "' conflicts with config(s):" +
                    conflicts);
            return false;
        }

        // Print dependencies
        else if (!transaction.dependencyConfigs_.empty())
        {
            std::string dependencies;

            for (auto&& dependencyConfig : transaction.dependencyConfigs_)
            {
                dependencies += " " + dependencyConfig->name_;
            }

            printer_.printStatus("Dependencies to install:" + dependencies +
                    "\nProceed with installation? [Y/n]");
            std::string input;
            std::getline(std::cin, input);
            return proceedWithInstallation(input);
        }
    }
    else if (MHWD::TRANSACTIONTYPE::REMOVE == transactionType)
    {
        // Print requirements
        if (!transaction.configsRequirements_.empty())
        {
            std::string requirements;

            for (auto&& requirement : transaction.configsRequirements_)
            {
                requirements += " " + requirement->name_;
            }

            printer_.printError("config '" + config->name_ + "' is required by config(s):" +
                    requirements);
            return false;
        }
    }

    MHWD::STATUS status = performTransaction(transaction);

    switch (status)
    {
    	case MHWD::STATUS::SUCCESS:
    		break;
    	case MHWD::STATUS::ERROR_CONFLICTS:
    		printer_.printError("config '" + config->name_ +
    				"' conflicts with installed config(s)!");
    		break;
    	case MHWD::STATUS::ERROR_REQUIREMENTS:
    		printer_.printError("config '" + config->name_ +
    				"' is required by installed config(s)!");
    		break;
    	case MHWD::STATUS::ERROR_NOT_INSTALLED:
    		printer_.printError("config '" + config->name_ + "' is not installed!");
    		break;
    	case MHWD::STATUS::ERROR_ALREADY_INSTALLED:
            printer_.printWarning("a version of config '" + config->name_ +
					"' is already installed!\nUse -f/--force to force installation...");
            break;
    	case MHWD::STATUS::ERROR_NO_MATCH_LOCAL_CONFIG:
    		printer_.printError("passed config does not match with installed config!");
    		break;
    	case MHWD::STATUS::ERROR_SCRIPT_FAILED:
    		printer_.printError("script failed!");
    		break;
    	case MHWD::STATUS::ERROR_SET_DATABASE:
    		printer_.printError("failed to set database!");
    		break;
    }

    data_.updateInstalledConfigData();

    return (MHWD::STATUS::SUCCESS == status);
}

bool Mhwd::proceedWithInstallation(const std::string& input) const
{
    if ((input.length() == 1) && (('y' == input[0]) || ('Y' == input[0])))
    {
        return true;
    }
    else if (0 == input.length())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Mhwd::isUserRoot() const
{
    constexpr unsigned short ROOT_UID = 0;
    if (ROOT_UID != getuid())
    {
        return false;
    }
    return true;
}

std::string Mhwd::checkEnvironment()
{
    std::string missingDir;

    // Check if required directories exists. Otherwise return missing directory...
    if (!dirExists(MHWD_USB_CONFIG_DIR))
    {
        missingDir = MHWD_USB_CONFIG_DIR;
    }
    if (!dirExists(MHWD_PCI_CONFIG_DIR))
    {
        missingDir = MHWD_PCI_CONFIG_DIR;
    }
    if (!dirExists(MHWD_USB_DATABASE_DIR))
    {
        missingDir = MHWD_USB_DATABASE_DIR;
    }
    if (!dirExists(MHWD_PCI_DATABASE_DIR))
    {
        missingDir = MHWD_PCI_DATABASE_DIR;
    }

    return missingDir;
}

void Mhwd::printDeviceDetails(std::string type, FILE *f)
{
    hw_item hw;
    if ("USB" == type)
    {
        hw = hw_usb;
    }
    else
    {
        hw = hw_pci;
    }

    std::unique_ptr<hd_data_t> hd_data{new hd_data_t()};
    hd_t *hd = hd_list(hd_data.get(), hw, 1, nullptr);
    hd_t *beginningOfhd = hd;

    for (; hd; hd = hd->next)
    {
        hd_dump_entry(hd_data.get(), hd, f);
    }

    hd_free_hd_list(beginningOfhd);
    hd_free_hd_data(hd_data.get());
}

std::shared_ptr<Config> Mhwd::getInstalledConfig(const std::string& configName,
        const std::string& configType)
{
    std::vector<std::shared_ptr<Config>>* installedConfigs;

    // Get the right configs
    if ("USB" == configType)
    {
        installedConfigs = &data_.installedUSBConfigs;
    }
    else
    {
        installedConfigs = &data_.installedPCIConfigs;
    }

    for (auto&& installedConfig = installedConfigs->begin();
            installedConfig != installedConfigs->end(); installedConfig++)
    {
        if (configName == (*installedConfig)->name_)
        {
            return (*installedConfig);
        }
    }

    return nullptr;
}

std::shared_ptr<Config> Mhwd::getDatabaseConfig(const std::string& configName,
        const std::string& configType)
{
    std::vector<std::shared_ptr<Config>>* allConfigs;

    // Get the right configs
    if ("USB" == configType)
    {
        allConfigs = &data_.allUSBConfigs;
    }
    else
    {
        allConfigs = &data_.allPCIConfigs;
    }

    for (auto&& iterator = allConfigs->begin();
            iterator != allConfigs->end(); ++iterator)
    {
        if (configName == (*iterator)->name_)
        {
            return (*iterator);
        }
    }

    return nullptr;
}

std::shared_ptr<Config> Mhwd::getAvailableConfig(const std::string& configName,
        const std::string& configType)
{
    std::vector<std::shared_ptr<Device>> *devices;

    // Get the right devices
    if ("USB" == configType)
    {
        devices = &data_.USBDevices;
    }
    else
    {
        devices = &data_.PCIDevices;
    }

    for (auto&& device = devices->begin(); device != devices->end();
            ++device)
    {
        if ((*device)->availableConfigs_.empty())
        {
            continue;
        }
        else
        {
            for (auto&& availableConfig = (*device)->availableConfigs_.begin();
                    availableConfig != (*device)->availableConfigs_.end(); availableConfig++)
            {
                if (configName == (*availableConfig)->name_)
                {
                    return (*availableConfig);
                }
            }
        }
    }
    return nullptr;
}

MHWD::STATUS Mhwd::performTransaction(const Transaction& transaction)
{
    if ((MHWD::TRANSACTIONTYPE::INSTALL == transaction.type_) &&
            !transaction.conflictedConfigs_.empty())
    {
        return MHWD::STATUS::ERROR_CONFLICTS;
    }
    else if ((MHWD::TRANSACTIONTYPE::REMOVE == transaction.type_)
            && !transaction.configsRequirements_.empty())
    {
        return MHWD::STATUS::ERROR_REQUIREMENTS;
    }
    else
    {
        // Check if already installed
        std::shared_ptr<Config> installedConfig{getInstalledConfig(transaction.config_->name_,
                transaction.config_->type_)};
        MHWD::STATUS status = MHWD::STATUS::SUCCESS;

        if ((MHWD::TRANSACTIONTYPE::REMOVE == transaction.type_)
                || (installedConfig != nullptr && transaction.isAllowedToReinstall()))
        {
            if (nullptr == installedConfig)
            {
                return MHWD::STATUS::ERROR_NOT_INSTALLED;
            }
            else
            {
                printer_.printMessage(MHWD::MESSAGETYPE::REMOVE_START, installedConfig->name_);
                if (MHWD::STATUS::SUCCESS != (status = uninstallConfig(installedConfig.get())))
                {
                    return status;
                }
                else
                {
                    printer_.printMessage(MHWD::MESSAGETYPE::REMOVE_END, installedConfig->name_);
                }
            }
        }

        if (MHWD::TRANSACTIONTYPE::INSTALL == transaction.type_)
        {
            // Check if already installed but not allowed to reinstall
            if ((nullptr != installedConfig) && !transaction.isAllowedToReinstall())
            {
                return MHWD::STATUS::ERROR_ALREADY_INSTALLED;
            }
            else
            {
                // Install all dependencies first
                for (auto&& dependencyConfig = transaction.dependencyConfigs_.end() - 1;
                        dependencyConfig != transaction.dependencyConfigs_.begin() - 1;
                        --dependencyConfig)
                {
                    printer_.printMessage(MHWD::MESSAGETYPE::INSTALLDEPENDENCY_START,
                            (*dependencyConfig)->name_);
                    if (MHWD::STATUS::SUCCESS != (status = installConfig((*dependencyConfig))))
                    {
                        return status;
                    }
                    else
                    {
                        printer_.printMessage(MHWD::MESSAGETYPE::INSTALLDEPENDENCY_END,
                                (*dependencyConfig)->name_);
                    }
                }

                printer_.printMessage(MHWD::MESSAGETYPE::INSTALL_START, transaction.config_->name_);
                if (MHWD::STATUS::SUCCESS != (status = installConfig(transaction.config_)))
                {
                    return status;
                }
                else
                {
                    printer_.printMessage(MHWD::MESSAGETYPE::INSTALL_END,
                            transaction.config_->name_);
                }
            }
        }
        return status;
    }
}

bool Mhwd::copyDirectory(const std::string& source, const std::string& destination)
{
    struct stat filestatus;

    if (0 != lstat(destination.c_str(), &filestatus))
    {
        if (!createDir(destination))
        {
            return false;
        }
    }
    else if (S_ISREG(filestatus.st_mode))
    {
        return false;
    }
    else if (S_ISDIR(filestatus.st_mode))
    {
        if (!removeDirectory(destination))
        {
            return false;
        }

        if (!createDir(destination))
        {
            return false;
        }
    }
    struct dirent *dir;
    DIR *d = opendir(source.c_str());

    if (!d)
    {
        return false;
    }
    else
    {
        bool success = true;
        while ((dir = readdir(d)) != nullptr)
        {
            std::string filename {dir->d_name};
            std::string sourcePath {source + "/" + filename};
            std::string destinationPath {destination + "/" + filename};

            if (("." == filename) || (".." == filename) || ("" == filename))
            {
                continue;
            }
            else
            {
                lstat(sourcePath.c_str(), &filestatus);

                if (S_ISREG(filestatus.st_mode))
                {
                    if (!copyFile(sourcePath, destinationPath))
                    {
                        success = false;
                    }
                }
                else if (S_ISDIR(filestatus.st_mode))
                {
                    if (!copyDirectory(sourcePath, destinationPath))
                    {
                        success = false;
                    }
                }
            }
        }
        closedir(d);
        return success;
    }
}

bool Mhwd::copyFile(const std::string& source, const std::string destination, const mode_t mode)
{
    std::ifstream src(source, std::ios::binary);
    std::ofstream dst(destination, std::ios::binary);
    if (!src || !dst)
    {
    	return false;
    }

    dst << src.rdbuf();
    mode_t process_mask = umask(0);
    chmod(destination.c_str(), mode);
    umask(process_mask);
    return true;
}

bool Mhwd::removeDirectory(const std::string& directory)
{
    DIR *d = opendir(directory.c_str());

    if (!d)
    {
        return false;
    }
    else
    {
        bool success = true;
        struct dirent *dir;
        while ((dir = readdir(d)) != nullptr)
        {
            std::string filename = std::string(dir->d_name);
            std::string filepath = directory + "/" + filename;

            if (("." == filename) || (".." == filename) || ("" == filename))
            {
                continue;
            }
            else
            {
                struct stat filestatus;
                lstat(filepath.c_str(), &filestatus);

                if (S_ISREG(filestatus.st_mode))
                {
                    if (0 != unlink(filepath.c_str()))
                    {
                        success = false;
                    }
                }
                else if (S_ISDIR(filestatus.st_mode))
                {
                    if (!removeDirectory(filepath))
                    {
                        success = false;
                    }
                }
            }
        }
        closedir(d);

        if (0 != rmdir(directory.c_str()))
        {
            success = false;
        }
        return success;
    }
}

bool Mhwd::dirExists(const std::string& path)
{
    struct stat filestatus;
    if (0 != stat(path.c_str(), &filestatus))
    {
        return false;
    }

    return true;
}

bool Mhwd::createDir(const std::string& path, const mode_t mode)
{
    mode_t process_mask = umask(0);
    int ret = mkdir(path.c_str(), mode);
    umask(process_mask);

    constexpr unsigned short SUCCESS = 0;
    return (SUCCESS == ret);
}

MHWD::STATUS Mhwd::installConfig(std::shared_ptr<Config> config)
{
    std::string databaseDir;
    if ("USB" == config->type_)
    {
        databaseDir = MHWD_USB_DATABASE_DIR;
    }
    else
    {
        databaseDir = MHWD_PCI_DATABASE_DIR;
    }

    if (!runScript(config, MHWD::TRANSACTIONTYPE::INSTALL))
    {
        return MHWD::STATUS::ERROR_SCRIPT_FAILED;
    }

    if (!copyDirectory(config->basePath_, databaseDir + "/" + config->name_))
    {
        return MHWD::STATUS::ERROR_SET_DATABASE;
    }

    // Installed config vectors have to be updated manual with updateInstalledConfigData(Data*)

    return MHWD::STATUS::SUCCESS;
}

MHWD::STATUS Mhwd::uninstallConfig(Config *config)
{
    std::shared_ptr<Config> installedConfig{getInstalledConfig(config->name_, config->type_)};

    // Check if installed
    if (nullptr == installedConfig)
    {
        return MHWD::STATUS::ERROR_NOT_INSTALLED;
    }
    else if (installedConfig->basePath_ != config->basePath_)
    {
        return MHWD::STATUS::ERROR_NO_MATCH_LOCAL_CONFIG;
    }
    else
    {
        // TODO: Should we check for local requirements here?

        // Run script
        if (!runScript(installedConfig, MHWD::TRANSACTIONTYPE::REMOVE))
        {
            return MHWD::STATUS::ERROR_SCRIPT_FAILED;
        }

        if (!removeDirectory(installedConfig->basePath_))
        {
            return MHWD::STATUS::ERROR_SET_DATABASE;
        }

        // Installed config vectors have to be updated manual with updateInstalledConfigData(Data*)

        return MHWD::STATUS::SUCCESS;
    }
}

bool Mhwd::runScript(std::shared_ptr<Config> config, MHWD::TRANSACTIONTYPE operationType)
{
    std::string cmd = "exec " + std::string(MHWD_SCRIPT_PATH);

    if (MHWD::TRANSACTIONTYPE::REMOVE == operationType)
    {
        cmd += " --remove";
    }
    else
    {
        cmd += " --install";
    }

    if (data_.environment.syncPackageManagerDatabase)
    {
        cmd += " --sync";
    }

    cmd += " --cachedir \"" + data_.environment.PMCachePath + "\"";
    cmd += " --pmconfig \"" + data_.environment.PMConfigPath + "\"";
    cmd += " --pmroot \"" + data_.environment.PMRootPath + "\"";
    cmd += " --config \"" + config->configPath_ + "\"";

    // Set all config devices as argument
    std::vector<std::shared_ptr<Device>> foundDevices;
    std::vector<std::shared_ptr<Device>> devices;
    data_.getAllDevicesOfConfig(config, foundDevices);

    for (auto&& foundDevice = foundDevices.begin();
            foundDevice != foundDevices.end(); ++foundDevice)
    {
        bool found = false;

        // Check if already in list
        for (auto&& dev = devices.begin(); dev != devices.end(); ++dev)
        {
            if ((*foundDevice)->sysfsBusID_ == (*dev)->sysfsBusID_
                    && (*foundDevice)->sysfsID_ == (*dev)->sysfsID_)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            devices.push_back(std::shared_ptr<Device>{*foundDevice});
        }
    }

    for (auto&& dev = devices.begin(); dev != devices.end(); ++dev)
    {
        Vita::string busID = (*dev)->sysfsBusID_;

        if ("PCI" == config->type_)
        {
            std::vector<Vita::string> split = Vita::string(busID).replace(".", ":").explode(":");
            const unsigned long size = split.size();

            if (size >= 3)
            {
                // Convert to int to remove leading 0
                busID = Vita::string::toStr<int>(std::stoi(split[size - 3], nullptr, 16));
                busID += ":" + Vita::string::toStr<int>(std::stoi(split[size - 2], nullptr, 16));
                busID += ":" + Vita::string::toStr<int>(std::stoi(split[size - 1], nullptr, 16));
            }
        }

        cmd += " --device \"" + (*dev)->classID_ + "|" + (*dev)->vendorID_ + "|" + (*dev)->deviceID_
                + "|" + busID + "\"";
    }

    cmd += " 2>&1";

    FILE *in;

    if (!(in = popen(cmd.c_str(), "r")))
    {
        return false;
    }
    else
    {
        char buff[512];
        while (fgets(buff, sizeof(buff), in) != nullptr)
        {
            printer_.printMessage(MHWD::MESSAGETYPE::CONSOLE_OUTPUT, buff);
        }

        int stat = pclose(in);

        if (WEXITSTATUS(stat) != 0)
        {
            return false;
        }
        else
        {
            // Only one database sync is required
            if (MHWD::TRANSACTIONTYPE::INSTALL == operationType)
            {
                data_.environment.syncPackageManagerDatabase = false;
            }
            return true;
        }
    }
}

void Mhwd::tryToParseCmdLineOptions(int argc, char* argv[], bool& autoConfigureNonFreeDriver,
		std::string& operationType, std::string& autoConfigureClassID)
{
	if (argc <= 1)
	{
		arguments_.LIST_AVAILABLE = true;
	}
	for (int nArg = 1; nArg < argc; ++nArg)
	{
		const std::string option{ argv[nArg] };
		if (("-h" == option) || ("--help" == option))
		{
			printer_.printHelp();
		}
		else if (("-f" == option) || ("--force" == option))
		{
			arguments_.FORCE = true;
		}
		else if (("-d" == option) || ("--detail" == option))
		{
			arguments_.DETAIL = true;
		}
		else if (("-la" == option) || ("--listall" == option))
		{
			arguments_.LIST_ALL = true;
		}
		else if (("-li" == option) || ("--listinstalled" == option))
		{
			arguments_.LIST_INSTALLED = true;
		}
		else if (("-l" == option) || ("--list" == option))
		{
			arguments_.LIST_AVAILABLE = true;
		}
		else if (("-lh" == option) || ("--listhardware" == option))
		{
			arguments_.LIST_HARDWARE = true;
		}
		else if ("--pci" == option)
		{
			arguments_.SHOW_PCI = true;
		}
		else if ("--usb" == option)
		{
			arguments_.SHOW_USB = true;
		}
		else if (("-a" == option) || ("--auto" == option))
		{
			if ((nArg + 3) >= argc)
			{
				throw std::runtime_error{"invalid use of option: -a/--auto\n"};
			}
			else
			{
				const std::string deviceType{ argv[nArg + 1] };
				const std::string driverType{ argv[nArg + 2] };
				const std::string classID{ argv[nArg + 3] };
				if ((("pci" != deviceType) && ("usb" != deviceType))
						|| (("free" != driverType) && ("nonfree" != driverType)))
				{
					throw std::runtime_error{"invalid use of option: -a/--auto\n"};
				}
				else
				{
					operationType = Vita::string{ deviceType }.toUpper();
					autoConfigureNonFreeDriver = ("nonfree" == driverType);
					autoConfigureClassID = Vita::string(classID).toLower().trim();
					arguments_.AUTOCONFIGURE = true;
					nArg += 3;
				}
			}
		}
		else if (("-ic" == option) || ("--installcustom" == option))
		{
			if ((nArg + 1) >= argc)
			{
				throw std::runtime_error{"invalid use of option: -ic/--installcustom\n"};
			}
			else
			{
				const std::string deviceType{ argv[++nArg] };
				if (("pci" != deviceType) && ("usb" != deviceType))
				{
					throw std::runtime_error{"invalid use of option: -ic/--installcustom\n"};
				}
				else
				{
					operationType = Vita::string{ deviceType }.toUpper();
					arguments_.CUSTOM_INSTALL = true;
				}
			}
		}
		else if (("-i" == option) || ("--install" == option))
		{
			if ((nArg + 1) >= argc)
			{
				throw std::runtime_error{"invalid use of option: -i/--install\n"};
			}
			else
			{
				const std::string deviceType{ argv[++nArg] };
				if (("pci" != deviceType) && ("usb" != deviceType))
				{
					throw std::runtime_error{"invalid use of option: -i/--install\n"};
				}
				else
				{
					operationType = Vita::string{ deviceType }.toUpper();
					arguments_.INSTALL = true;
				}
			}
		}
		else if (("-r" == option) || ("--remove" == option))
		{
			if ((nArg + 1) >= argc)
			{
				throw std::runtime_error{"invalid use of option: -r/--remove\n"};
			}
			else
			{
				const std::string deviceType{ argv[++nArg] };
				if (("pci" != deviceType) && ("usb" != deviceType))
				{
					throw std::runtime_error{"invalid use of option: -r/--remove\n"};
				}
				else
				{
					operationType = Vita::string{ deviceType }.toUpper();
					arguments_.REMOVE = true;
				}
			}
		}
		else if ("--pmcachedir" == option)
		{
			if (nArg + 1 >= argc)
			{
				throw std::runtime_error{"invalid use of option: --pmcachedir\n"};
			}
			else
			{
				data_.environment.PMCachePath = Vita::string(argv[++nArg]).trim("\"").trim();
			}
		}
		else if ("--pmconfig" == option)
		{
			if (nArg + 1 >= argc)
			{
				throw std::runtime_error{"invalid use of option: --pmconfig\n"};
			}
			else
			{
				data_.environment.PMConfigPath = Vita::string(argv[++nArg]).trim("\"").trim();
			}
		}
		else if ("--pmroot" == option)
		{
			if (nArg + 1 >= argc)
			{
				throw std::runtime_error{"invalid use of option: --pmroot\n"};
			}
			else
			{
				data_.environment.PMRootPath = Vita::string(argv[++nArg]).trim("\"").trim();
			}
		}
		else if (arguments_.INSTALL || arguments_.REMOVE)
		{
			bool found = false;
			std::string name;
			if (arguments_.CUSTOM_INSTALL)
			{
				name = std::string{ argv[nArg] };
			}
			else
			{
				name = Vita::string(argv[nArg]).toLower();
			}
			for (const auto& config : configs_)
			{
				if (config == name)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				configs_.push_back(name);
			}
		}
		else
		{
			throw std::runtime_error{"invalid option: " + std::string(argv[nArg]) + "\n"};
		}
	}
	if (!arguments_.SHOW_PCI && !arguments_.SHOW_USB)
	{
		arguments_.SHOW_USB = true;
		arguments_.SHOW_PCI = true;
	}
}

bool Mhwd::optionsDontInterfereWithEachOther() const
{
	if (arguments_.INSTALL && arguments_.REMOVE)
	{
    	printer_.printError("install and remove options can only be used separately!\n");
    	printer_.printHelp();
    	return false;
	}
	else if ((arguments_.INSTALL || arguments_.REMOVE) && arguments_.AUTOCONFIGURE)
	{
    	printer_.printError("auto option can't be combined with install and remove options!\n");
    	printer_.printHelp();
    	return false;
	}
	else if ((arguments_.REMOVE || arguments_.INSTALL) && configs_.empty())
	{
    	printer_.printError("nothing to do?!\n");
    	printer_.printHelp();
    	return false;
	}

	return true;
}

int Mhwd::launch(int argc, char *argv[])
{
    std::string operationType;
    bool autoConfigureNonFreeDriver = false;
    std::string autoConfigureClassID;

    try
    {
    	tryToParseCmdLineOptions(argc, argv, autoConfigureNonFreeDriver, operationType,
    			autoConfigureClassID);
    }
    catch(const std::runtime_error& e)
    {
    	printer_.printError(e.what());
    	printer_.printHelp();
    	return 1;
    }

    if (!optionsDontInterfereWithEachOther())
    {
    	return 1;
    }

    // Check environment
    std::string missingDir { checkEnvironment() };
    if (!missingDir.empty())
    {
        printer_.printError("directory '" + missingDir + "' does not exist!");
        return 1;
    }

    // Check for invalid configs
    for (auto&& invalidConfig : data_.invalidConfigs)
    {
        printer_.printWarning("config '" + invalidConfig->configPath_ + "' is invalid!");
    }

    // > Perform operations:

    // List all configs
    if (arguments_.LIST_ALL && arguments_.SHOW_PCI)
    {
        if (!data_.allPCIConfigs.empty())
        {
            printer_.listConfigs(data_.allPCIConfigs, "All PCI configs:");
        }
        else
        {
            printer_.printWarning("No PCI configs found!");
        }
    }
    if (arguments_.LIST_ALL && arguments_.SHOW_USB)
    {
        if (!data_.allUSBConfigs.empty())
        {
            printer_.listConfigs(data_.allUSBConfigs, "All USB configs:");
        }
        else
        {
            printer_.printWarning("No USB configs found!");
        }
    }

    // List installed configs
    if (arguments_.LIST_INSTALLED && arguments_.SHOW_PCI)
    {
        if (arguments_.DETAIL)
        {
            printer_.printInstalledConfigs("PCI", data_.installedPCIConfigs);
        }
        else
        {
            if (!data_.installedPCIConfigs.empty())
            {
                printer_.listConfigs(data_.installedPCIConfigs, "Installed PCI configs:");
            }
            else
            {
                printer_.printWarning("No installed PCI configs!");
            }
        }
    }
    if (arguments_.LIST_INSTALLED && arguments_.SHOW_USB)
    {
        if (arguments_.DETAIL)
        {
            printer_.printInstalledConfigs("USB", data_.installedUSBConfigs);
        }
        else
        {
            if (!data_.installedUSBConfigs.empty())
            {
                printer_.listConfigs(data_.installedUSBConfigs, "Installed USB configs:");
            }
            else
            {
                printer_.printWarning("No installed USB configs!");
            }
        }
    }

    // List available configs
    if (arguments_.LIST_AVAILABLE && arguments_.SHOW_PCI)
    {
        if (arguments_.DETAIL)
        {
            printer_.printAvailableConfigsInDetail("PCI", data_.PCIDevices);
        }
        else
        {
            for (auto&& PCIDevice : data_.PCIDevices)
            {
                if (!PCIDevice->availableConfigs_.empty())
                {
                    printer_.listConfigs(PCIDevice->availableConfigs_,
                            PCIDevice->sysfsBusID_ + " (" + PCIDevice->classID_ + ":"
                                    + PCIDevice->vendorID_ + ":" + PCIDevice->deviceID_ + ") "
                                    + PCIDevice->className_ + " " + PCIDevice->vendorName_ + ":");
                }
            }
        }
    }

    if (arguments_.LIST_AVAILABLE && arguments_.SHOW_USB)
    {
        if (arguments_.DETAIL)
        {
            printer_.printAvailableConfigsInDetail("USB", data_.USBDevices);
        }

        else
        {
            for (auto&& USBdevice : data_.USBDevices)
            {
                if (!USBdevice->availableConfigs_.empty())
                {
                    printer_.listConfigs(USBdevice->availableConfigs_,
                            USBdevice->sysfsBusID_ + " (" + USBdevice->classID_ + ":"
							+ USBdevice->vendorID_ + ":" + USBdevice->deviceID_ + ") "
							+ USBdevice->className_ + " " + USBdevice->vendorName_ + ":");
                }
            }
        }
    }

    // List hardware information
    if (arguments_.LIST_HARDWARE && arguments_.SHOW_PCI)
    {
        if (arguments_.DETAIL)
        {
            printDeviceDetails("PCI");
        }
        else
        {
            printer_.listDevices(data_.PCIDevices, "PCI");
        }
    }
    if (arguments_.LIST_HARDWARE && arguments_.SHOW_USB)
    {
        if (arguments_.DETAIL)
        {
            printDeviceDetails("USB");
        }
        else
        {
            printer_.listDevices(data_.USBDevices, "USB");
        }
    }

    // Auto configuration
    if (arguments_.AUTOCONFIGURE)
    {
        std::vector<std::shared_ptr<Device>> *devices;
        std::vector<std::shared_ptr<Config>> *installedConfigs;

        if ("USB" == operationType)
        {
            devices = &data_.USBDevices;
            installedConfigs = &data_.installedUSBConfigs;
        }
        else
        {
            devices = &data_.PCIDevices;
            installedConfigs = &data_.installedPCIConfigs;
        }
        bool foundDevice = false;
        for (auto&& device : *devices)
        {
            if (device->classID_ != autoConfigureClassID)
            {
                continue;
            }
            else
            {
                foundDevice = true;
                std::shared_ptr<Config> config;

                for (auto&& availableConfig : device->availableConfigs_)
                {
                    if (autoConfigureNonFreeDriver || availableConfig->freedriver_)
                    {
                        config = availableConfig;
                        break;
                    }
                }

                if (nullptr == config)
                {
                    printer_.printWarning(
                            "No config found for device: " + device->sysfsBusID_ + " ("
                                    + device->classID_ + ":" + device->vendorID_ + ":"
                                    + device->deviceID_ + ") " + device->className_ + " "
                                    + device->vendorName_ + " " + device->deviceName_);
                    continue;
                }
                else
                {
                	bool alreadyInList = std::find(configs_.begin(), configs_.end(), config->name_) != configs_.end();

                    // If force is not set then skip found config
                    bool skip = false;
                    if (!arguments_.FORCE)
                    {
                    	skip = std::find_if(installedConfigs->begin(), installedConfigs->end(),
                    			[&config](const std::shared_ptr<Config>& conf) -> bool {
                    				return conf->name_ == config->name_;
                    			})
                    				!= installedConfigs->end();
                    }
                    // Print found config
                    if (skip)
                    {
                        printer_.printStatus(
                                "Skipping already installed config '" + config->name_ +
                                "' for device: " + device->sysfsBusID_ + " (" +
                                device->classID_ + ":" + device->vendorID_ + ":" +
                                device->deviceID_ + ") " + device->className_ + " " +
                                device->vendorName_ + " " + device->deviceName_);
                    }
                    else
                    {
                        printer_.printStatus(
                                "Using config '" + config->name_ + "' for device: " +
                                device->sysfsBusID_ + " (" + device->classID_ + ":" +
                                device->vendorID_ + ":" + device->deviceID_ + ") " +
                                device->className_ + " " + device->vendorName_ + " " +
                                device->deviceName_);
                    }

                    if (!alreadyInList && !skip)
                    {
                        configs_.push_back(config->name_);
                    }
                }
            }
        }

        if (!foundDevice)
        {
            printer_.printWarning("No device of class " + autoConfigureClassID + " found!");
        }
        else if (!configs_.empty())
        {
            arguments_.INSTALL = true;
        }
    }

    // Transaction
    if (arguments_.INSTALL || arguments_.REMOVE)
    {
        if (isUserRoot())
        {
            for (auto&& configName = configs_.begin();
                    configName != configs_.end(); configName++)
            {
                if (arguments_.CUSTOM_INSTALL)
                {
                    // Custom install -> get configs
                    struct stat filestatus;
                    std::string filepath = (*configName) + "/MHWDCONFIG";

                    if (0 != stat(filepath.c_str(), &filestatus))
                    {
                        printer_.printError("custom config '" + filepath + "' does not exist!");
                        return 1;
                    }
                    else if (!S_ISREG(filestatus.st_mode))
                    {
                        printer_.printError("custom config '" + filepath + "' is invalid!");
                        return 1;
                    }
                    else
                    {
                        config_.reset(new Config(filepath, operationType));
                        if (!data_.fillConfig(config_, filepath, operationType))
                        {
                            printer_.printError("failed to read custom config '" + filepath + "'!");
                            return 1;
                        }

                        else if (!performTransaction(config_, MHWD::TRANSACTIONTYPE::INSTALL))
                        {
                            return 1;
                        }
                    }
                }
                else if (arguments_.INSTALL)
                {
                    config_ = getAvailableConfig((*configName), operationType);
                    if (config_ == nullptr)
                    {
                        config_ = getDatabaseConfig((*configName), operationType);
                        if (config_ == nullptr)
                        {
                            printer_.printError("config '" + (*configName) + "' does not exist!");
                            return 1;
                        }
                        else
                        {
                            printer_.printWarning(
                                    "no matching device for config '" + (*configName) + "' found!");
                        }
                    }

                    if (!performTransaction(config_, MHWD::TRANSACTIONTYPE::INSTALL))
                    {
                        return 1;
                    }
                }
                else if (arguments_.REMOVE)
                {
                    config_ = getInstalledConfig((*configName), operationType);

                    if (nullptr == config_)
                    {
                        printer_.printError("config '" + (*configName) + "' is not installed!");
                        return 1;
                    }

                    else if (!performTransaction(config_, MHWD::TRANSACTIONTYPE::REMOVE))
                    {
                        return 1;
                    }
                }
            }
        }
        else
        {
            printer_.printError("You cannot perform this operation unless you are root!");
        }
    }
    return 0;
}
