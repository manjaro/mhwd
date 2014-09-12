/*
 * Printer.cpp
 *
 *  Created on: 28 sie 2014
 *      Author: dec
 */

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "Printer.hpp"

Printer::Printer()
{
}

Printer::~Printer()
{
}

void Printer::printStatus(std::string statusMsg) const
{
    std::cout << CONSOLE_MESSAGE_COLOR << "> "
            << CONSOLE_COLOR_RESET << statusMsg << std::endl;
}

void Printer::printError(std::string errorMsg) const
{
    std::cout << CONSOLE_MESSAGE_COLOR << "Error: "
            << CONSOLE_COLOR_RESET << errorMsg << std::endl;
}

void Printer::printWarning(std::string warningMsg) const
{
    std::cout << CONSOLE_MESSAGE_COLOR << "Warning: "
            << CONSOLE_COLOR_RESET << warningMsg << std::endl;
}

void Printer::printMessage(MHWD::MESSAGETYPE type, std::string msg) const
{
    if (type == MHWD::MESSAGETYPE::CONSOLE_OUTPUT)
    {
        std::cout << CONSOLE_TEXT_OUTPUT_COLOR << msg << CONSOLE_COLOR_RESET;
    }
    else if (type == MHWD::MESSAGETYPE::INSTALLDEPENDENCY_START)
    {
        printStatus("Installing dependency " + msg + "...");
    }
    else if (type == MHWD::MESSAGETYPE::INSTALLDEPENDENCY_END)
    {
        printStatus("Successfully installed dependency " + msg);
    }
    else if (type == MHWD::MESSAGETYPE::INSTALL_START)
    {
        printStatus("Installing " + msg + "...");
    }
    else if (type == MHWD::MESSAGETYPE::INSTALL_END)
    {
        printStatus("Successfully installed " + msg);
    }
    else if (type == MHWD::MESSAGETYPE::REMOVE_START)
    {
        printStatus("Removing " + msg + "...");
    }
    else if (type == MHWD::MESSAGETYPE::REMOVE_END)
    {
        printStatus("Successfully removed " + msg);
    }
    else
    {
        printError("You shouldn't see this?! Unknown message type!");
    }
}

void Printer::printHelp() const
{
    std::cout << "Usage: mhwd [OPTIONS] <config(s)>\n\n"
            << "  --pci\t\t\t\t\tlist only pci devices and driver configs\n"
            << "  --usb\t\t\t\t\tlist only usb devices and driver configs\n"
            << "  -h/--help\t\t\t\tshow help\n"
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

void Printer::listDevices(const std::vector<Device*>& devices, std::string type) const
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
        for (auto device : devices)
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

void Printer::listConfigs(const std::vector<Config*>& configs, std::string beg,
        std::string empty) const
{
    if (configs.empty())
    {
        if (!empty.empty())
        {
            printWarning(empty);
        }
    }
    else
    {
        printStatus(beg);
        printLine();
        std::cout << std::setw(22) << "NAME"
                << std::setw(22) << "VERSION"
                << std::setw(20) << "FREEDRIVER"
                << std::setw(15) << "TYPE" << std::endl;
        printLine();
        for (auto config : configs)
        {
            std::cout << std::setw(22) << config->name_
                    << std::setw(22) << config->version_
                    << std::setw(20) << std::boolalpha << config->freedriver_
                    << std::setw(15) << config->type_ << std::endl;
        }
        std::cout << std::endl << std::endl;
    }
}

void Printer::printAvailableConfigs(const std::string& deviceType,
        const std::vector<Device*>& devices) const
{
    bool configFound = false;

    for (auto device : devices)
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
                std::cout << "  > INSTALLED:" << std::endl;
                for (auto installedConfig : device->installedConfigs_)
                {
                    printConfigDetails(*installedConfig);
                }
                std::cout << std::endl << std::endl;
            }
            if (!device->availableConfigs_.empty())
            {
                std::cout << "  > AVAILABLE:" << std::endl;
                for (auto availableConfig : device->availableConfigs_)
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

void Printer::printInstalledConfigs(const std::string& deviceType,
        const std::vector<Config*>& installedConfigs) const
{
    if (installedConfigs.empty())
    {
        printWarning("no installed configs for " + deviceType + " devices found!");
    }
    else
    {
        for (auto config : installedConfigs)
        {
            printConfigDetails(*config);
        }
        std::cout << std::endl;
    }
}

void Printer::printConfigDetails(const Config& config) const
{
    std::string dependencies;
    std::string conflicts;
    std::string info;
    std::string classids;
    std::string vendorids;

    for (auto hwd : config.hwdIDs_)
    {
        for (auto vendorID : hwd.vendorIDs)
        {
            vendorids += vendorID + " ";
        }

        for (auto classID : hwd.classIDs)
        {
            classids += classID + " ";
        }
    }

    for (auto dependency : config.dependencies_)
    {
        dependencies += dependency + " ";
    }

    for (auto conflict : config.conflicts_)
    {
        conflicts += conflict + " ";
    }

    if (dependencies.empty())
    {
        dependencies = "-";
    }

    if (conflicts.empty())
    {
        conflicts = "-";
    }

    info = config.info_;
    if (info.empty())
    {
        info = "-";
    }

    std::cout << "   NAME:\t" << config.name_
            << "\n   ATTACHED:\t" << config.type_
            << "\n   VERSION:\t" << config.version_
            << "\n   INFO:\t" << info
            << "\n   PRIORITY:\t" << config.priority_
            << "\n   FREEDRIVER:\t" << std::boolalpha << config.freedriver_
            << "\n   DEPENDS:\t" << dependencies
            << "\n   CONFLICTS:\t" << conflicts
            << "\n   CLASSIDS:\t" << classids
            << "\n   VENDORIDS:\t" << vendorids << std::endl;
}

void Printer::printLine() const
{
    std::cout << std::setfill('-') << std::setw(80) << "-" << std::setfill(' ') << std::endl;
}
