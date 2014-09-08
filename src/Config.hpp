/*
 * Config.hpp
 *
 *  Created on: 28 sie 2014
 *      Author: dec
 */

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include "Enums.hpp"
#include "../libmhwd/vita/string.hpp"

#include <string>
#include <vector>

struct Config
{
    Config(std::string configPath, std::string type);
    bool readConfigFile(std::string configPath);

    struct HardwareID
    {
        std::vector<std::string> classIDs;
        std::vector<std::string> vendorIDs;
        std::vector<std::string> deviceIDs;
        std::vector<std::string> blacklistedClassIDs;
        std::vector<std::string> blacklistedVendorIDs;
        std::vector<std::string> blacklistedDeviceIDs;
    };

    std::string type_;
    std::string basePath_;
    std::string configPath_;
    std::string name_;
    std::string info_;
    std::string version_;
    bool freedriver_ = true;
    int priority_ = 0;
    std::vector<HardwareID> hwdIDs_;
    std::vector<std::string> conflicts_;
    std::vector<std::string> dependencies_;

private:
    std::vector<std::string> splitValue(Vita::string str, Vita::string onlyEnding = "");
    Vita::string getRightConfigPath(Vita::string str, Vita::string baseConfigPath);
};

#endif /* CONFIG_HPP_ */
