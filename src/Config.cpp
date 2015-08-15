/*
 *  This file is part of the mhwd - Manjaro Hardware Detection project
 *
 *  mhwd - Manjaro Hardware Detection
 *  Roland Singer <roland@manjaro.org>
 *  Łukasz Matysiak <december0123@gmail.com>
 *  Filipe Marques <eagle.software3@gmail.com>
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

#include "Config.hpp"
#include "Utils.hpp"

#include <fstream>
#include <string>
#include <vector>

Config::Config(std::string configPath, std::string type)
    : type_(type), basePath_(configPath.substr(0, configPath.find_last_of('/'))),
      configPath_(configPath), hwdIDs_(1)
{}

bool Config::readConfigFile(std::string configPath)
{
    std::ifstream file(configPath);

    if (!file)
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

        switch(MhwdUtils::hash(std::string(key).c_str()))
        {
            case MhwdUtils::hash_compile_time("include"):
                readConfigFile(getRightConfigPath(value, basePath_));
                break;
            case MhwdUtils::hash_compile_time("name"):
                name_ = value.toLower();
                break;
            case MhwdUtils::hash_compile_time("version"):
                version_ = value;
                break;
            case MhwdUtils::hash_compile_time("info"):
                info_ = value;
                break;
            case MhwdUtils::hash_compile_time("priority"):
                priority_ = value.convert<int>();
                break;
            case MhwdUtils::hash_compile_time("freedriver"):
                value = value.toLower();
                freedriver_ = value == "false" ? false : true;
                break;
            case MhwdUtils::hash_compile_time("classids"):
                // Add new HardwareIDs group to vector if vector is not empty
                if (!hwdIDs_.back().classIDs.empty())
                {
                    Config::HardwareID hwdID;
                    hwdIDs_.push_back(hwdID);
                }

                hwdIDs_.back().classIDs = splitValue(value);
                break;
            case MhwdUtils::hash_compile_time("vendorids"):
                // Add new HardwareIDs group to vector if vector is not empty
                if (!hwdIDs_.back().vendorIDs.empty())
                {
                    Config::HardwareID hwdID;
                    hwdIDs_.push_back(hwdID);
                }

                hwdIDs_.back().vendorIDs = splitValue(value);
                break;
            case MhwdUtils::hash_compile_time("deviceids"):
                // Add new HardwareIDs group to vector if vector is not empty
                if (!hwdIDs_.back().deviceIDs.empty())
                {
                    Config::HardwareID hwdID;
                    hwdIDs_.push_back(hwdID);
                }

                hwdIDs_.back().deviceIDs = splitValue(value);
                break;
            case MhwdUtils::hash_compile_time("blacklistedclassids"):
                hwdIDs_.back().blacklistedClassIDs = splitValue(value);
                break;
            case MhwdUtils::hash_compile_time("blacklistedvendorids"):
                hwdIDs_.back().blacklistedVendorIDs = splitValue(value);
                break;
            case MhwdUtils::hash_compile_time("blacklisteddeviceids"):
                hwdIDs_.back().blacklistedDeviceIDs = splitValue(value);
                break;
            case MhwdUtils::hash_compile_time("mhwddepends"):
                dependencies_ = splitValue(value);
                break;
            case MhwdUtils::hash_compile_time("mhwdconflicts"):
                conflicts_ = splitValue(value);
                break;
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

    return ! name_.empty();
}

std::vector<std::string> Config::splitValue(Vita::string str, Vita::string onlyEnding)
{
    std::vector<Vita::string> work = str.toLower().explode(" ");
    std::vector<std::string> final;

    for (auto&& iterator = work.begin(); iterator != work.end();
            ++iterator)
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
