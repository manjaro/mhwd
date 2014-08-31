/*
 * Printer.cpp
 *
 *  Created on: 28 sie 2014
 *      Author: dec
 */

#define CONSOLE_COLOR_RESET "\e[m"
#define CONSOLE_MESSAGE_COLOR "\e[1m\e[31m"
#define CONSOLE_TEXT_OUTPUT_COLOR "\e[0;32m"

#include "Printer.hpp"
#include <iomanip>
#include <iostream>

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
			<< "  --usb\t\t\t\t\tlist only usb devices and driver configsn"
			<< "  -h/--help\t\t\t\tshow help\n" << "  -f/--force\t\t\t\tforce reinstallation\n"
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

void Printer::listDevices(std::vector<Device*> devices, std::string type) const
{
	if (devices.empty())
	{
		printWarning("No " + type + " devices found!");
		return;
	}

	printStatus(type + " devices:");

	std::cout << std::endl << std::setfill('-') << std::setw(80) << "-" << std::setfill(' ')
			<< std::endl;

	std::cout << std::setw(30) << "TYPE"
			<< std::setw(15) << "BUS"
			<< std::setw(8) << "CLASS"
			<< std::setw(8) << "VENDOR"
			<< std::setw(8) << "DEVICE"
			<< std::setw(10) << "CONFIGS";

	std::cout << std::endl << std::setfill('-') << std::setw(80) << "-" << std::setfill(' ')
			<< std::endl;

	for (auto device : devices)
	{
		std::cout << std::setw(30) << device->className
				<< std::setw(15) << device->sysfsBusID
				<< std::setw(8) << device->classID
				<< std::setw(8) << device->vendorID
				<< std::setw(8) << device->deviceID
				<< std::setw(10) << device->availableConfigs.size() << std::endl;
	}
	std::cout << std::endl << std::endl;
}

void Printer::listConfigs(std::vector<Config*> configs, std::string beg, std::string empty) const
{
	if (configs.empty())
	{
		if (!empty.empty())
		{
			printWarning(empty);
		}
		else
		{
			return;
		}
	}
	else
	{
		printStatus(beg);

		std::cout << std::endl << std::setfill('-') << std::setw(80) << "-" << std::setfill(' ')
				<< std::endl;
		std::cout << std::setw(22) << "NAME"
				<< std::setw(22) << "VERSION"
				<< std::setw(20) << "FREEDRIVER"
				<< std::setw(15) << "TYPE" << std::endl;
		std::cout << std::setfill('-') << std::setw(80) << "-" << std::setfill(' ') << std::endl;

		for (auto config : configs)
		{
			std::string freedriver;
			std::string type;

			if (config->freedriver_)
			{
				freedriver = "true";
			}
			else
			{
				freedriver = "false";
			}

			if (config->type_ == MHWD::DEVICETYPE::USB)
			{
				type = "USB";
			}
			else
			{
				type = "PCI";
			}

			std::cout << std::setw(22) << config->name_
					<< std::setw(22) << config->version_
					<< std::setw(20) << freedriver
					<< std::setw(15) << type << std::endl;
		}

		std::cout << std::endl << std::endl;
	}
}

void Printer::printAvailableConfigs(MHWD::DEVICETYPE type, std::vector<Device*> devices) const
{
	std::string beg;

	if (type == MHWD::DEVICETYPE::USB)
	{
		beg = "USB";
	}
	else
	{
		beg = "PCI";
	}

	bool found = false;

	for (auto device : devices)
	{
		if (device->availableConfigs.empty() && device->installedConfigs.empty())
		{
			continue;
		}
		else
		{
			found = true;

			std::cout << std::endl << std::setfill('-') << std::setw(80) << "-" << std::setfill(' ')
			<< std::endl;

			printStatus(
					beg + " Device: " + device->sysfsID + " (" + device->classID + ":"
					+ device->vendorID + ":" + device->deviceID + ")");

			std::cout << "  " << device->className
					<< " " << device->vendorName
					<< " " << device->deviceName << std::endl;

			std::cout << std::setfill('-') << std::setw(80) << "-" << std::setfill(' ') << std::endl;

			if (!device->installedConfigs.empty())
			{
				std::cout << "  > INSTALLED:" << std::endl;
				for (auto installedConfig : device->installedConfigs)
				{
					printConfigDetails(*installedConfig);
				}
				std::cout << std::endl << std::endl;
			}

			if (!device->availableConfigs.empty())
			{
				std::cout << "  > AVAILABLE:" << std::endl;
				for (auto availableConfig : device->availableConfigs)
				{
					printConfigDetails(*availableConfig);
				}
				std::cout << std::endl;
			}
		}
	}

	if (!found)
		printWarning("no configs for " + beg + " devices found!");
}

void Printer::printInstalledConfigs(MHWD::DEVICETYPE type, std::vector<Config*> installedConfigs) const
{
	std::string beg;

	if (type == MHWD::DEVICETYPE::USB)
	{
		beg = "USB";
	}
	else
	{
		beg = "PCI";
	}

	if (installedConfigs.empty())
	{
		printWarning("no installed configs for " + beg + " devices found!");
		return;
	}

	for (auto config : installedConfigs)
	{
		printConfigDetails(*config);
	}

	std::cout << std::endl;
}

void Printer::printConfigDetails(const Config& config) const
{
	std::string type;
	std::string dependencies;
	std::string conflicts;
	std::string info;
	std::string classids;
	std::string vendorids;

	if (config.type_ == MHWD::DEVICETYPE::USB)
	{
		type = "USB";
	}
	else
	{
		type = "PCI";
	}

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

	std::cout << "   NAME:\t" << config.name_ << std::endl
			<< "   ATTACHED:\t" << type << std::endl
			<< "   VERSION:\t" << config.version_ << std::endl
			<< "   INFO:\t" << info << std::endl
			<< "   PRIORITY:\t" << config.priority_ << std::endl
			<< "   FREEDRIVER:\t" << std::boolalpha << config.freedriver_ << std::endl
			<< "   DEPENDS:\t" << dependencies << std::endl
			<< "   CONFLICTS:\t" << conflicts << std::endl
			<< "   CLASSIDS:\t" << classids << std::endl
			<< "   VENDORIDS:\t" << vendorids << std::endl;
}
