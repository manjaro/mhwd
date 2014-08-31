/*
 * Data.cpp
 *
 *  Created on: 28 sie 2014
 *      Author: dec
 */

#include "Data.hpp"

#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <sstream>
Data::Data()
{
	fillDevices(MHWD::DEVICETYPE::PCI);
	fillDevices(MHWD::DEVICETYPE::USB);

	updateConfigData();
}

Data::~Data()
{
	for (auto PCIDevice : PCIDevices)
	{
		delete PCIDevice;
		PCIDevice = nullptr;
	}

	for (auto USBDevice : USBDevices)
	{
		delete USBDevice;
		USBDevice = nullptr;
	}

	for (auto installedPCIConfig : installedPCIConfigs)
	{
		delete installedPCIConfig;
		installedPCIConfig = nullptr;
	}

	for (auto installedUSBConfig : installedUSBConfigs)
	{
		delete installedUSBConfig;
		installedUSBConfig = nullptr;
	}

	for (auto PCIConfig : allPCIConfigs)
	{
		delete PCIConfig;
		PCIConfig = nullptr;
	}

	for (auto USBConfig : allUSBConfigs)
	{
		delete USBConfig;
		USBConfig = nullptr;
	}

	for (auto invalidConfig : invalidConfigs)
	{
		delete invalidConfig;
		invalidConfig = nullptr;
	}

	PCIDevices.clear();
	USBDevices.clear();
	installedPCIConfigs.clear();
	installedUSBConfigs.clear();
	allUSBConfigs.clear();
	allPCIConfigs.clear();
	invalidConfigs.clear();
}

void Data::updateInstalledConfigData()
{
	// Clear config vectors in each device element
	for (std::vector<Device*>::iterator PCIDevice = PCIDevices.begin();
			PCIDevice != PCIDevices.end(); PCIDevice++)
	{
		(*PCIDevice)->installedConfigs.clear();
	}

	for (std::vector<Device*>::iterator USBDevice = USBDevices.begin();
			USBDevice != USBDevices.end(); USBDevice++)
	{
		(*USBDevice)->installedConfigs.clear();
	}

	// Clear installed config vectors
	for (auto PCIConfig : installedPCIConfigs)
	{
		delete PCIConfig;
		PCIConfig = nullptr;
	}

	for (auto USBConfig : installedUSBConfigs)
	{
		delete USBConfig;
		USBConfig = nullptr;
	}

	installedPCIConfigs.clear();
	installedUSBConfigs.clear();

	// Refill data
	fillInstalledConfigs(MHWD::DEVICETYPE::PCI);
	fillInstalledConfigs(MHWD::DEVICETYPE::USB);

	setMatchingConfigs(&PCIDevices, &installedPCIConfigs, true);
	setMatchingConfigs(&USBDevices, &installedUSBConfigs, true);
}

void Data::fillInstalledConfigs(MHWD::DEVICETYPE type)
{
	std::vector<std::string> configPaths;
	std::vector<Config*>* configs;

	if (type == MHWD::DEVICETYPE::USB)
	{
		configs = &installedUSBConfigs;
		configPaths = getRecursiveDirectoryFileList(MHWD_USB_DATABASE_DIR, MHWD_CONFIG_NAME);
	}
	else
	{
		configs = &installedPCIConfigs;
		configPaths = getRecursiveDirectoryFileList(MHWD_PCI_DATABASE_DIR, MHWD_CONFIG_NAME);
	}

	for (std::vector<std::string>::const_iterator configPath = configPaths.begin();
			configPath != configPaths.end(); ++configPath)
	{
		Config *config = new Config((*configPath), type);

		if (config->readConfigFile((*configPath)))
		{
			configs->push_back(config);
		}
		else
		{
			invalidConfigs.push_back(config);
		}
	}
}

void Data::getAllDevicesOfConfig(Config *config, std::vector<Device*>* foundDevices)
{
	std::vector<Device*> devices;

	if (config->type_ == MHWD::DEVICETYPE::USB)
	{
		devices = USBDevices;
	}
	else
	{
		devices = PCIDevices;
	}

	getAllDevicesOfConfig(&devices, config, foundDevices);
}


void Data::getAllDevicesOfConfig(std::vector<Device*>* devices, Config *config,
		std::vector<Device*>* foundDevices)
{
	foundDevices->clear();

	for (std::vector<Config::HardwareIDs>::const_iterator hwdID = config->hwdIDs_.begin();
			hwdID != config->hwdIDs_.end(); ++hwdID)
	{
		bool foundDevice = false;
		// Check all devices
		for (std::vector<Device*>::iterator i_device = devices->begin(); i_device != devices->end();
				++i_device)
		{
			bool found = false;
			// Check class ids
			for (std::vector<std::string>::const_iterator classID = hwdID->classIDs.begin();
					classID != hwdID->classIDs.end(); ++classID)
			{
				if (*classID == "*" || *classID == (*i_device)->classID)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				continue;
			}
			else
			{
				// Check blacklisted class ids
				found = false;
				for (std::vector<std::string>::const_iterator blacklistedClassID =
						(*hwdID).blacklistedClassIDs.begin();
						blacklistedClassID != (*hwdID).blacklistedClassIDs.end(); ++blacklistedClassID)
				{
					if (*blacklistedClassID == (*i_device)->classID)
					{
						found = true;
						break;
					}
				}

				if (found)
				{
					continue;
				}
				else
				{
					// Check vendor ids
					found = false;

					for (std::vector<std::string>::const_iterator vendorID = hwdID->vendorIDs.begin();
							vendorID != hwdID->vendorIDs.end(); ++vendorID)
					{
						if (*vendorID == "*" || *vendorID == (*i_device)->vendorID)
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						continue;
					}
					else
					{
						// Check blacklisted vendor ids
						found = false;

						for (std::vector<std::string>::const_iterator blacklistedVendorID =
								hwdID->blacklistedVendorIDs.begin();
								blacklistedVendorID != hwdID->blacklistedVendorIDs.end(); ++blacklistedVendorID)
						{
							if (*blacklistedVendorID == (*i_device)->vendorID)
							{
								found = true;
								break;
							}
						}

						if (found)
						{
							continue;
						}
						else
						{
							// Check device ids
							found = false;

							for (std::vector<std::string>::const_iterator deviceID = hwdID->deviceIDs.begin();
									deviceID != hwdID->deviceIDs.end(); ++deviceID)
							{
								if (*deviceID == "*" || *deviceID == (*i_device)->deviceID)
								{
									found = true;
									break;
								}
							}

							if (!found)
							{
								continue;
							}
							else
							{
								// Check blacklisted device ids
								found = false;

								for (std::vector<std::string>::const_iterator blacklistedDeviceID =
										hwdID->blacklistedDeviceIDs.begin();
										blacklistedDeviceID != hwdID->blacklistedDeviceIDs.end(); ++blacklistedDeviceID)
								{
									if (*blacklistedDeviceID == (*i_device)->deviceID)
									{
										found = true;
										break;
									}
								}

								if (found)
								{
									continue;
								}
								else
								{
									foundDevice = true;
									foundDevices->push_back((*i_device));
								}
							}
						}
					}
				}
			}
		}

		if (!foundDevice)
		{
			foundDevices->clear();
			return;
		}
	}
}


std::vector<Config*> Data::getAllDependenciesToInstall(Config *config)
{
	std::vector<Config*> depends;
	std::vector<Config*> installedConfigs;

	// Get the right configs
	if (config->type_ == MHWD::DEVICETYPE::USB)
	{
		installedConfigs = installedUSBConfigs;
	}
	else
	{
		installedConfigs = installedPCIConfigs;
	}

	// Get all depends
	getAllDependenciesToInstall(config, &installedConfigs, &depends);

	return depends;
}

void Data::getAllDependenciesToInstall(Config *config,
		std::vector<Config*>* installedConfigs, std::vector<Config*> *dependencies)
{
	for (std::vector<std::string>::const_iterator configDependency = config->dependencies_.begin();
			configDependency != config->dependencies_.end(); configDependency++)
	{
		bool found = false;

		for (std::vector<Config*>::const_iterator installedConfig = installedConfigs->begin();
				installedConfig != installedConfigs->end(); installedConfig++)
		{
			if ((*configDependency) == (*installedConfig)->name_)
			{
				found = true;
				break;
			}
		}

		if (found)
			continue;

		// Check if already in vector
		for (std::vector<Config*>::const_iterator dependency = dependencies->begin();
				dependency != dependencies->end(); dependency++)
		{
			if ((*dependency)->name_ == (*configDependency))
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			continue;
		}
		else
		{
			// Add to vector and check for further subdepends...
			Config *dependconfig = getDatabaseConfig((*configDependency), config->type_);
			if (dependconfig == nullptr)
			{
				continue;
			}
			else
			{
				dependencies->push_back(dependconfig);
				getAllDependenciesToInstall(dependconfig, installedConfigs, dependencies);
			}
		}
	}
}

Config* Data::getDatabaseConfig(const std::string configName,
		const MHWD::DEVICETYPE configType)
{
	std::vector<Config*> allConfigs;

	// Get the right configs
	if (configType == MHWD::DEVICETYPE::USB)
	{
		allConfigs = allUSBConfigs;
	}
	else
	{
		allConfigs = allPCIConfigs;
	}

	for (std::vector<Config*>::iterator config = allConfigs.begin(); config != allConfigs.end();
			config++)
	{
		if (configName == (*config)->name_)
		{
			return (*config);
		}
	}

	return nullptr;
}

std::vector<Config*> Data::getAllLocalConflicts(Config *config)
{
	std::vector<Config*> conflicts;
	std::vector<Config*> dependencies = getAllDependenciesToInstall(config);
	std::vector<Config*> installedConfigs;

	// Get the right configs
	if (config->type_ == MHWD::DEVICETYPE::USB)
	{
		installedConfigs = installedUSBConfigs;
	}
	else
	{
		installedConfigs = installedPCIConfigs;
	}

	dependencies.push_back(config);

	for (std::vector<Config*>::const_iterator dependency = dependencies.begin();
			dependency != dependencies.end(); dependency++)
	{
		for (std::vector<std::string>::const_iterator dependencyConflict =
				(*dependency)->conflicts_.begin();
				dependencyConflict != (*dependency)->conflicts_.end(); dependencyConflict++)
		{
			for (std::vector<Config*>::const_iterator installedConfig = installedConfigs.begin();
					installedConfig != installedConfigs.end(); installedConfig++)
			{
				if ((*dependencyConflict) != (*installedConfig)->name_)
				{
					continue;
				}
				else
				{
					// Check if already in vector
					bool found = false;
					for (std::vector<Config*>::const_iterator conflict = conflicts.begin();
							conflict != conflicts.end(); conflict++)
					{
						if ((*conflict)->name_ == (*dependencyConflict))
						{
							found = true;
							break;
						}
					}

					if (found)
					{
						continue;
					}
					else
					{
						conflicts.push_back((*installedConfig));
						break;
					}
				}
			}
		}
	}

	return conflicts;
}

std::vector<Config*> Data::getAllLocalRequirements(Config *config)
{
	std::vector<Config*> requirements;
	std::vector<Config*> installedConfigs;

	// Get the right configs
	if (config->type_ == MHWD::DEVICETYPE::USB)
	{
		installedConfigs = installedUSBConfigs;
	}
	else
	{
		installedConfigs = installedPCIConfigs;
	}

	// Check if this config is required by another installed config
	for (std::vector<Config*>::const_iterator installedConfig = installedConfigs.begin();
			installedConfig != installedConfigs.end(); installedConfig++)
	{
		for (std::vector<std::string>::const_iterator dependency =
				(*installedConfig)->dependencies_.begin();
				dependency != (*installedConfig)->dependencies_.end(); dependency++)
		{
			if ((*dependency) != config->name_)
			{
				continue;
			}
			else
			{
				// Check if already in vector
				bool found = false;
				for (std::vector<Config*>::const_iterator requirement = requirements.begin();
						requirement != requirements.end(); requirement++)
				{
					if ((*requirement)->name_ == (*installedConfig)->name_)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					requirements.push_back((*installedConfig));
					break;
				}
			}
		}
	}

	return requirements;
}


void Data::fillDevices(MHWD::DEVICETYPE type)
{
	hd_data_t *hd_data;
	hd_t *hd;
	hw_item hw;
	std::vector<Device*>* devices;

	if (type == MHWD::DEVICETYPE::USB)
	{
		hw = hw_usb;
		devices = &USBDevices;
	}
	else
	{
		hw = hw_pci;
		devices = &PCIDevices;
	}

	// Get the hardware devices
	hd_data = (hd_data_t*) calloc(1, sizeof *hd_data);
	hd = hd_list(hd_data, hw, 1, nullptr);

	Device *device;
	for (; hd; hd = hd->next)
	{
		device = new Device();
		device->type = type;
		device->classID = from_Hex(hd->base_class.id, 2) + from_Hex(hd->sub_class.id, 2).toLower();
		device->vendorID = from_Hex(hd->vendor.id, 4).toLower();
		device->deviceID = from_Hex(hd->device.id, 4).toLower();
		device->className = from_CharArray(hd->base_class.name);
		device->vendorName = from_CharArray(hd->vendor.name);
		device->deviceName = from_CharArray(hd->device.name);
		device->sysfsBusID = from_CharArray(hd->sysfs_bus_id);
		device->sysfsID = from_CharArray(hd->sysfs_id);
		devices->push_back(device);
	}

	hd_free_hd_list(hd);
	hd_free_hd_data(hd_data);
	free(hd_data);
}

void Data::fillAllConfigs(MHWD::DEVICETYPE type)
{
	std::vector<std::string> configPaths;
	std::vector<Config*>* configs;

	if (type == MHWD::DEVICETYPE::USB)
	{
		configs = &allUSBConfigs;
		configPaths = getRecursiveDirectoryFileList(MHWD_USB_CONFIG_DIR, MHWD_CONFIG_NAME);
	}
	else
	{
		configs = &allPCIConfigs;
		configPaths = getRecursiveDirectoryFileList(MHWD_PCI_CONFIG_DIR, MHWD_CONFIG_NAME);
	}

	for (std::vector<std::string>::const_iterator configPath = configPaths.begin();
			configPath != configPaths.end(); ++configPath)
	{
		Config *config = new Config((*configPath), type);

		if (config->readConfigFile((*configPath)))
		{
			configs->push_back(config);
		}
		else
		{
			invalidConfigs.push_back(config);
		}
	}
}

bool Data::fillConfig(Config *config, std::string configPath, MHWD::DEVICETYPE type)
{
	config->type_ = type;
	config->priority_ = 0;
	config->freedriver_ = true;
	config->basePath_ = configPath.substr(0, configPath.find_last_of('/'));
	config->configPath_ = configPath;

	// Add new HardwareIDs group to vector if vector is empty
	if (config->hwdIDs_.empty())
	{
		Config::HardwareIDs hwdID;
		config->hwdIDs_.push_back(hwdID);
	}

	return readConfigFile(config, config->configPath_);
}

std::vector<std::string> Data::getRecursiveDirectoryFileList(const std::string directoryPath,
		std::string onlyFilename)
{
	std::vector<std::string> list;
	struct dirent *dir;
	DIR *d = opendir(directoryPath.c_str());

	if (d)
	{
		while ((dir = readdir(d)) != nullptr)
		{
			std::string filename = std::string(dir->d_name);
			std::string filepath = directoryPath + "/" + filename;

			if (filename == "." || filename == ".." || filename == "")
				continue;

			struct stat filestatus;
			lstat(filepath.c_str(), &filestatus);

			if (S_ISREG(filestatus.st_mode) && (onlyFilename.empty() || onlyFilename == filename))
			{
				list.push_back(filepath);
			}
			else if (S_ISDIR(filestatus.st_mode))
			{
				std::vector<std::string> templist = getRecursiveDirectoryFileList(filepath,
						onlyFilename);

				for (std::vector<std::string>::const_iterator iterator = templist.begin();
						iterator != templist.end(); iterator++)
				{
					list.push_back((*iterator));
				}
			}
		}

		closedir(d);
	}

	return list;
}

bool Data::readConfigFile(Config *config, std::string configPath)
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
		getline(file, line);

		size_t pos = line.find_first_of('#');
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
		if (value.size() > 1 && value.substr(0, 1) == ">")
		{
			std::ifstream file(getRightConfigPath(value.substr(1), config->basePath_).c_str(),
					std::ios::in);
			if (!file.is_open())
			{
				return false;
			}

			Vita::string line;
			value.clear();

			while (!file.eof())
			{
				getline(file, line);

				size_t pos = line.find_first_of('#');
				if (pos != std::string::npos)
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
			readConfigFile(config, getRightConfigPath(value, config->basePath_));
		}
		else if (key == "name")
		{
			config->name_ = value.toLower();
		}
		else if (key == "version")
		{
			config->version_ = value;
		}
		else if (key == "info")
		{
			config->info_ = value;
		}
		else if (key == "priority")
		{
			config->priority_ = value.convert<int>();
		}
		else if (key == "freedriver")
		{
			value = value.toLower();

			if (value == "false")
				config->freedriver_ = false;
			else if (value == "true")
				config->freedriver_ = true;
		}
		else if (key == "classids")
		{
			// Add new HardwareIDs group to vector if vector is not empty
			if (!config->hwdIDs_.back().classIDs.empty())
			{
				Config::HardwareIDs hwdID;
				config->hwdIDs_.push_back(hwdID);
			}

			config->hwdIDs_.back().classIDs = splitValue(value);
		}
		else if (key == "vendorids")
		{
			// Add new HardwareIDs group to vector if vector is not empty
			if (!config->hwdIDs_.back().vendorIDs.empty())
			{
				Config::HardwareIDs hwdID;
				config->hwdIDs_.push_back(hwdID);
			}

			config->hwdIDs_.back().vendorIDs = splitValue(value);
		}
		else if (key == "deviceids")
		{
			// Add new HardwareIDs group to vector if vector is not empty
			if (!config->hwdIDs_.back().deviceIDs.empty())
			{
				Config::HardwareIDs hwdID;
				config->hwdIDs_.push_back(hwdID);
			}

			config->hwdIDs_.back().deviceIDs = splitValue(value);
		}
		else if (key == "blacklistedclassids")
		{
			config->hwdIDs_.back().blacklistedClassIDs = splitValue(value);
		}
		else if (key == "blacklistedvendorids")
		{
			config->hwdIDs_.back().blacklistedVendorIDs = splitValue(value);
		}
		else if (key == "blacklisteddeviceids")
		{
			config->hwdIDs_.back().blacklistedDeviceIDs = splitValue(value);
		}
		else if (key == "mhwddepends")
		{
			config->dependencies_ = splitValue(value);
		}
		else if (key == "mhwdconflicts")
		{
			config->conflicts_ = splitValue(value);
		}
	}

	// Append * to all empty vectors
	for (std::vector<Config::HardwareIDs>::iterator hwdID = config->hwdIDs_.begin();
			hwdID != config->hwdIDs_.end(); hwdID++)
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

	if (config->name_.empty())
	{
		return false;
	}

	return true;
}

Vita::string Data::getRightConfigPath(Vita::string str, Vita::string baseConfigPath)
{
	str = str.trim();

	if (str.size() <= 0 || str.substr(0, 1) == "/")
		return str;

	return baseConfigPath + "/" + str;
}

std::vector<std::string> Data::splitValue(Vita::string str, Vita::string onlyEnding)
{
	std::vector<Vita::string> work = str.toLower().explode(" ");
	std::vector<std::string> final;

	for (std::vector<Vita::string>::const_iterator iterator = work.begin(); iterator != work.end();
			iterator++)
	{
		if (*iterator != "" && onlyEnding.empty())
			final.push_back(*iterator);
		else if (*iterator != "" && Vita::string(*iterator).explode(".").back() == onlyEnding
				&& (*iterator).size() > 5)
			final.push_back(Vita::string(*iterator).substr(0, (*iterator).size() - 5));
	}

	return final;
}

void Data::updateConfigData()
{
	// Clear config vectors in each device element
	for (std::vector<Device*>::iterator PCIDevice = PCIDevices.begin();
			PCIDevice != PCIDevices.end(); PCIDevice++)
	{
		(*PCIDevice)->availableConfigs.clear();
	}

	for (std::vector<Device*>::iterator USBDevice = USBDevices.begin();
			USBDevice != USBDevices.end(); USBDevice++)
	{
		(*USBDevice)->availableConfigs.clear();
	}

	// Clear installed config vectors
	for (auto PCIConfig : allPCIConfigs)
	{
		delete PCIConfig;
		PCIConfig = nullptr;
	}

	for (auto USBConfig : allUSBConfigs)
	{
		delete USBConfig;
		USBConfig = nullptr;
	}

	allPCIConfigs.clear();
	allUSBConfigs.clear();

	// Refill data
	fillAllConfigs(MHWD::DEVICETYPE::PCI);
	fillAllConfigs(MHWD::DEVICETYPE::USB);

	setMatchingConfigs(&PCIDevices, &allPCIConfigs, false);
	setMatchingConfigs(&USBDevices, &allUSBConfigs, false);

	// Update also installed config data
	updateInstalledConfigData();
}

void Data::setMatchingConfigs(std::vector<Device*>* devices, std::vector<Config*>* configs,
		bool setAsInstalled)
{
	for (std::vector<Config*>::iterator config = configs->begin(); config != configs->end();
			++config)
	{
		setMatchingConfig((*config), devices, setAsInstalled);
	}
}

void Data::setMatchingConfig(Config* config, std::vector<Device*>* devices, bool setAsInstalled)
{
	std::vector<Device*> foundDevices;

	getAllDevicesOfConfig(devices, config, &foundDevices);

	// Set config to all matching devices
	for (std::vector<Device*>::iterator foundDevice = foundDevices.begin();
			foundDevice != foundDevices.end(); ++foundDevice)
	{
		if (setAsInstalled)
		{
			addConfigSorted(&(*foundDevice)->installedConfigs, config);
		}
		else
		{
			addConfigSorted(&(*foundDevice)->availableConfigs, config);
		}
	}
}

void Data::addConfigSorted(std::vector<Config*>* configs, Config* config)
{
	for (std::vector<Config*>::const_iterator iterator = configs->begin();
			iterator != configs->end(); iterator++)
	{
		if (config->name_ == (*iterator)->name_)
		{
			return;
		}
	}

	for (std::vector<Config*>::iterator iterator = configs->begin(); iterator != configs->end();
			iterator++)
	{
		if (config->priority_ > (*iterator)->priority_)
		{
			configs->insert(iterator, config);
			return;
		}
	}

	configs->push_back(config);
}

Vita::string Data::from_Hex(uint16_t hexnum, int fill)
{
	std::stringstream stream;
	stream << std::hex << std::setfill('0') << std::setw(fill) << hexnum;
	return stream.str();
}

Vita::string Data::from_CharArray(char* c)
{
	if (c == nullptr)
	{
		return "";
	}

	return Vita::string(c);
}
