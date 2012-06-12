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

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include <mhwd.h>
#include "vita/string.hpp"

using namespace std;


#define CONSOLE_COLOR_RESET "\e[m"
#define CONSOLE_MESSAGE_COLOR "\e[1m\e[31m"
#define CONSOLE_TEXT_OUTPUT_COLOR "\e[0;32m"


enum ARGUMENTS {
    ARG_NONE = 0x0000,
    ARG_SHOWPCI = 0x0001,
    ARG_SHOWUSB = 0x0002,
    ARG_INSTALL = 0x004,
    ARG_REMOVE = 0x0008,
    ARG_DETAIL = 0x0010,
    ARG_FORCE = 0x0020,
    ARG_LISTALL = 0x0040,
    ARG_LISTINSTALLED = 0x0080,
    ARG_LISTAVAILABLE = 0x0100,
    ARG_LISTHARDWARE = 0x0200,
    ARG_CUSTOMINSTALL = 0x0400,
    ARG_AUTOCONFIGURE = 0x0800
};


ARGUMENTS arguments;
mhwd::Data data;





void catchSignal(int) {
    // Reset console color
    cout << CONSOLE_COLOR_RESET;
    exit(1);
}



void printStatus(string str) {
    cout << CONSOLE_MESSAGE_COLOR << "> " << CONSOLE_COLOR_RESET << str << endl;
}



void printError(string str) {
    cout << CONSOLE_MESSAGE_COLOR << "Error: " << CONSOLE_COLOR_RESET << str << endl;
}



void printWarning(string str) {
    cout << CONSOLE_MESSAGE_COLOR << "Warning: " << CONSOLE_COLOR_RESET << str << endl;
}




void messageFunc(mhwd::MESSAGETYPE type, std::string str) {
    if (type == mhwd::MESSAGETYPE_CONSOLE_OUTPUT)
        cout << CONSOLE_TEXT_OUTPUT_COLOR << str << CONSOLE_COLOR_RESET;
    else if (type == mhwd::MESSAGETYPE_INSTALLDEPENDENCY_START)
        printStatus("Installing dependency " + str + "...");
    else if (type == mhwd::MESSAGETYPE_INSTALLDEPENDENCY_END)
        printStatus("Successfully installed dependency " + str);
    else if (type == mhwd::MESSAGETYPE_INSTALL_START)
        printStatus("Installing " + str + "...");
    else if (type == mhwd::MESSAGETYPE_INSTALL_END)
        printStatus("Successfully installed " + str);
    else if (type == mhwd::MESSAGETYPE_REMOVE_START)
        printStatus("Removing " + str + "...");
    else if (type == mhwd::MESSAGETYPE_REMOVE_END)
        printStatus("Successfully removed " + str);
    else
        printError("You shouldn't see this?! Unknown message type!");
}



void printHelp() {
    cout << "Usage: mhwd [OPTIONS] <config(s)>" << endl << endl;
    cout << "  --pci\t\t\t\t\tlist only pci devices and driver configs" << endl;
    cout << "  --usb\t\t\t\t\tlist only usb devices and driver configs" << endl;
    cout << "  -h/--help\t\t\t\tshow help" << endl;
    cout << "  -f/--force\t\t\t\tforce reinstallation" << endl;
    cout << "  -d/--detail\t\t\t\tshow detailed info for -l/-li/-lh" << endl;
    cout << "  -l/--list\t\t\t\tlist available configs for devices" << endl;
    cout << "  -la/--listall\t\t\t\tlist all driver configs" << endl;
    cout << "  -li/--listinstalled\t\t\tlist installed driver configs" << endl;
    cout << "  -lh/--listhardware\t\t\tlist hardware information" << endl;
    cout << "  -i/--install <usb/pci> <config(s)>\tinstall driver config(s)" << endl;
    cout << "  -ic/--installcustom <usb/pci> <path>\tinstall custom config(s)" << endl;
    cout << "  -r/--remove <usb/pci> <config(s)>\tremove driver config(s)" << endl;
    cout << "  -a/--auto <usb/pci> <free/nonfree> <classid>\tauto install configs for classid" << endl;
    cout << "  --pmcachedir <path>\t\t\tset package manager cache path" << endl;
    cout << "  --pmconfig <path>\t\t\tset package manager config" << endl;
    cout << "  --pmroot <path>\t\t\tset package manager root" << endl;
    cout << endl;
}



void listDevices(vector<mhwd::Device*> devices, string type) {
    if (devices.empty()) {
        printWarning("No " + type + " devices found!");
        return;
    }

    printStatus(type + " devices:");

    cout << endl << setfill('-') << setw(80) << "-" << setfill(' ') << endl;
    cout << setw(30) << "TYPE" << setw(15) << "BUS" << setw(8) << "CLASS" << setw(8) << "VENDOR" << setw(8) << "DEVICE" << setw(10) << "CONFIGS" << endl;
    cout << setfill('-') << setw(80) << "-" << setfill(' ') << endl;

    for (vector<mhwd::Device*>::iterator iterator = devices.begin(); iterator != devices.end(); iterator++) {
        cout << setw(30) << (*iterator)->className << setw(15) << (*iterator)->sysfsBusID << setw(8) << (*iterator)->classID << setw(8) << (*iterator)->vendorID << setw(8) << (*iterator)->deviceID << setw(10) << (*iterator)->availableConfigs.size() << endl;
    }

    cout << endl << endl;
}



void listConfigs(vector<mhwd::Config*> configs, string beg, string empty = "") {
    if (configs.empty()) {
        if (!empty.empty())
            printWarning(empty);
        return;
    }

    printStatus(beg);

    cout << endl << setfill('-') << setw(80) << "-" << setfill(' ') << endl;
    cout << setw(22) << "NAME" << setw(22) << "VERSION" << setw(20) << "FREEDRIVER" << setw(15) << "TYPE" << endl;
    cout << setfill('-') << setw(80) << "-" << setfill(' ') << endl;

    for (vector<mhwd::Config*>::iterator iterator = configs.begin(); iterator != configs.end(); iterator++) {
        string freedriver, type;

        if ((*iterator)->freedriver)
            freedriver = "true";
        else
            freedriver = "false";

        if ((*iterator)->type == mhwd::TYPE_USB)
            type = "USB";
        else
            type = "PCI";

        cout << setw(22) << (*iterator)->name << setw(22) << (*iterator)->version << setw(20) << freedriver << setw(15) << type << endl;
    }

    cout << endl << endl;
}



void printConfigDetails(mhwd::Config *config) {
    string freedriver, type, dependencies, conflicts, info, classids, vendorids;

    if (config->freedriver)
        freedriver = "true";
    else
        freedriver = "false";

    if (config->type == mhwd::TYPE_USB)
        type = "USB";
    else
        type = "PCI";

    for (vector<mhwd::Config::HardwareIDs>::iterator hwd = config->hwdIDs.begin(); hwd != config->hwdIDs.end(); hwd++)
        for (vector<std::string>::iterator it = (*hwd).classIDs.begin(); it != (*hwd).classIDs.end(); it++)
            classids += (*it) + " ";

    for (vector<mhwd::Config::HardwareIDs>::iterator hwd = config->hwdIDs.begin(); hwd != config->hwdIDs.end(); hwd++)
        for (vector<std::string>::iterator it = (*hwd).vendorIDs.begin(); it != (*hwd).vendorIDs.end(); it++)
            vendorids += (*it) + " ";

    for (vector<std::string>::iterator it = config->dependencies.begin(); it != config->dependencies.end(); it++)
        dependencies += (*it) + " ";

    for (vector<std::string>::iterator it = config->conflicts.begin(); it != config->conflicts.end(); it++)
        conflicts += (*it) + " ";

    classids = Vita::string(classids).trim();
    vendorids = Vita::string(vendorids).trim();
    dependencies = Vita::string(dependencies).trim();
    conflicts = Vita::string(conflicts).trim();

    if (dependencies.empty())
        dependencies = "-";

    if (conflicts.empty())
        conflicts = "-";

    info = config->info;
    if (info.empty())
        info = "-";

    cout << "   NAME:\t" << config->name << endl;
    cout << "   ATTACHED:\t" << type << endl;
    cout << "   VERSION:\t" << config->version << endl;
    cout << "   INFO:\t" << info << endl;
    cout << "   PRIORITY:\t" << config->priority << endl;
    cout << "   FREEDRIVER:\t" << freedriver << endl;
    cout << "   DEPENDS:\t" << dependencies << endl;
    cout << "   CONFLICTS:\t" << conflicts << endl;
    cout << "   CLASSIDS:\t" << classids << endl;
    cout << "   VENDORIDS:\t" << vendorids << endl;
}



void printAvailableConfigs(mhwd::TYPE type) {
    string beg;
    vector<mhwd::Device*> *devices;

    if (type == mhwd::TYPE_USB) {
        beg = "USB";
        devices = &data.USBDevices;
    }
    else {
        beg = "PCI";
        devices = &data.PCIDevices;
    }

    bool found = false;


    for (vector<mhwd::Device*>::iterator dev_iter = devices->begin(); dev_iter != devices->end(); dev_iter++) {
        if ((*dev_iter)->availableConfigs.empty() && (*dev_iter)->installedConfigs.empty())
            continue;

        found = true;

        cout << endl << setfill('-') << setw(80) << "-" << setfill(' ') << endl;
        printStatus(beg + " Device: " + (*dev_iter)->sysfsID + " (" + (*dev_iter)->classID + ":" + (*dev_iter)->vendorID + ":" + (*dev_iter)->deviceID + ")");
        cout << "  " << (*dev_iter)->className << " " << (*dev_iter)->vendorName << " " << (*dev_iter)->deviceName << endl;
        cout << setfill('-') << setw(80) << "-" << setfill(' ') << endl;


        if (!(*dev_iter)->installedConfigs.empty()) {
            cout << "  > INSTALLED:" << endl;
            for (vector<mhwd::Config*>::iterator iterator = (*dev_iter)->installedConfigs.begin(); iterator != (*dev_iter)->installedConfigs.end(); iterator++) {
                cout << endl;
                printConfigDetails((*iterator));
            }
            cout << endl << endl;
        }


        if (!(*dev_iter)->availableConfigs.empty()) {
            cout << "  > AVAILABLE:" << endl;
            for (vector<mhwd::Config*>::iterator iterator = (*dev_iter)->availableConfigs.begin(); iterator != (*dev_iter)->availableConfigs.end(); iterator++) {
                cout << endl;
                printConfigDetails((*iterator));
            }
            cout << endl;
        }
    }

    if (!found)
        printWarning("no configs for " + beg + " devices found!");
}



void printInstalledConfigs(mhwd::TYPE type) {
    string beg;
    vector<mhwd::Config*> *configs;

    if (type == mhwd::TYPE_USB) {
        beg = "USB";
        configs = &data.installedUSBConfigs;
    }
    else {
        beg = "PCI";
        configs = &data.installedPCIConfigs;
    }

    if (configs->empty()) {
        printWarning("no installed configs for " + beg + " devices found!");
        return;
    }
    for (vector<mhwd::Config*>::iterator iterator = configs->begin(); iterator != configs->end(); iterator++) {
        cout << endl;
        printConfigDetails((*iterator));
    }

    cout << endl;
}



bool performTransaction(mhwd::Config* config, mhwd::Transaction::TYPE type) {
    struct mhwd::Transaction transaction = mhwd::createTransaction(&data, config, type, (arguments & ARG_FORCE));

    // Print things to do
    if (type == mhwd::Transaction::TYPE_INSTALL) {
        // Print conflicts
        if (!transaction.conflictedConfigs.empty()) {
            string conflicts;

            for (vector<mhwd::Config*>::iterator iter = transaction.conflictedConfigs.begin(); iter != transaction.conflictedConfigs.end(); iter++)
                conflicts += " " + (*iter)->name;

            printError("config '" + config->name + "' conflicts with config(s):" + conflicts);
            return false;
        }


        // Print dependencies
        if (!transaction.dependencyConfigs.empty()) {
            string dependencies;

            for (vector<mhwd::Config*>::iterator iter = transaction.dependencyConfigs.begin(); iter != transaction.dependencyConfigs.end(); iter++)
                dependencies += " " + (*iter)->name;

            printStatus("Dependencies to install:" + dependencies);
            cout << "Proceed with installation? [Y/n] ";

            string input;
            // TODO: Get only one char!
            cin >> input;

            if (Vita::string(input).toLower().trim() != "y")
                return false;
        }
    }
    else if (type == mhwd::Transaction::TYPE_REMOVE) {
        // Print requirements
        if (!transaction.requiredByConfigs.empty()) {
            string requirements;

            for (vector<mhwd::Config*>::iterator iter = transaction.requiredByConfigs.begin(); iter != transaction.requiredByConfigs.end(); iter++)
                requirements += " " + (*iter)->name;

            printError("config '" + config->name + "' is required by config(s):" + requirements);
            return false;
        }
    }


    mhwd::STATUS status = mhwd::performTransaction(&data, &transaction);

    if (status == mhwd::STATUS_ERROR_ALREADY_INSTALLED)
        printWarning("a version of config '" + config->name + "' is already installed!\nUse -f/--force to force installation...");
    else if (status == mhwd::STATUS_ERROR_CONFLICTS)
        printError("config '" + config->name + "' conflicts with installed config(s)!");
    else if (status == mhwd::STATUS_ERROR_NOT_INSTALLED)
        printError("config '" + config->name + "' is not installed!");
    else if (status == mhwd::STATUS_ERROR_NO_MATCH_LOCAL_CONFIG)
        printError("passed config does not match with installed config!");
    else if (status == mhwd::STATUS_ERROR_REQUIREMENTS)
        printError("config '" + config->name + "' is required by installed config(s)!");
    else if (status == mhwd::STATUS_ERROR_SCRIPT_FAILED)
        printError("script failed!");
    else if (status == mhwd::STATUS_ERROR_SET_DATABASE)
        printError("failed to set database!");

    // Update mhwd data object
    mhwd::updateInstalledConfigData(&data);

    return (status == mhwd::STATUS_SUCCESS);
}




void cleanup() {
    // Cleanup
    mhwd::freeData(&data);
}



void checkRoot() {
    if (getuid()) {
        printError("you cannot perform this operation unless you are root!");
        cleanup();
        exit(1);
    }
}






int main (int argc, char *argv[])
{
    vector<string> configList;
    mhwd::TYPE operationType;
    bool autoConfigureNonFreeDriver;
    string autoConfigureClassID;


    // Setup signals
    signal(SIGHUP, catchSignal);
    signal(SIGINT, catchSignal);
    signal(SIGTERM, catchSignal);
    signal(SIGKILL, catchSignal);

    // Set up data object
    mhwd::initData(&data);
    data.environment.messageFunc = &messageFunc;


    if (argc <= 1)
        arguments = ARG_LISTAVAILABLE;
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
        else if (strcmp(argv[nArg], "-d") == 0 || strcmp(argv[nArg], "--detail") == 0) {
            arguments = (ARGUMENTS)(arguments | ARG_DETAIL);
        }
        else if (strcmp(argv[nArg], "-la") == 0 || strcmp(argv[nArg], "--listall") == 0) {
            arguments = (ARGUMENTS)(arguments | ARG_LISTALL);
        }
        else if (strcmp(argv[nArg], "-li") == 0 || strcmp(argv[nArg], "--listinstalled") == 0) {
            arguments = (ARGUMENTS)(arguments | ARG_LISTINSTALLED);
        }
        else if (strcmp(argv[nArg], "-l") == 0 || strcmp(argv[nArg], "--list") == 0) {
            arguments = (ARGUMENTS)(arguments | ARG_LISTAVAILABLE);
        }
        else if (strcmp(argv[nArg], "-lh") == 0 || strcmp(argv[nArg], "--listhardware") == 0) {
            arguments = (ARGUMENTS)(arguments | ARG_LISTHARDWARE);
        }
        else if (strcmp(argv[nArg], "--pci") == 0) {
            arguments = (ARGUMENTS)(arguments | ARG_SHOWPCI);
        }
        else if (strcmp(argv[nArg], "--usb") == 0) {
            arguments = (ARGUMENTS)(arguments | ARG_SHOWUSB);
        }
        else if (strcmp(argv[nArg], "-a") == 0 || strcmp(argv[nArg], "--auto") == 0) {
            ++nArg;
            if (nArg + 2 >= argc || (strcmp(argv[nArg], "pci") != 0 && strcmp(argv[nArg], "usb") != 0) || (strcmp(argv[nArg+1], "free") != 0 && strcmp(argv[nArg+1], "nonfree") != 0)) {
                printError("invalid use of option: -a/--auto\n");
                printHelp();
                return 1;
            }

            if (strcmp(argv[nArg], "usb") == 0)
                operationType = mhwd::TYPE_USB;
            else
                operationType = mhwd::TYPE_PCI;

            ++nArg;
            if (strcmp(argv[nArg], "nonfree") == 0)
                autoConfigureNonFreeDriver = true;
            else
                autoConfigureNonFreeDriver = false;


            autoConfigureClassID = Vita::string(argv[++nArg]).toLower().trim();
            arguments = (ARGUMENTS)(arguments | ARG_AUTOCONFIGURE);
        }
        else if (strcmp(argv[nArg], "-ic") == 0 || strcmp(argv[nArg], "--installcustom") == 0) {
            ++nArg;
            if (nArg >= argc || (strcmp(argv[nArg], "pci") != 0 && strcmp(argv[nArg], "usb") != 0)) {
                printError("invalid use of option: -ic/--installcustom\n");
                printHelp();
                return 1;
            }

            if (strcmp(argv[nArg], "usb") == 0)
                operationType = mhwd::TYPE_USB;
            else
                operationType = mhwd::TYPE_PCI;

            arguments = (ARGUMENTS)(arguments | ARG_INSTALL | ARG_CUSTOMINSTALL);
        }
        else if (strcmp(argv[nArg], "-i") == 0 || strcmp(argv[nArg], "--install") == 0) {
            ++nArg;
            if (nArg >= argc || (strcmp(argv[nArg], "pci") != 0 && strcmp(argv[nArg], "usb") != 0)) {
                printError("invalid use of option: -i/--install\n");
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
                printError("invalid use of option: -r/--remove\n");
                printHelp();
                return 1;
            }

            if (strcmp(argv[nArg], "usb") == 0)
                operationType = mhwd::TYPE_USB;
            else
                operationType = mhwd::TYPE_PCI;

            arguments = (ARGUMENTS)(arguments | ARG_REMOVE);
        }
        else if (strcmp(argv[nArg], "--pmcachedir") == 0) {
            if (nArg + 1 >= argc) {
                printError("invalid use of option: --pmcachedir\n");
                printHelp();
                return 1;
            }

            data.environment.PMCachePath = Vita::string(argv[++nArg]).trim("\"").trim();
        }
        else if (strcmp(argv[nArg], "--pmconfig") == 0) {
            if (nArg + 1 >= argc) {
                printError("invalid use of option: --pmconfig\n");
                printHelp();
                return 1;
            }

            data.environment.PMConfigPath = Vita::string(argv[++nArg]).trim("\"").trim();
        }
        else if (strcmp(argv[nArg], "--pmroot") == 0) {
            if (nArg + 1 >= argc) {
                printError("invalid use of option: --pmroot\n");
                printHelp();
                return 1;
            }

            data.environment.PMRootPath = Vita::string(argv[++nArg]).trim("\"").trim();
        }
        else if ((arguments & ARG_INSTALL) || (arguments & ARG_REMOVE)) {
            bool found = false;
            std::string name;

            if (arguments & ARG_CUSTOMINSTALL)
                name= Vita::string(argv[nArg]);
            else
                name= Vita::string(argv[nArg]).toLower();

            // Check if already in list
            for (vector<string>::iterator iter = configList.begin(); iter != configList.end(); iter++) {
                if ((*iter) == name) {
                    found = true;
                    break;
                }
            }

            if (!found)
                configList.push_back(name);
        }
        else {
            printError("invalid option: " + string(argv[nArg]) + "\n");
            printHelp();
            return 1;
        }
    }



    // Check if arguments are right
    if ((arguments & ARG_INSTALL) && (arguments & ARG_REMOVE)) {
        printError("install and remove options can be used only seperate!\n");
        printHelp();
        return 1;
    }
    else if (((arguments & ARG_INSTALL) || (arguments & ARG_REMOVE)) && (arguments & ARG_AUTOCONFIGURE)) {
        printError("auto option can't be combined with install and remove options!\n");
        printHelp();
        return 1;
    }
    else if ((arguments & ARG_REMOVE || arguments & ARG_INSTALL) && configList.empty()) {
        printError("nothing to do?!\n");
        printHelp();
        return 1;
    }

    if (!(arguments & ARG_SHOWPCI) && !(arguments & ARG_SHOWUSB))
        arguments = (ARGUMENTS)(arguments | ARG_SHOWUSB | ARG_SHOWPCI);



    // Check environment
    string ret = mhwd::checkEnvironment();
    if (!ret.empty()) {
        printError("directory '" + ret + "' does not exist!");
        cleanup();
        return 1;
    }


    // Fill data with hardware informations
    mhwd::fillData(&data);

    // Check for invalid configs
    for (vector<mhwd::Config*>::iterator iterator = data.invalidConfigs.begin(); iterator != data.invalidConfigs.end(); iterator++) {
        printWarning("config '" + (*iterator)->configPath + "' is invalid!");
    }


    // > Perform operations:

    // List all configs
    if (arguments & ARG_LISTALL && arguments & ARG_SHOWPCI)
            listConfigs(data.allPCIConfigs, "All PCI configs:", "No PCI configs found!");
    if (arguments & ARG_LISTALL && arguments & ARG_SHOWUSB)
            listConfigs(data.allUSBConfigs, "All USB configs:", "No USB configs found!");


    // List installed configs
    if (arguments & ARG_LISTINSTALLED && arguments & ARG_SHOWPCI) {
        if (arguments & ARG_DETAIL)
            printInstalledConfigs(mhwd::TYPE_PCI);
        else
            listConfigs(data.installedPCIConfigs, "Installed PCI configs:", "No installed PCI configs!");
    }
    if (arguments & ARG_LISTINSTALLED && arguments & ARG_SHOWUSB) {
        if (arguments & ARG_DETAIL)
            printInstalledConfigs(mhwd::TYPE_USB);
        else
            listConfigs(data.installedUSBConfigs, "Installed USB configs:", "No installed USB configs!");
    }


    // List available configs
    if (arguments & ARG_LISTAVAILABLE && arguments & ARG_SHOWPCI) {
        if (arguments & ARG_DETAIL)
            printAvailableConfigs(mhwd::TYPE_PCI);
        else
            for (vector<mhwd::Device*>::iterator dev_iter = data.PCIDevices.begin(); dev_iter != data.PCIDevices.end(); dev_iter++)
                listConfigs((*dev_iter)->availableConfigs, (*dev_iter)->sysfsBusID + " (" + (*dev_iter)->classID + ":" + (*dev_iter)->vendorID + ":" + (*dev_iter)->deviceID + ") " + (*dev_iter)->className + " " + (*dev_iter)->vendorName + ":");
    }
    if (arguments & ARG_LISTAVAILABLE && arguments & ARG_SHOWUSB) {
        if (arguments & ARG_DETAIL)
            printAvailableConfigs(mhwd::TYPE_USB);
        else
            for (vector<mhwd::Device*>::iterator dev_iter = data.USBDevices.begin(); dev_iter != data.USBDevices.end(); dev_iter++)
                listConfigs((*dev_iter)->availableConfigs, (*dev_iter)->sysfsBusID + " (" + (*dev_iter)->classID + ":" + (*dev_iter)->vendorID + ":" + (*dev_iter)->deviceID + ") " + (*dev_iter)->className + " " + (*dev_iter)->vendorName + ":");
    }


    // List hardware information
    if (arguments & ARG_LISTHARDWARE && arguments & ARG_SHOWPCI) {
        if (arguments & ARG_DETAIL)
            mhwd::printDeviceDetails(mhwd::TYPE_PCI);
        else
            listDevices(data.PCIDevices, "PCI");
    }
    if (arguments & ARG_LISTHARDWARE && arguments & ARG_SHOWUSB) {
        if (arguments & ARG_DETAIL)
            mhwd::printDeviceDetails(mhwd::TYPE_USB);
        else
            listDevices(data.USBDevices, "USB");
    }



    // Auto configuration
    if (arguments & ARG_AUTOCONFIGURE) {
        bool founddevice = false;
        vector<mhwd::Device*> *devices;
        vector<mhwd::Config*> *installedConfigs;

        if (operationType == mhwd::TYPE_USB) {
            devices = &data.USBDevices;
            installedConfigs = &data.installedUSBConfigs;
        }
        else {
            devices = &data.PCIDevices;
            installedConfigs = &data.installedPCIConfigs;
        }


        for (vector<mhwd::Device*>::iterator dev_iter = devices->begin(); dev_iter != devices->end(); dev_iter++) {
            if ((*dev_iter)->classID != autoConfigureClassID)
                continue;

            founddevice = true;
            mhwd::Config *config = NULL;

            for (vector<mhwd::Config*>::iterator iter = (*dev_iter)->availableConfigs.begin(); iter != (*dev_iter)->availableConfigs.end(); iter++) {
                if (!autoConfigureNonFreeDriver && !(*iter)->freedriver)
                    continue;

                config = (*iter);
                break;
            }

            if (config == NULL) {
                printWarning("No config found for device: " + (*dev_iter)->sysfsBusID + " (" + (*dev_iter)->classID + ":" + (*dev_iter)->vendorID + ":" + (*dev_iter)->deviceID + ") " + (*dev_iter)->className + " " + (*dev_iter)->vendorName + " " + (*dev_iter)->deviceName);
                continue;
            }

            // Check if already in list
            bool found = false;
            for (vector<string>::iterator iter = configList.begin(); iter != configList.end(); iter++) {
                if ((*iter) == config->name) {
                    found = true;
                    break;
                }
            }

            // If force is not set then skip found config
            bool skip = false;
            if (!(arguments & ARG_FORCE)) {
                for (vector<mhwd::Config*>::iterator iter = installedConfigs->begin(); iter != installedConfigs->end(); iter++) {
                    if ((*iter)->name == config->name) {
                        skip = true;
                        break;
                    }
                }
            }

            // Print found config
            if (skip)
                printStatus("Skipping already installed config '" + config->name + "' for device: " + (*dev_iter)->sysfsBusID + " (" + (*dev_iter)->classID + ":" + (*dev_iter)->vendorID + ":" + (*dev_iter)->deviceID + ") " + (*dev_iter)->className + " " + (*dev_iter)->vendorName + " " + (*dev_iter)->deviceName);
            else
                printStatus("Using config '" + config->name + "' for device: " + (*dev_iter)->sysfsBusID + " (" + (*dev_iter)->classID + ":" + (*dev_iter)->vendorID + ":" + (*dev_iter)->deviceID + ") " + (*dev_iter)->className + " " + (*dev_iter)->vendorName + " " + (*dev_iter)->deviceName);

            if (!found && !skip)
                configList.push_back(config->name);
        }


        if (!founddevice)
            printWarning("No device of class " + autoConfigureClassID + " found!");
        else if (!configList.empty())
            arguments = (ARGUMENTS)(arguments | ARG_INSTALL);
    }



    // Transaction
    if (arguments & ARG_INSTALL || arguments & ARG_REMOVE) {
        checkRoot();

        for (vector<string>::iterator iter = configList.begin(); iter != configList.end(); iter++) {
            mhwd::Config *config;

            if (arguments & ARG_CUSTOMINSTALL) {
                // Custom install -> get configs
                struct stat filestatus;
                string filepath = (*iter) + "/MHWDCONFIG";

                if (stat(filepath .c_str(), &filestatus) != 0) {
                    printError("custom config '" + filepath  + "' does not exist!");
                    cleanup();
                    return 1;
                }
                if (!S_ISREG(filestatus.st_mode)) {
                    printError("custom config '" + filepath  + "' is invalid!");
                    cleanup();
                    return 1;
                }

                config = new mhwd::Config();
                if (!mhwd::fillConfig(config, filepath, operationType)) {
                    printError("failed to read custom config '" + filepath  + "'!");
                    cleanup();
                    delete(config);
                    return 1;
                }

                if (!performTransaction(config, mhwd::Transaction::TYPE_INSTALL)) {
                    cleanup();
                    delete(config);
                    return 1;
                }

                delete(config);
            }
            else if (arguments & ARG_INSTALL) {
                config = mhwd::getAvailableConfig(&data, (*iter), operationType);

                if (config == NULL) {
                    config = mhwd::getDatabaseConfig(&data, (*iter), operationType);
                    if (config == NULL) {
                        printError("config '" + (*iter) + "' does not exist!");
                        cleanup();
                        return 1;
                    }
                    else {
                        printWarning("no matching device for config '" + (*iter) + "' found!");
                    }
                }

                if (!performTransaction(config, mhwd::Transaction::TYPE_INSTALL)) {
                    cleanup();
                    return 1;
                }
            }
            else if (arguments & ARG_REMOVE) {
                config = mhwd::getInstalledConfig(&data, (*iter), operationType);

                if (config == NULL) {
                    printError("config '" + (*iter) + "' is not installed!");
                    cleanup();
                    return 1;
                }

                if (!performTransaction(config, mhwd::Transaction::TYPE_REMOVE)) {
                    cleanup();
                    return 1;
                }
            }
        }
    }


    // Cleanup
    cleanup();

    return 0;
}
