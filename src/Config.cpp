/*
 * Config.cpp
 *
 *  Created on: 28 sie 2014
 *      Author: dec
 */

#include <fstream>
#include <string>
#include <vector>

#include "Config.hpp"

Config::Config(std::string configPath, std::string type)
    : type_(type), basePath_(configPath.substr(0, configPath.find_last_of('/'))),
      configPath_(configPath)
{
    if (hwdIDs_.empty())
    {
        Config::HardwareID hwdID;
        hwdIDs_.push_back(hwdID);
    }
}

bool Config::readConfigFile(std::string configPath)
{
    std::ifstream file(configPath.c_str(), std::ios::in);

    if (!file.is_open())
    {
        return false;
    }

    Vita::string line;
    Vita::string key;
    Vita::string value;
    std::vector<Vita::string> parts;

    while (!file.eof())
    {
        std::getline(file, line);

        std::size_t pos = line.find_first_of('#');
        if (pos != std::string::npos)
        {
            line.erase(pos);
        }

        if (line.trim().empty())
        {
            continue;
        }

        parts = line.explode("=");
        key = parts.front().trim().toLower();
        value = parts.back().trim("\"").trim();

        // Read in extern file
        if ((value.size() > 1) && (">" == value.substr(0, 1)))
        {
            std::ifstream file(getRightConfigPath(value.substr(1), basePath_).c_str());
            if (!file.is_open())
            {
                return false;
            }

            Vita::string line;
            value.clear();

            while (!file.eof())
            {
                std::getline(file, line);

                std::size_t pos = line.find_first_of('#');
                if (std::string::npos != pos)
                {
                    line.erase(pos);
                }

                if (line.trim().empty())
                {
                    continue;
                }

                value += " " + line.trim();
            }

            value = value.trim();

            // remove all multiple spaces
            while (value.find("  ") != std::string::npos)
            {
                value = value.replace("  ", " ");
            }
        }

        if (key == "include")
        {
            readConfigFile(getRightConfigPath(value, basePath_));
        }
        else if (key == "name")
        {
            name_ = value.toLower();
        }
        else if (key == "version")
        {
            version_ = value;
        }
        else if (key == "info")
        {
            info_ = value;
        }
        else if (key == "priority")
        {
            priority_ = value.convert<int>();
        }
        else if (key == "freedriver")
        {
            value = value.toLower();

            if (value == "false")
            {
                freedriver_ = false;
            }
            else if (value == "true")
            {
                freedriver_ = true;
            }
        }
        else if (key == "classids")
        {
            // Add new HardwareIDs group to vector if vector is not empty
            if (!hwdIDs_.back().classIDs.empty())
            {
                Config::HardwareID hwdID;
                hwdIDs_.push_back(hwdID);
            }

            hwdIDs_.back().classIDs = splitValue(value);
        }
        else if (key == "vendorids")
        {
            // Add new HardwareIDs group to vector if vector is not empty
            if (!hwdIDs_.back().vendorIDs.empty())
            {
                Config::HardwareID hwdID;
                hwdIDs_.push_back(hwdID);
            }

            hwdIDs_.back().vendorIDs = splitValue(value);
        }
        else if (key == "deviceids")
        {
            // Add new HardwareIDs group to vector if vector is not empty
            if (!hwdIDs_.back().deviceIDs.empty())
            {
                Config::HardwareID hwdID;
                hwdIDs_.push_back(hwdID);
            }

            hwdIDs_.back().deviceIDs = splitValue(value);
        }
        else if (key == "blacklistedclassids")
        {
            hwdIDs_.back().blacklistedClassIDs = splitValue(value);
        }
        else if (key == "blacklistedvendorids")
        {
            hwdIDs_.back().blacklistedVendorIDs = splitValue(value);
        }
        else if (key == "blacklisteddeviceids")
        {
            hwdIDs_.back().blacklistedDeviceIDs = splitValue(value);
        }
        else if (key == "mhwddepends")
        {
            dependencies_ = splitValue(value);
        }
        else if (key == "mhwdconflicts")
        {
            conflicts_ = splitValue(value);
        }
    }

    // Append * to all empty vectors
    for (auto&& hwdID = hwdIDs_.begin();
            hwdID != hwdIDs_.end(); hwdID++)
    {
        if ((*hwdID).classIDs.empty())
        {
            (*hwdID).classIDs.push_back("*");
        }

        if ((*hwdID).vendorIDs.empty())
        {
            (*hwdID).vendorIDs.push_back("*");
        }

        if ((*hwdID).deviceIDs.empty())
        {
            (*hwdID).deviceIDs.push_back("*");
        }
    }

    if (name_.empty())
    {
        return false;
    }

    return true;
}

std::vector<std::string> Config::splitValue(Vita::string str, Vita::string onlyEnding)
{
    std::vector<Vita::string> work = str.toLower().explode(" ");
    std::vector<std::string> final;

    for (auto&& iterator = work.begin(); iterator != work.end();
            iterator++)
    {
        if (("" != *iterator) && onlyEnding.empty())
        {
            final.push_back(*iterator);
        }
        else if (("" != *iterator) && (Vita::string(*iterator).explode(".").back() == onlyEnding)
                && ((*iterator).size() > 5))
        {
            final.push_back(Vita::string(*iterator).substr(0, (*iterator).size() - 5));
        }
    }

    return final;
}

Vita::string Config::getRightConfigPath(Vita::string str, Vita::string baseConfigPath)
{
    str = str.trim();

    if ((str.size() <= 0) || ("/" == str.substr(0, 1)))
    {
        return str;
    }

    return baseConfigPath + "/" + str;
}
