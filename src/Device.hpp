/*
 * Device.hpp
 *
 *  Created on: 28 sie 2014
 *      Author: dec
 */

#ifndef DEVICE_HPP_
#define DEVICE_HPP_

#include "Config.hpp"
#include "Enums.hpp"

#include <string>
#include <vector>

struct Device
{
	std::string type;
	std::string className;
	std::string deviceName;
	std::string vendorName;
	std::string classID;
	std::string deviceID;
	std::string vendorID;
	std::string sysfsBusID;
	std::string sysfsID;
	std::vector<Config*> availableConfigs;
	std::vector<Config*> installedConfigs;
};

#endif /* DEVICE_HPP_ */
