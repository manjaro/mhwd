/*
 * Data.hpp
 *
 *  Created on: 28 sie 2014
 *      Author: dec
 */

#ifndef DATA_HPP_
#define DATA_HPP_

#include "../libmhwd/const.h"
#include "../libmhwd/vita/string.hpp"
#include "Config.hpp"
#include "Device.hpp"

#include <hd.h>
#include <sys/stat.h>
#include <sys/types.h>


class Data {
public:
	Data();
	~Data();

    struct Environment {
    		std::string PMCachePath {MHWD_PM_CACHE_DIR};
    		std::string PMConfigPath {MHWD_PM_CONFIG};
    		std::string PMRootPath {MHWD_PM_ROOT};
    		bool syncPackageManagerDatabase = true;
    	};
	Environment environment;
	std::vector<Device*> USBDevices;
	std::vector<Device*> PCIDevices;
	std::vector<Config*> installedUSBConfigs;
	std::vector<Config*> installedPCIConfigs;
	std::vector<Config*> allUSBConfigs;
	std::vector<Config*> allPCIConfigs;
	std::vector<Config*> invalidConfigs;

	void updateInstalledConfigData();
	void getAllDevicesOfConfig(Config *config, std::vector<Device*>* foundDevices);
	bool fillConfig(Config *config, std::string configPath, std::string type);

	std::vector<Config*> getAllDependenciesToInstall(Config *config);
	void getAllDependenciesToInstall(Config *config, std::vector<Config*>* installedConfigs,
			std::vector<Config*> *depends);
	Config* getDatabaseConfig(const std::string configName, const std::string configType);
	std::vector<Config*> getAllLocalConflicts(Config *config);
	std::vector<Config*> getAllLocalRequirements(Config *config);

private:
	void getAllDevicesOfConfig(std::vector<Device*>* devices, Config *config, std::vector<Device*>* foundDevices);
	void fillInstalledConfigs(std::string type);
	void fillDevices(std::string type);
	void fillAllConfigs(std::string type);
    void setMatchingConfigs(std::vector<Device*>* devices, std::vector<Config*>* configs, bool setAsInstalled);
    void setMatchingConfig(Config* config, std::vector<Device*>* devices, bool setAsInstalled);
    void addConfigSorted(std::vector<Config*>* configs, Config* config);
	std::vector<std::string> getRecursiveDirectoryFileList(const std::string directoryPath,
			std::string onlyFilename = "");

	bool readConfigFile(Config *config, std::string configPath);
	Vita::string getRightConfigPath(Vita::string str, Vita::string baseConfigPath);
	std::vector<std::string> splitValue(Vita::string str, Vita::string onlyEnding = "");
	void updateConfigData();

    Vita::string from_Hex(uint16_t hexnum, int fill);
    Vita::string from_CharArray(char* c);
};

#endif /* DATA_HPP_ */
