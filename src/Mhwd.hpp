/*
 * Mhwd.hpp
 *
 *  Created on: 26 sie 2014
 *      Author: dec
 */

#ifndef MHWD_HPP_
#define MHWD_HPP_

#include "Config.hpp"
#include "const.h"
#include "Data.hpp"
#include "Device.hpp"
#include "Enums.hpp"
#include "Printer.hpp"
#include "vita/string.hpp"
#include "Transaction.hpp"

#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

class Mhwd
{
public:

    Mhwd();
    ~Mhwd();
    int launch(int argc, char *argv[]);

private:
    bool performTransaction(Config* config, MHWD::TRANSACTIONTYPE type);
    bool isUserRoot() const;
    std::string checkEnvironment();

    void printDeviceDetails(std::string type, FILE *f = stdout);

    Config* getInstalledConfig(const std::string& configName, const std::string& configType);
    Config* getDatabaseConfig(const std::string& configName, const std::string configType); // -> transaction ??
    Config* getAvailableConfig(const std::string& configName, const std::string configType);

    std::vector<Config*> getAllLocalRequirements(Config *config);

    MHWD::ARGUMENTS arguments_;
    std::shared_ptr<Config> config_;
    Data data_;
    Printer printer_;

    MHWD::STATUS performTransaction(Transaction *transaction);

    int hexToInt(std::string hex);

    //###############//
    //### Configs ###//
    //###############//
    std::vector<std::string> splitValue(Vita::string str, Vita::string onlyEnding = ""); // -> data // ??
    Vita::string getRightConfigPath(Vita::string str, Vita::string baseConfigPath);

    //#####################################//
    //### Directory and File Operations ###//
    //#####################################//
    bool copyDirectory(const std::string source, const std::string destination);
    bool copyFile(const std::string source, const std::string destination, const mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH);
    bool removeDirectory(const std::string directory);
    bool checkExist(const std::string path);
    bool createDir(const std::string path, const mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH | S_IXGRP | S_IXOTH);

    //###########################//
    //### Script & Operations ###//
    //###########################//
    MHWD::STATUS installConfig(Config *config);
    MHWD::STATUS uninstallConfig(Config *config);
    bool runScript(Config *config, MHWD::TRANSACTIONTYPE operationType);
};

#endif /* MHWD_HPP_ */
