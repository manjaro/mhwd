/*
 * Device.hpp
 *
 *  Created on: 28 sie 2014
 *      Author: dec
 */

#ifndef DEVICE_HPP_
#define DEVICE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "Config.hpp"
#include "Enums.hpp"

struct Device
{
    std::string type_;
    std::string className_;
    std::string deviceName_;
    std::string vendorName_;
    std::string classID_;
    std::string deviceID_;
    std::string vendorID_;
    std::string sysfsBusID_;
    std::string sysfsID_;
    std::vector<std::shared_ptr<Config>> availableConfigs_;
    std::vector<std::shared_ptr<Config>> installedConfigs_;
};

#endif /* DEVICE_HPP_ */
