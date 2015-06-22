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

#include "ConsoleWriter.hpp"

#include <hd.h>

#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

void ConsoleWriter::printStatus(std::string statusMsg) const
{
    std::cout << CONSOLE_RED_MESSAGE_COLOR << "> "
            << CONSOLE_COLOR_RESET << statusMsg << std::endl;
}

void ConsoleWriter::printError(std::string errorMsg) const
{
    std::cout << CONSOLE_RED_MESSAGE_COLOR << "Error: "
            << CONSOLE_COLOR_RESET << errorMsg << std::endl;
}

void ConsoleWriter::printWarning(std::string warningMsg) const
{
    std::cout << CONSOLE_RED_MESSAGE_COLOR << "Warning: "
            << CONSOLE_COLOR_RESET << warningMsg << std::endl;
}

void ConsoleWriter::printMessage(MHWD::MESSAGETYPE type, std::string msg) const
{
    switch(type)
    {
        case MHWD::MESSAGETYPE::CONSOLE_OUTPUT:
            std::cout << CONSOLE_TEXT_OUTPUT_COLOR << msg << CONSOLE_COLOR_RESET;
            break;
        case MHWD::MESSAGETYPE::INSTALLDEPENDENCY_START:
            printStatus("Installing dependency " + msg + "...");
            break;
        case MHWD::MESSAGETYPE::INSTALLDEPENDENCY_END:
            printStatus("Successfully installed dependency " + msg);
            break;
        case MHWD::MESSAGETYPE::INSTALL_START:
            printStatus("Installing " + msg + "...");
            break;
        case MHWD::MESSAGETYPE::INSTALL_END:
            printStatus("Successfully installed " + msg);
            break;
        case MHWD::MESSAGETYPE::REMOVE_START:
            printStatus("Removing " + msg + "...");
            break;
        case MHWD::MESSAGETYPE::REMOVE_END:
            printStatus("Successfully removed " + msg);
            break;
        default:
            printError("You shouldn't see this?! Unknown message type!");
            break;
    }
}

void ConsoleWriter::printHelp() const
{
    std::cout << "Usage: mhwd [OPTIONS] <config(s)>\n\n"
            << "  --pci\t\t\t\t\tlist only pci devices and driver configs\n"
            << "  --usb\t\t\t\t\tlist only usb devices and driver configs\n"
            << "  -h/--help\t\t\t\tshow help\n"
            << "  -v/--version\t\t\t\tshow version of mhwd\n"
            << "  -f/--force\t\t\t\tforce reinstallation\n"
            << "  -d/--detail\t\t\t\tshow detailed info for -l/-li/-lh\n"
            << "  -l/--list\t\t\t\tlist available configs for devices\n"
            << "  -la/--listall\t\t\t\tlist all driver configs\n"
            << "  -li/--listinstalled\t\t\tlist installed driver configs\n"
            << "  -lh/--listhardware\t\t\tlist hardware information\n"
            << "  -i/--install <usb/pci> <config(s)>\tinstall driver config(s)\n"
            << "  -ic/--installcustom <usb/pci> <path>\tinstall custom config(s)\n"
            << "  -r/--remove <usb/pci> <config(s)>\tremove driver config(s)\n"
            << "  -a/--auto <usb/pci> <free/nonfree> <classid>\tauto install configs for classid\n"
            << "  --pmcachedir <path>\t\t\tset package manager cache path\n"
            << "  --pmconfig <path>\t\t\tset package manager config\n"
            << "  --pmroot <path>\t\t\tset package manager root\n" << std::endl;
}

void ConsoleWriter::printVersion(std::string& versionMhwd, std::string& yearCopy) const
{
    std::cout << "Manjaro Hardware Detection v"<< versionMhwd <<"\n\n"
            << "Copyright (C) "<< yearCopy <<" Manjaro Linux Developers\n"
            << "This is free software licensed under GNU GPL v3.0\n"
            << "FITNESS FOR A PARTICULAR PURPOSE.\n"
            << std::endl;
}

void ConsoleWriter::listDevices(const std::vector<std::shared_ptr<Device>>& devices, std::string type) const
{
    if (devices.empty())
    {
        printWarning("No " + type + " devices found!");
    }
    else
    {
        printStatus(type + " devices:");
        printLine();
        std::cout << std::setw(30) << "TYPE"
                << std::setw(15) << "BUS"
                << std::setw(8) << "CLASS"
                << std::setw(8) << "VENDOR"
                << std::setw(8) << "DEVICE"
                << std::setw(10) << "CONFIGS" << std::endl;
        printLine();
        for (const auto& device : devices)
        {
            std::cout << std::setw(30) << device->className_
                    << std::setw(15) << device->sysfsBusID_
                    << std::setw(8) << device->classID_
                    << std::setw(8) << device->vendorID_
                    << std::setw(8) << device->deviceID_
                    << std::setw(10) << device->availableConfigs_.size() << std::endl;
        }
        std::cout << std::endl << std::endl;
    }
}

void ConsoleWriter::listConfigs(const std::vector<std::shared_ptr<Config>>& configs, std::string header) const
{
    printStatus(header);
    printLine();
    std::cout << std::setw(22) << "NAME"
            << std::setw(22) << "VERSION"
            << std::setw(20) << "FREEDRIVER"
            << std::setw(15) << "TYPE" << std::endl;
    printLine();
    for (const auto& config : configs)
    {
        std::cout << std::setw(22) << config->name_
                << std::setw(22) << config->version_
                << std::setw(20) << std::boolalpha << config->freedriver_
                << std::setw(15) << config->type_ << std::endl;
    }
    std::cout << std::endl << std::endl;
}

void ConsoleWriter::printAvailableConfigsInDetail(const std::string& deviceType,
        const std::vector<std::shared_ptr<Device>>& devices) const
{
    bool configFound = false;

    for (const auto& device : devices)
    {
        if (device->availableConfigs_.empty() && device->installedConfigs_.empty())
        {
            continue;
        }
        else
        {
            configFound = true;

            printLine();
            printStatus(
                    deviceType + " Device: " + device->sysfsID_ + " (" + device->classID_ + ":"
                    + device->vendorID_ + ":" + device->deviceID_ + ")");
            std::cout << "  " << device->className_
                    << " " << device->vendorName_
                    << " " << device->deviceName_ << std::endl;
            printLine();
            if (!device->installedConfigs_.empty())
            {
                std::cout << "  > INSTALLED:\n\n";
                for (auto&& installedConfig : device->installedConfigs_)
                {
                    printConfigDetails(*installedConfig);
                }
                std::cout << "\n\n";
            }
            if (!device->availableConfigs_.empty())
            {
                std::cout << "  > AVAILABLE:\n\n";
                for (auto&& availableConfig : device->availableConfigs_)
                {
                    printConfigDetails(*availableConfig);
                }
                std::cout << std::endl;
            }
        }
    }

    if (!configFound)
    {
        printWarning("no configs for " + deviceType + " devices found!");
    }
}

void ConsoleWriter::printInstalledConfigs(const std::string& deviceType,
        const std::vector<std::shared_ptr<Config>>& installedConfigs) const
{
    if (installedConfigs.empty())
    {
        printWarning("no installed configs for " + deviceType + " devices found!");
    }
    else
    {
        for (const auto& config : installedConfigs)
        {
            printConfigDetails(*config);
        }
        std::cout << std::endl;
    }
}

void ConsoleWriter::printConfigDetails(const Config& config) const
{
    std::string classids;
    std::string vendorids;
    for (const auto& hwd : config.hwdIDs_)
    {
        for (const auto& vendorID : hwd.vendorIDs)
        {
            vendorids += vendorID + " ";
        }

        for (const auto& classID : hwd.classIDs)
        {
            classids += classID + " ";
        }
    }
    std::string dependencies;
    for (const auto& dependency : config.dependencies_)
    {
        dependencies += dependency + " ";
    }
    std::string conflicts;
    for (const auto& conflict : config.conflicts_)
    {
        conflicts += conflict + " ";
    }

    std::cout << "   NAME:\t" << config.name_
            << "\n   ATTACHED:\t" << config.type_
            << "\n   VERSION:\t" << config.version_
            << "\n   INFO:\t" << (config.info_.empty() ? "-" : config.info_)
            << "\n   PRIORITY:\t" << config.priority_
            << "\n   FREEDRIVER:\t" << std::boolalpha << config.freedriver_
            << "\n   DEPENDS:\t" << (dependencies.empty() ? "-" : dependencies)
            << "\n   CONFLICTS:\t" << (conflicts.empty() ? "-" : conflicts)
            << "\n   CLASSIDS:\t" << classids
            << "\n   VENDORIDS:\t" << vendorids << "\n" << std::endl;
}

void ConsoleWriter::printLine() const
{
    std::cout << std::string(80, '-') << std::endl;
}

void ConsoleWriter::printDeviceDetails(hw_item hw, FILE *f) const
{
    std::unique_ptr<hd_data_t> hd_data{new hd_data_t()};
    hd_t *hd = hd_list(hd_data.get(), hw, 1, nullptr);

    for (hd_t* hdIter = hd; hdIter; hdIter = hdIter->next)
    {
        hd_dump_entry(hd_data.get(), hdIter, f);
    }

    hd_free_hd_list(hd);
    hd_free_hd_data(hd_data.get());
}
