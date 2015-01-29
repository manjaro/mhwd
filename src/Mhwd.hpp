/*
 * Mhwd.hpp
 *
 *  Created on: 26 sie 2014
 *      Author: dec
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
