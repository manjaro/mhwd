/*
 * Mhwd.cpp
 *
 *  Created on: 26 sie 2014
 *      Author: dec
 */

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "Mhwd.hpp"
#include "vita/string.hpp"

Mhwd::Mhwd() : arguments_(MHWD::ARGUMENTS::NONE), data_(), printer_()
{
}

Mhwd::~Mhwd()
{
}

bool Mhwd::performTransaction(Config* config, MHWD::TRANSACTIONTYPE type)
{
//  Transaction transaction (data_, config, type,
//          (arguments_ & MHWD::ARGUMENTS::FORCE));
    Transaction transaction;
    transaction.config_ = config;
    transaction.allowedToReinstall_ = (arguments_ & MHWD::ARGUMENTS::FORCE);
    transaction.type_ = type;
    transaction.dependencyConfigs_ = data_.getAllDependenciesToInstall(config);
    transaction.conflictedConfigs_ = data_.getAllLocalConflicts(config);
    transaction.configsRequirements_ = data_.getAllLocalRequirements(config);

    // Print things to do
    if (type == MHWD::TRANSACTIONTYPE::INSTALL)
    {
        // Print conflicts
        if (!transaction.conflictedConfigs_.empty())
        {
            std::string conflicts;

            for (auto conflictedConfig : transaction.conflictedConfigs_)
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

            for (auto dependencyConfig : transaction.dependencyConfigs_)
            {
                dependencies += " " + dependencyConfig->name_;
            }

            printer_.printStatus("Dependencies to install:" + dependencies +
                    "\nProceed with installation? [Y/n]");

            std::string input;
            std::getline(std::cin, input);

            if (input.length() == 1 && (input[0] == 'y' || input[0] == 'Y'))
            {
                return true;
            }
            else if (input.length() == 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    else if (type == MHWD::TRANSACTIONTYPE::REMOVE)
    {
        // Print requirements
        if (!transaction.configsRequirements_.empty())
        {
            std::string requirements;

            for (auto requirement : transaction.configsRequirements_)
            {
                requirements += " " + requirement->name_;
            }

            printer_.printError("config '" + config->name_ + "' is required by config(s):" +
                    requirements);
            return false;
        }
    }

    MHWD::STATUS status = performTransaction(&transaction);

    if (status == MHWD::STATUS::ERROR_ALREADY_INSTALLED)
    {
        printer_.printWarning(
                "a version of config '" + config->name_
                        + "' is already installed!\nUse -f/--force to force installation...");
    }
    else if (status == MHWD::STATUS::ERROR_CONFLICTS)
    {
        printer_.printError("config '" + config->name_ + "' conflicts with installed config(s)!");
    }
    else if (status == MHWD::STATUS::ERROR_NOT_INSTALLED)
    {
        printer_.printError("config '" + config->name_ + "' is not installed!");
    }
    else if (status == MHWD::STATUS::ERROR_NO_MATCH_LOCAL_CONFIG)
    {
        printer_.printError("passed config does not match with installed config!");
    }
    else if (status == MHWD::STATUS::ERROR_REQUIREMENTS)
    {
        printer_.printError("config '" + config->name_ + "' is required by installed config(s)!");
    }
    else if (status == MHWD::STATUS::ERROR_SCRIPT_FAILED)
    {
        printer_.printError("script failed!");
    }
    else if (status == MHWD::STATUS::ERROR_SET_DATABASE)
    {
        printer_.printError("failed to set database!");
    }

    // Update mhwd data object
    data_.updateInstalledConfigData();

    return (status == MHWD::STATUS::SUCCESS);
}

bool Mhwd::isUserRoot() const
{
    constexpr unsigned short ROOT_UID = 0;
    if (getuid() != ROOT_UID)
    {
        return false;
    }
    return true;
}

std::string Mhwd::checkEnvironment()
{
    std::string retValue = "";

    // Check if required directories exists. Otherwise return missing directory...
    if (!checkExist(MHWD_USB_CONFIG_DIR))
    {
        retValue = MHWD_USB_CONFIG_DIR;
    }
    if (!checkExist(MHWD_PCI_CONFIG_DIR))
    {
        retValue = MHWD_PCI_CONFIG_DIR;
    }
    if (!checkExist(MHWD_USB_DATABASE_DIR))
    {
        retValue = MHWD_USB_DATABASE_DIR;
    }
    if (!checkExist(MHWD_PCI_DATABASE_DIR))
    {
        retValue = MHWD_PCI_DATABASE_DIR;
    }

    return retValue;
}

void Mhwd::printDeviceDetails(std::string type, FILE *f)
{
    hw_item hw;

    if (type == "USB")
    {
        hw = hw_usb;
    }
    else
    {
        hw = hw_pci;
    }

    hd_data_t *hd_data;
    hd_t *hd;
    hd_data = new hd_data_t();
    hd = hd_list(hd_data, hw, 1, nullptr);
    hd_t *beginningOfhd = hd;

    for (; hd; hd = hd->next)
    {
        hd_dump_entry(hd_data, hd, f);
    }

    hd_free_hd_list(beginningOfhd);
    hd_free_hd_data(hd_data);
    delete hd_data;
}

Config* Mhwd::getInstalledConfig(const std::string& configName,
        const std::string& configType)
{
    std::vector<Config*>* installedConfigs;

    // Get the right configs
    if (configType == "USB")
    {
        installedConfigs = &data_.installedUSBConfigs;
    }
    else
    {
        installedConfigs = &data_.installedPCIConfigs;
    }

    for (auto installedConfig = installedConfigs->begin();
            installedConfig != installedConfigs->end(); installedConfig++)
    {
        if (configName == (*installedConfig)->name_)
        {
            return (*installedConfig);
        }
    }

    return nullptr;
}

Config* Mhwd::getDatabaseConfig(const std::string& configName,
        const std::string configType)
{
    std::vector<Config*>* allConfigs;

    // Get the right configs
    if (configType == "USB")
    {
        allConfigs = &data_.allUSBConfigs;
    }
    else
    {
        allConfigs = &data_.allPCIConfigs;
    }

    for (auto iterator = allConfigs->begin();
            iterator != allConfigs->end(); iterator++)
    {
        if (configName == (*iterator)->name_)
        {
            return (*iterator);
        }
    }

    return nullptr;
}

Config* Mhwd::getAvailableConfig(const std::string& configName,
        const std::string configType)
{
    std::vector<Device*> *devices;

    // Get the right devices
    if (configType == "USB")
    {
        devices = &data_.USBDevices;
    }
    else
    {
        devices = &data_.PCIDevices;
    }

    for (auto device = devices->begin(); device != devices->end();
            device++)
    {
        if ((*device)->availableConfigs_.empty())
        {
            continue;
        }
        else
        {
            for (auto availableConfig = (*device)->availableConfigs_.begin();
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

MHWD::STATUS Mhwd::performTransaction(Transaction *transaction)
{
    if ((transaction->type_ == MHWD::TRANSACTIONTYPE::INSTALL) &&
            !transaction->conflictedConfigs_.empty())
    {
        return MHWD::STATUS::ERROR_CONFLICTS;
    }
    else if ((transaction->type_ == MHWD::TRANSACTIONTYPE::REMOVE)
            && !transaction->configsRequirements_.empty())
    {
        return MHWD::STATUS::ERROR_REQUIREMENTS;
    }
    else
    {

        // Check if already installed
        Config *installedConfig = getInstalledConfig(transaction->config_->name_,
                transaction->config_->type_);
        MHWD::STATUS status = MHWD::STATUS::SUCCESS;

        if ((transaction->type_ == MHWD::TRANSACTIONTYPE::REMOVE)
                || (installedConfig != nullptr && transaction->isAllowedToReinstall()))
        {
            if (installedConfig == nullptr)
            {
                return MHWD::STATUS::ERROR_NOT_INSTALLED;
            }
            else
            {
                printer_.printMessage(MHWD::MESSAGETYPE::REMOVE_START, installedConfig->name_);
                if ((status = uninstallConfig(installedConfig)) != MHWD::STATUS::SUCCESS)
                {
                    return status;
                }
                else
                {
                    printer_.printMessage(MHWD::MESSAGETYPE::REMOVE_END, installedConfig->name_);
                }
            }
        }

        if (transaction->type_ == MHWD::TRANSACTIONTYPE::INSTALL)
        {
            // Check if already installed but not allowed to reinstall
            if ((installedConfig != nullptr) && !transaction->isAllowedToReinstall())
            {
                return MHWD::STATUS::ERROR_ALREADY_INSTALLED;
            }
            else
            {
                // Install all dependencies first
                for (auto dependencyConfig = transaction->dependencyConfigs_.end() - 1;
                        dependencyConfig != transaction->dependencyConfigs_.begin() - 1;
                        --dependencyConfig)
                {
                    printer_.printMessage(MHWD::MESSAGETYPE::INSTALLDEPENDENCY_START,
                            (*dependencyConfig)->name_);
                    if ((status = installConfig((*dependencyConfig))) != MHWD::STATUS::SUCCESS)
                    {
                        return status;
                    }
                    else
                    {
                        printer_.printMessage(MHWD::MESSAGETYPE::INSTALLDEPENDENCY_END,
                                (*dependencyConfig)->name_);
                    }
                }

                printer_.printMessage(MHWD::MESSAGETYPE::INSTALL_START, transaction->config_->name_);
                if ((status = installConfig(transaction->config_)) != MHWD::STATUS::SUCCESS)
                {
                    return status;
                }
                else
                {
                    printer_.printMessage(MHWD::MESSAGETYPE::INSTALL_END,
                            transaction->config_->name_);
                }
            }
        }

        return status;
    }
}

int Mhwd::hexToInt(std::string hex)
{
    return std::stoi(hex, nullptr, 16);
}

bool Mhwd::copyDirectory(const std::string source, const std::string destination)
{
    struct stat filestatus;

    if (lstat(destination.c_str(), &filestatus) != 0)
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

    bool success = true;
    struct dirent *dir;
    DIR *d = opendir(source.c_str());

    if (!d)
    {
        return false;
    }

    while ((dir = readdir(d)) != nullptr)
    {
        std::string filename = std::string(dir->d_name);
        std::string sourcepath = source + "/" + filename;
        std::string destinationpath = destination + "/" + filename;

        if (filename == "." || filename == ".." || filename == "")
        {
            continue;
        }

        lstat(sourcepath.c_str(), &filestatus);

        if (S_ISREG(filestatus.st_mode))
        {
            if (!copyFile(sourcepath, destinationpath))
            {
                success = false;
            }
        }
        else if (S_ISDIR(filestatus.st_mode))
        {
            if (!copyDirectory(sourcepath, destinationpath))
            {
                success = false;
            }
        }
    }

    closedir(d);
    return success;
}

bool Mhwd::copyFile(const std::string source, const std::string destination, const mode_t mode)
{
    std::ifstream src(source, std::ios::binary);
    std::ofstream dst(destination, std::ios::binary);
    if (src.is_open() && dst.is_open())
    {
        dst << src.rdbuf();

        mode_t process_mask = umask(0);
        chmod(destination.c_str(), mode);
        umask(process_mask);

        return true;
    }
    else
    {
        return false;
    }
}

bool Mhwd::removeDirectory(const std::string directory)
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

            if (filename == "." || filename == ".." || filename == "")
            {
                continue;
            }
            else
            {
                struct stat filestatus;
                lstat(filepath.c_str(), &filestatus);

                if (S_ISREG(filestatus.st_mode))
                {
                    if (unlink(filepath.c_str()) != 0)
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

        if (rmdir(directory.c_str()) != 0)
        {
            success = false;
        }

        return success;
    }
}

bool Mhwd::checkExist(const std::string path)
{
    struct stat filestatus;
    if (stat(path.c_str(), &filestatus) != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool Mhwd::createDir(const std::string path, const mode_t mode)
{
    mode_t process_mask = umask(0);
    int ret = mkdir(path.c_str(), mode);
    umask(process_mask);

    return (ret == 0);
}

MHWD::STATUS Mhwd::installConfig(Config *config)
{
    std::string databaseDir;

    // Get the right configs
    if (config->type_ == "USB")
    {
        databaseDir = MHWD_USB_DATABASE_DIR;
    }
    else
    {
        databaseDir = MHWD_PCI_DATABASE_DIR;
    }

    // Run script
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
    Config *installedConfig = getInstalledConfig(config->name_, config->type_);

    // Check if installed
    if (installedConfig == nullptr)
    {
        return MHWD::STATUS::ERROR_NOT_INSTALLED;
    }
    else if (installedConfig->basePath_ != config->basePath_)
    {
        return MHWD::STATUS::ERROR_NO_MATCH_LOCAL_CONFIG;
    }

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

bool Mhwd::runScript(Config *config, MHWD::TRANSACTIONTYPE operationType)
{
    std::string cmd = "exec " + std::string(MHWD_SCRIPT_PATH);

    if (operationType == MHWD::TRANSACTIONTYPE::REMOVE)
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
    std::vector<Device*> foundDevices;
    std::vector<Device*> devices;
    data_.getAllDevicesOfConfig(config, &foundDevices);

    for (auto iterator = foundDevices.begin();
            iterator != foundDevices.end(); iterator++)
    {
        bool found = false;

        // Check if already in list
        for (auto dev = devices.begin(); dev != devices.end(); dev++)
        {
            if ((*iterator)->sysfsBusID_ == (*dev)->sysfsBusID_
                    && (*iterator)->sysfsID_ == (*dev)->sysfsID_)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            devices.push_back((*iterator));
        }
    }

    for (auto dev = devices.begin(); dev != devices.end(); dev++)
    {
        Vita::string busID = (*dev)->sysfsBusID_;

        if (config->type_ == "PCI")
        {
            std::vector<Vita::string> split = Vita::string(busID).replace(".", ":").explode(":");
            const int size = split.size();

            if (size >= 3)
            {
                // Convert to int to remove leading 0
                busID = Vita::string::toStr<int>(hexToInt(split[size - 3]));
                busID += ":" + Vita::string::toStr<int>(hexToInt(split[size - 2]));
                busID += ":" + Vita::string::toStr<int>(hexToInt(split[size - 1]));
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
            printer_.printMessage(MHWD::MESSAGETYPE::CONSOLE_OUTPUT, std::string(buff));
        }

        int stat = pclose(in);

        if (WEXITSTATUS(stat) != 0)
        {
            return false;
        }
        else
        {
            // Only one database sync is required
            if (operationType == MHWD::TRANSACTIONTYPE::INSTALL)
            {
                data_.environment.syncPackageManagerDatabase = false;
            }
            return true;
        }
    }
}

int Mhwd::launch(int argc, char *argv[])
{
    std::vector<std::string> configs;
    std::string operationType;
    bool autoConfigureNonFreeDriver;
    std::string autoConfigureClassID;

    if (argc <= 1)
    {
        arguments_ = MHWD::ARGUMENTS::LISTAVAILABLE;
    }
    else
    {
        arguments_ = MHWD::ARGUMENTS::NONE;
    }

    // Get command line arguments_
    for (int nArg = 1; nArg < argc; nArg++)
    {
        std::string option { argv[nArg] };

        if (option == "-h" || option == "--help")
        {
            printer_.printHelp();
            return 0;
        }
        else if (option == "-f" || option == "--force")
        {
            arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ | MHWD::ARGUMENTS::FORCE);
        }
        else if (option == "-d" || option == "--detail")
        {
            arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ | MHWD::ARGUMENTS::DETAIL);
        }
        else if (option == "-la" || option == "--listall")
        {
            arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ | MHWD::ARGUMENTS::LISTALL);
        }
        else if (option == "-li" || option == "--listinstalled")
        {
            arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ | MHWD::ARGUMENTS::LISTINSTALLED);
        }
        else if (option == "-l" || option == "--list")
        {
            arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ | MHWD::ARGUMENTS::LISTAVAILABLE);
        }
        else if (option == "-lh" || option == "--listhardware")
        {
            arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ | MHWD::ARGUMENTS::LISTHARDWARE);
        }
        else if (option == "--pci")
        {
            arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ | MHWD::ARGUMENTS::SHOWPCI);
        }
        else if (option == "--usb")
        {
            arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ | MHWD::ARGUMENTS::SHOWUSB);
        }
        else if (option == "-a" || option == "--auto")
        {
            ++nArg;
            if (nArg + 2 >= argc
                    || (strcmp(argv[nArg], "pci") != 0 && strcmp(argv[nArg], "usb") != 0)
                    || (strcmp(argv[nArg + 1], "free") != 0
                            && strcmp(argv[nArg + 1], "nonfree") != 0))
            {
                printer_.printError("invalid use of option: -a/--auto\n");
                printer_.printHelp();
                return 1;
            }
            else
            {
                if (strcmp(argv[nArg], "usb") == 0)
                {
                    operationType = "USB";
                }
                else
                {
                    operationType = "PCI";
                }

                ++nArg;
                if (strcmp(argv[nArg], "nonfree") == 0)
                {
                    autoConfigureNonFreeDriver = true;
                }
                else
                {
                    autoConfigureNonFreeDriver = false;
                }

                autoConfigureClassID = Vita::string(argv[++nArg]).toLower().trim();
                arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ |
                        MHWD::ARGUMENTS::AUTOCONFIGURE);
            }
        }
        else if (strcmp(argv[nArg], "-ic") == 0 || strcmp(argv[nArg], "--installcustom") == 0)
        {
            ++nArg;
            if ((nArg >= argc) || ((strcmp(argv[nArg], "pci") != 0) &&
                    (strcmp(argv[nArg], "usb") != 0)))
            {
                printer_.printError("invalid use of option: -ic/--installcustom\n");
                printer_.printHelp();
                return 1;
            }
            else
            {
                if (strcmp(argv[nArg], "usb") == 0)
                {
                    operationType = "USB";
                }
                else
                {
                    operationType = "PCI";
                }

                arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ | MHWD::ARGUMENTS::INSTALL
                        | MHWD::ARGUMENTS::CUSTOMINSTALL);
            }
        }
        else if (strcmp(argv[nArg], "-i") == 0 || strcmp(argv[nArg], "--install") == 0)
        {
            ++nArg;
            if (nArg >= argc || (strcmp(argv[nArg], "pci") != 0 &&
                    strcmp(argv[nArg], "usb") != 0))
            {
                printer_.printError("invalid use of option: -i/--install\n");
                printer_.printHelp();
                return 1;
            }
            else
            {
                if (strcmp(argv[nArg], "usb") == 0)
                {
                    operationType = "USB";
                }
                else
                {
                    operationType = "PCI";
                }

                arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ | MHWD::ARGUMENTS::INSTALL);
            }
        }
        else if (strcmp(argv[nArg], "-r") == 0 || strcmp(argv[nArg], "--remove") == 0)
        {
            ++nArg;
            if (nArg >= argc || (strcmp(argv[nArg], "pci") != 0 && strcmp(argv[nArg], "usb") != 0))
            {
                printer_.printError("invalid use of option: -r/--remove\n");
                printer_.printHelp();
                return 1;
            }
            else
            {
                if (strcmp(argv[nArg], "usb") == 0)
                {
                    operationType = "USB";
                }
                else
                {
                    operationType = "PCI";
                }

                arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ | MHWD::ARGUMENTS::REMOVE);
            }
        }
        else if (strcmp(argv[nArg], "--pmcachedir") == 0)
        {
            if (nArg + 1 >= argc)
            {
                printer_.printError("invalid use of option: --pmcachedir\n");
                printer_.printHelp();
                return 1;
            }
            else
            {
                data_.environment.PMCachePath = Vita::string(argv[++nArg]).trim("\"").trim();
            }
        }
        else if (strcmp(argv[nArg], "--pmconfig") == 0)
        {
            if (nArg + 1 >= argc)
            {
                printer_.printError("invalid use of option: --pmconfig\n");
                printer_.printHelp();
                return 1;
            }
            else
            {
                data_.environment.PMConfigPath = Vita::string(argv[++nArg]).trim("\"").trim();
            }
        }
        else if (strcmp(argv[nArg], "--pmroot") == 0)
        {
            if (nArg + 1 >= argc)
            {
                printer_.printError("invalid use of option: --pmroot\n");
                printer_.printHelp();
                return 1;
            }
            else
            {
                data_.environment.PMRootPath = Vita::string(argv[++nArg]).trim("\"").trim();
            }
        }
        else if (((arguments_ & MHWD::ARGUMENTS::INSTALL))
                || (arguments_ & MHWD::ARGUMENTS::REMOVE))
        {
            bool found = false;
            std::string name;

            if (arguments_ & MHWD::ARGUMENTS::CUSTOMINSTALL)
            {
                name = std::string{argv[nArg]};
            }
            else
            {
                name = Vita::string(argv[nArg]).toLower();
            }

            for (auto config : configs)
            {
                if (config == name)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                configs.push_back(name);
            }
        }
        else
        {
            printer_.printError("invalid option: " + std::string(argv[nArg]) + "\n");
            printer_.printHelp();
            return 1;
        }
    }

    // Check if arguments_ are right
    if (((arguments_ & MHWD::ARGUMENTS::INSTALL)) && (arguments_ & MHWD::ARGUMENTS::REMOVE))
    {
        printer_.printError("install and remove options can be used only seperate!\n");
        printer_.printHelp();
        return 1;
    }
    else if ((((arguments_ & MHWD::ARGUMENTS::INSTALL)) || (arguments_ & MHWD::ARGUMENTS::REMOVE))
            && (arguments_ & MHWD::ARGUMENTS::AUTOCONFIGURE))
    {
        printer_.printError("auto option can't be combined with install and remove options!\n");
        printer_.printHelp();
        return 1;
    }
    else if (((arguments_ & MHWD::ARGUMENTS::REMOVE) || (arguments_ & MHWD::ARGUMENTS::INSTALL))
            && configs.empty())
    {
        printer_.printError("nothing to do?!\n");
        printer_.printHelp();
        return 1;
    }
    else if (!((arguments_ & MHWD::ARGUMENTS::SHOWPCI)) &&
            !((arguments_ & MHWD::ARGUMENTS::SHOWUSB)))
    {
        arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ | MHWD::ARGUMENTS::SHOWUSB
                | MHWD::ARGUMENTS::SHOWPCI);
    }

    // Check environment
    std::string missingDir { checkEnvironment() };
    if (!missingDir.empty())
    {
        printer_.printError("directory '" + missingDir + "' does not exist!");
        return 1;
    }

    // Check for invalid configs
    for (auto invalidConfig : data_.invalidConfigs)
    {
        printer_.printWarning("config '" + invalidConfig->configPath_ + "' is invalid!");
    }

    // > Perform operations:

    // List all configs
    if ((arguments_ & MHWD::ARGUMENTS::LISTALL) && (arguments_ & MHWD::ARGUMENTS::SHOWPCI))
    {
        printer_.listConfigs(data_.allPCIConfigs, "All PCI configs:", "No PCI configs found!");
    }
    if ((arguments_ & MHWD::ARGUMENTS::LISTALL) && (arguments_ & MHWD::ARGUMENTS::SHOWUSB))
    {
        printer_.listConfigs(data_.allUSBConfigs, "All USB configs:", "No USB configs found!");
    }

    // List installed configs
    if ((arguments_ & MHWD::ARGUMENTS::LISTINSTALLED) && (arguments_ & MHWD::ARGUMENTS::SHOWPCI))
    {
        if (arguments_ & MHWD::ARGUMENTS::DETAIL)
        {
            printer_.printInstalledConfigs("PCI", data_.installedPCIConfigs);
        }
        else
        {
            printer_.listConfigs(data_.installedPCIConfigs, "Installed PCI configs:",
                    "No installed PCI configs!");
        }
    }
    if ((arguments_ & MHWD::ARGUMENTS::LISTINSTALLED) && (arguments_ & MHWD::ARGUMENTS::SHOWUSB))
    {
        if (arguments_ & MHWD::ARGUMENTS::DETAIL)
        {
            printer_.printInstalledConfigs("USB", data_.installedUSBConfigs);
        }
        else
        {
            printer_.listConfigs(data_.installedUSBConfigs, "Installed USB configs:",
                    "No installed USB configs!");
        }
    }

    // List available configs
    if ((arguments_ & MHWD::ARGUMENTS::LISTAVAILABLE) && ((arguments_ & MHWD::ARGUMENTS::SHOWPCI)))
    {
        if (arguments_ & MHWD::ARGUMENTS::DETAIL)
        {
            printer_.printAvailableConfigs("PCI", data_.PCIDevices);
        }
        else
        {
            for (auto PCIDevice : data_.PCIDevices)
            {
                printer_.listConfigs(PCIDevice->availableConfigs_,
                        PCIDevice->sysfsBusID_ + " (" + PCIDevice->classID_ + ":"
                                + PCIDevice->vendorID_ + ":" + PCIDevice->deviceID_ + ") "
                                + PCIDevice->className_ + " " + PCIDevice->vendorName_ + ":");
            }
        }
    }

    if ((arguments_ & MHWD::ARGUMENTS::LISTAVAILABLE) && (arguments_ & MHWD::ARGUMENTS::SHOWUSB))
    {
        if (arguments_ & MHWD::ARGUMENTS::DETAIL)
        {
            printer_.printAvailableConfigs("USB", data_.USBDevices);
        }

        else
        {
            for (auto device : data_.USBDevices)
            {
                printer_.listConfigs(device->availableConfigs_,
                        device->sysfsBusID_ + " (" + device->classID_ + ":" + device->vendorID_ + ":"
                                + device->deviceID_ + ") " + device->className_ + " "
                                + device->vendorName_ + ":");
            }
        }
    }

    // List hardware information
    if ((arguments_ & MHWD::ARGUMENTS::LISTHARDWARE) && (arguments_ & MHWD::ARGUMENTS::SHOWPCI))
    {
        if (arguments_ & MHWD::ARGUMENTS::DETAIL)
        {
            printDeviceDetails("PCI");
        }
        else
        {
            printer_.listDevices(data_.PCIDevices, "PCI");
        }
    }
    if ((arguments_ & MHWD::ARGUMENTS::LISTHARDWARE) && (arguments_ & MHWD::ARGUMENTS::SHOWUSB))
    {
        if (arguments_ & MHWD::ARGUMENTS::DETAIL)
        {
            printDeviceDetails("USB");
        }
        else
        {
            printer_.listDevices(data_.USBDevices, "USB");
        }
    }

    // Auto configuration
    if (arguments_ & MHWD::ARGUMENTS::AUTOCONFIGURE)
    {
        bool founddevice = false;
        std::vector<Device*> *devices;
        std::vector<Config*> *installedConfigs;

        if (operationType == "USB")
        {
            devices = &data_.USBDevices;
            installedConfigs = &data_.installedUSBConfigs;
        }
        else
        {
            devices = &data_.PCIDevices;
            installedConfigs = &data_.installedPCIConfigs;
        }

        for (auto device : *devices)
        {
            if (device->classID_ != autoConfigureClassID)
            {
                continue;
            }
            else
            {
                founddevice = true;
                Config *config = nullptr;

                for (auto availableConfig : device->availableConfigs_)
                {
                    if (!autoConfigureNonFreeDriver && !(availableConfig->freedriver_))
                    {
                        continue;
                    }
                    else
                    {
                        config = availableConfig;
                        break;
                    }
                }

                if (config == nullptr)
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
                    // Check if already in list
                    bool found = false;
                    for (auto iter = configs.begin();
                            iter != configs.end(); iter++)
                    {
                        if ((*iter) == config->name_)
                        {
                            found = true;
                            break;
                        }
                    }

                    // If force is not set then skip found config
                    bool skip = false;
                    if (!(arguments_ & MHWD::ARGUMENTS::FORCE))
                    {
                        for (auto iter = installedConfigs->begin();
                                iter != installedConfigs->end(); iter++)
                        {
                            if ((*iter)->name_ == config->name_)
                            {
                                skip = true;
                                break;
                            }
                        }
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

                    if (!found && !skip)
                    {
                        configs.push_back(config->name_);
                    }
                }
            }
        }

        if (!founddevice)
        {
            printer_.printWarning("No device of class " + autoConfigureClassID + " found!");
        }
        else if (!configs.empty())
        {
            arguments_ = static_cast<MHWD::ARGUMENTS>(arguments_ | MHWD::ARGUMENTS::INSTALL);
        }
    }

    // Transaction
    if ((arguments_ & MHWD::ARGUMENTS::INSTALL) || (arguments_ & MHWD::ARGUMENTS::REMOVE))
    {
        if (isUserRoot())
        {
            for (auto configName = configs.begin();
                    configName != configs.end(); configName++)
            {
                if (arguments_ & MHWD::ARGUMENTS::CUSTOMINSTALL)
                {
                    // Custom install -> get configs
                    struct stat filestatus;
                    std::string filepath = (*configName) + "/MHWDCONFIG";

                    if (stat(filepath.c_str(), &filestatus) != 0)
                    {
                        printer_.printError("custom config '" + filepath + "' does not exist!");
                        return 1;
                    }
                    else if (!S_ISREG(filestatus.st_mode))
                    {
                        printer_.printError("custom config '" + filepath + "' is invalid!");
                        return 1;
                    }

                    config_ = new Config(filepath, operationType);
                    if (!data_.fillConfig(config_, filepath, operationType))
                    {
                        printer_.printError("failed to read custom config '" + filepath + "'!");
                        delete config_;
                        return 1;
                    }

                    else if (!performTransaction(config_, MHWD::TRANSACTIONTYPE::INSTALL))
                    {
                        return 1;
                    }
                }
                else if (arguments_ & MHWD::ARGUMENTS::INSTALL)
                {
                    config_ = getAvailableConfig((*configName), operationType);
                    if (config_ == nullptr)
                    {
                        config_ = getDatabaseConfig((*configName), operationType);
                        if (config_ == nullptr)
                        {
                            printer_.printError("config '" + (*configName) + "' does not exist!");
                            delete config_;
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
                        delete config_;
                        return 1;
                    }
                }
                else if (arguments_ & MHWD::ARGUMENTS::REMOVE)
                {
                    config_ = getInstalledConfig((*configName), operationType);

                    if (config_ == nullptr)
                    {
                        printer_.printError("config '" + (*configName) + "' is not installed!");
                        delete config_;
                        return 1;
                    }

                    else if (!performTransaction(config_, MHWD::TRANSACTIONTYPE::REMOVE))
                    {
                        delete config_;
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
