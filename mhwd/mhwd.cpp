/*
 *  mhwd - Manjaro Hardware Detection
 *  Roland Singer <roland@manjaro.org>
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

#include <stdio.h>
#include <string.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <mhwd.h>
#include "vita/string.hpp"

using namespace std;


enum ARGUMENTS {
    ARG_NONE = 0x0000,
    ARG_SHOWAVAILABLE = 0x0001,
    ARG_SHOWINSTALLED = 0x0002,
    ARG_SHOWPCI = 0x0004,
    ARG_SHOWUSB = 0x0008,
    ARG_INSTALL = 0x0010,
    ARG_REMOVE = 0x0020,
    ARG_SHOWDETAILED = 0x0040,
    ARG_FORCE = 0x0080
};


ARGUMENTS arguments;
mhwd::Data data;
vector<string> configList;
mhwd::TYPE operationType;

void messageFunc(string str);
void printHelp();
void printAvailableConfigs(mhwd::TYPE type);
void printInstalledConfigs(mhwd::TYPE type);
void checkInvalidConfigs();
bool installConfigs();
bool removeConfigs();



int main (int argc, char *argv[])
{
    // Set up data object
    mhwd::initData(&data);
    data.environment.messageFunc = &messageFunc;


    if (argc <= 1)
        arguments = ARG_SHOWAVAILABLE;
    else
        arguments = ARG_NONE;

    // Get command line arguments
    for (int nArg=1; nArg < argc; nArg++) {
        if (strcmp(argv[nArg], "-h") == 0 || strcmp(argv[nArg], "--help") == 0) {
            printHelp();
            return 0;
        }
        else if (strcmp(argv[nArg], "-f") == 0 || strcmp(argv[nArg], "--force") == 0) {
            arguments = (ARGUMENTS)(arguments | ARG_FORCE);
        }
        else if (strcmp(argv[nArg], "-s") == 0 || strcmp(argv[nArg], "--show") == 0) {
            arguments = (ARGUMENTS)(arguments | ARG_SHOWAVAILABLE);
        }
        else if (strcmp(argv[nArg], "-si") == 0 || strcmp(argv[nArg], "--showinstalled") == 0) {
            arguments = (ARGUMENTS)(arguments | ARG_SHOWINSTALLED);
        }
        else if (strcmp(argv[nArg], "-sh") == 0 || strcmp(argv[nArg], "--showhardware") == 0) {
            arguments = (ARGUMENTS)(arguments | ARG_SHOWDETAILED);
        }
        else if (strcmp(argv[nArg], "--pci") == 0) {
            arguments = (ARGUMENTS)(arguments | ARG_SHOWPCI);
        }
        else if (strcmp(argv[nArg], "--usb") == 0) {
            arguments = (ARGUMENTS)(arguments | ARG_SHOWUSB);
        }
        else if (strcmp(argv[nArg], "-i") == 0 || strcmp(argv[nArg], "--install") == 0) {
            ++nArg;
            if (nArg >= argc || (strcmp(argv[nArg], "pci") != 0 && strcmp(argv[nArg], "usb") != 0)) {
                cout << "invalid use of option: -i/--install" << endl << endl;
                printHelp();
                return 1;
            }

            if (strcmp(argv[nArg], "usb") == 0)
                operationType = mhwd::TYPE_USB;
            else
                operationType = mhwd::TYPE_PCI;

            arguments = (ARGUMENTS)(arguments | ARG_INSTALL);
        }
        else if (strcmp(argv[nArg], "-r") == 0 || strcmp(argv[nArg], "--remove") == 0) {
            ++nArg;
            if (nArg >= argc || (strcmp(argv[nArg], "pci") != 0 && strcmp(argv[nArg], "usb") != 0)) {
                cout << "invalid use of option: -i/--install" << endl << endl;
                printHelp();
                return 1;
            }

            if (strcmp(argv[nArg], "usb") == 0)
                operationType = mhwd::TYPE_USB;
            else
                operationType = mhwd::TYPE_PCI;

            arguments = (ARGUMENTS)(arguments | ARG_REMOVE);
        }
        else if (strcmp(argv[nArg], "--cachedir") == 0) {
            if (nArg + 1 >= argc) {
                cout << "invalid use of option: --cachedir" << endl << endl;
                printHelp();
                return 1;
            }

            data.environment.cachePath = Vita::string(argv[++nArg]).trim("\"").trim();
        }
        else if ((arguments & ARG_INSTALL) || (arguments & ARG_REMOVE)) {
            configList.push_back(Vita::string(argv[nArg]).toLower());
        }
        else {
            cout << "invalid option: " << argv[nArg] << endl << endl;
            printHelp();
            return 1;
        }
    }


    if ((arguments & ARG_INSTALL) && (arguments & ARG_REMOVE)) {
        cout << "Error: install and remove option can be used only seperate!" << endl << endl;
        printHelp();
        return 1;
    }
    else if ((arguments & ARG_REMOVE || arguments & ARG_INSTALL) && configList.empty()) {
        cout << "nothing to do?!" << endl << endl;
        printHelp();
        return 1;
    }

    if (!(arguments & ARG_SHOWPCI) && !(arguments & ARG_SHOWUSB))
        arguments = (ARGUMENTS)(arguments | ARG_SHOWUSB | ARG_SHOWPCI);


    // Fill data with hardware informations
    mhwd::fillData(&data);

    // Check for invalid configs
    checkInvalidConfigs();

    // Check set arguments
    if (arguments & ARG_SHOWDETAILED && arguments & ARG_SHOWPCI)
        mhwd::printDeviceDetails(mhwd::TYPE_PCI);
    if (arguments & ARG_SHOWDETAILED && arguments & ARG_SHOWUSB)
        mhwd::printDeviceDetails(mhwd::TYPE_USB);

    if (arguments & ARG_SHOWAVAILABLE && arguments & ARG_SHOWPCI)
        printAvailableConfigs(mhwd::TYPE_PCI);
    if (arguments & ARG_SHOWAVAILABLE && arguments & ARG_SHOWUSB)
        printAvailableConfigs(mhwd::TYPE_USB);


    if (arguments & ARG_SHOWINSTALLED && arguments & ARG_SHOWPCI)
        printInstalledConfigs(mhwd::TYPE_PCI);
    if (arguments & ARG_SHOWINSTALLED && arguments & ARG_SHOWUSB)
        printInstalledConfigs(mhwd::TYPE_USB);


    if (arguments & ARG_INSTALL) {
        if (!installConfigs())
            return 1;
    }
    else if (arguments & ARG_REMOVE) {
        if (!removeConfigs())
            return 1;
    }


    return 0;
}



void messageFunc(string str) {
    cout << str;
}



void printHelp() {
    cout << "Usage: mhwd [OPTIONS] <config(s)>" << endl << endl;
    cout << "  --pci\t\t\t\t\tshow only pci devices and driver configs" << endl;
    cout << "  --usb\t\t\t\t\tshow only usb devices and driver configs" << endl;
    cout << "  -h/--help\t\t\t\tshow help" << endl;
    cout << "  -f/--force\t\t\t\tforce reinstallation" << endl;
    cout << "  -s/--show\t\t\t\tshow devices and driver configs" << endl;
    cout << "  -si/--showinstalled\t\t\tshow installed driver configs" << endl;
    cout << "  -sh/--showhardware\t\t\tshow detailed hardware info" << endl;
    cout << "  -i/--install <usb/pci> <config(s)>\tinstall driver config(s)" << endl;
    cout << "  -r/--remove <usb/pci> <config(s)>\tremove driver config(s)" << endl;
    cout << "  --cachedir <path>\t\t\tset package manager cache path" << endl;
    cout << endl;
}



void printAvailableConfigs(mhwd::TYPE type) {
    string beg;
    vector<mhwd::Device> *devices;

    if (type == mhwd::TYPE_USB) {
        beg = "USB";
        devices = &data.USBDevices;
    }
    else {
        beg = "PCI";
        devices = &data.PCIDevices;
    }


    for (vector<mhwd::Device>::iterator dev_iter = devices->begin(); dev_iter != devices->end(); dev_iter++) {
        if ((*dev_iter).availableConfigs.empty() && (*dev_iter).installedConfigs.empty())
            continue;

        cout << endl << setfill('-') << setw(80) << "-" << setfill(' ') << endl;
        cout << "> " << beg << " Device: " << (*dev_iter).className << " (" << (*dev_iter).classID << ") ";
        cout << (*dev_iter).vendorName << " (" << (*dev_iter).vendorID << ") ";
        cout << (*dev_iter).deviceName << " (" << (*dev_iter).deviceID << ") " << endl;
        cout << setfill('-') << setw(80) << "-" << setfill(' ') << endl;

        if (!(*dev_iter).installedConfigs.empty())
            cout << "  > INSTALLED:" << endl;

        for (vector<mhwd::Config>::iterator iterator = (*dev_iter).installedConfigs.begin(); iterator != (*dev_iter).installedConfigs.end(); iterator++) {
            cout << endl << "    NAME:\t" << (*iterator).name << endl;
            cout << "    VERSION:\t" << (*iterator).version << endl;
            cout << "    INFO:\t" << (*iterator).info << endl;
            cout << "    PRIORITY:\t" << (*iterator).priority << endl;
            if ((*iterator).freedriver)
                cout << "    FREEDRIVER:\ttrue" << endl;
            else
                cout << "    FREEDRIVER:\tfalse" << endl;
        }

        if (!(*dev_iter).installedConfigs.empty())
            cout << endl << endl;
        if (!(*dev_iter).availableConfigs.empty())
            cout << "  > AVAILABLE:" << endl;

        for (vector<mhwd::Config>::iterator iterator = (*dev_iter).availableConfigs.begin(); iterator != (*dev_iter).availableConfigs.end(); iterator++) {
            cout << endl << "    NAME:\t" << (*iterator).name << endl;
            cout << "    VERSION:\t" << (*iterator).version << endl;
            cout << "    INFO:\t" << (*iterator).info << endl;
            cout << "    PRIORITY:\t" << (*iterator).priority << endl;
            if ((*iterator).freedriver)
                cout << "    FREEDRIVER:\ttrue" << endl;
            else
                cout << "    FREEDRIVER:\tfalse" << endl;
        }

        cout << endl;
    }
}



void printInstalledConfigs(mhwd::TYPE type) {
    string beg;
    vector<mhwd::Config> *configs;

    if (type == mhwd::TYPE_USB) {
        beg = "USB";
        configs = &data.installedUSBConfigs;
    }
    else {
        beg = "PCI";
        configs = &data.installedPCIConfigs;
    }

    if (configs->empty())
        return;

    for (vector<mhwd::Config>::iterator iterator = configs->begin(); iterator != configs->end(); iterator++) {
        cout << endl << "   NAME:\t" << (*iterator).name << endl;
        cout << "   ATTACHED:\t" << beg << endl;
        cout << "   VERSION:\t" << (*iterator).version << endl;
        cout << "   INFO:\t" << (*iterator).info << endl;
        cout << "   PRIORITY:\t" << (*iterator).priority << endl;
        if ((*iterator).freedriver)
            cout << "   FREEDRIVER:\ttrue" << endl;
        else
            cout << "   FREEDRIVER:\tfalse" << endl;
    }

    cout << endl;
}



void checkInvalidConfigs() {
    for (vector<mhwd::Config>::iterator iterator = data.invalidConfigs.begin(); iterator != data.invalidConfigs.end(); iterator++) {
        cout << "Warning: Config '" << (*iterator).configPath << "' is invalid!" << endl;
    }
}


/* TODO: Implement config dependency installing and conflict checking

bool installConfigs(vector<string> &configList, vector<string> &skipList) {
    for (vector<string>::iterator iter = configList.begin(); iter != configList.end(); iter++) {
        bool skip = false;

        for (vector<string>::iterator skip_iter = skipList.begin(); skip_iter != skipList.end(); skip_iter++) {
            if ((*iter) == (*skip_iter)) {
                skip = true;
                break;
            }
        }

        if (skip)
            continue;

        skipList.push_back((*iter));


        mhwd::Config *config = mhwd::getAvailableConfig(&data, (*iter), operationType);

        if (config == NULL) {
            cout << "Config " << (*iter) << " does not exist!" << endl;
            return false;
        }
        else if (!config->dependencies.empty()) {
            cout << "> Installing dependencies of " << config->name << "..." << endl;

            if (!installConfigs(config->dependencies, skipList))
                return false;
        }

        cout << "> Installing Config " << config->name << "..." << endl;

        if (!mhwd::installConfig(&data, config)) {
            cout << "Installation failed: " << data.lastError << endl;
            return false;
        }
    }

    return true;
}*/



bool installConfigs() {
    for (vector<string>::iterator iter = configList.begin(); iter != configList.end(); iter++) {
        mhwd::Config *config = mhwd::getAvailableConfig(&data, (*iter), operationType);
        mhwd::Config *installedConfig = mhwd::getInstalledConfig(&data, (*iter), operationType);

        if (config == NULL) {
            cout << "Config " << (*iter) << " does not exist!" << endl;
            return false;
        }
        else if (installedConfig != NULL && !(arguments & ARG_FORCE)) {
            cout << "A version of config " << (*iter) << " is already installed!\nUse -f/--force to force installation..." << endl;
            return false;
        }
        else if (installedConfig != NULL && arguments & ARG_FORCE) {
            cout << "> Removing Config " << installedConfig->name << "..." << endl;

            if (!mhwd::uninstallConfig(&data, installedConfig)) {
                cout << "Remove failed: " << data.lastError << endl;
                return false;
            }
        }

        cout << "> Installing Config " << config->name << "..." << endl;

        if (!mhwd::installConfig(&data, config)) {
            cout << "Installation failed: " << data.lastError << endl;
            return false;
        }
    }

    return true;
}



bool removeConfigs() {
    for (vector<string>::iterator iter = configList.begin(); iter != configList.end(); iter++) {
        mhwd::Config *config = mhwd::getInstalledConfig(&data, (*iter), operationType);

        if (config == NULL) {
            cout << "Config " << (*iter) << " is not installed!" << endl;
            return false;
        }

        cout << "> Removing Config " << config->name << "..." << endl;

        if (!mhwd::uninstallConfig(&data, config)) {
            cout << "Remove failed: " << data.lastError << endl;
            return false;
        }
    }

    return true;
}
