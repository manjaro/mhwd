
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "mhwd.h"

using namespace std;


void message(string msg) {
    cout << msg;
}


int main (int argc, char *argv[])
{
      mhwd::Data data;
      mhwd::fillData(&data);

      data.environment.messageFunc = &message;

      /*for (vector<mhwd::Device>::iterator dev_iter = data.PCIDevices.begin(); dev_iter != data.PCIDevices.end(); dev_iter++) {
          for (vector<mhwd::Config>::iterator iterator = (*dev_iter).availableConfigs.begin(); iterator != (*dev_iter).availableConfigs.end(); iterator++) {
              if (mhwd::installConfig(&data, &(*iterator)))
                  std::cout << "installed config!" << std::endl;
              else
                  std::cout << "failed to installed config!\n" << data.lastError << std::endl;
          }
      }*/

      for (vector<mhwd::Config>::iterator iterator = data.installedPCIConfigs.begin(); iterator != data.installedPCIConfigs.end(); iterator++) {
          if (mhwd::uninstallConfig(&data, &(*iterator)))
              std::cout << "installed config!" << std::endl;
          else
              std::cout << "failed to installed config!\n" << data.lastError << std::endl;
      }


      for (vector<mhwd::Device>::iterator dev_iter = data.PCIDevices.begin(); dev_iter != data.PCIDevices.end(); dev_iter++) {
          if (!(*dev_iter).availableConfigs.empty())
              cout << endl << endl << (*dev_iter).className << " " << (*dev_iter).vendorName << ":" << endl << "Available: ";

          for (vector<mhwd::Config>::iterator iterator = (*dev_iter).availableConfigs.begin(); iterator != (*dev_iter).availableConfigs.end(); iterator++) {
              cout << (*iterator).name << "-" << (*iterator).version << "  ";
          }

          if (!(*dev_iter).installedConfigs.empty())
              cout << endl <<"Installed: ";

          for (vector<mhwd::Config>::const_iterator iterator = (*dev_iter).installedConfigs.begin(); iterator != (*dev_iter).installedConfigs.end(); iterator++) {
              cout << (*iterator).name << "-" << (*iterator).version << "  ";
          }
      }


      /*for(unsigned int i = 0; i < devices.size(); i++) {
          cout << setw(30) << devices[i]->getClassName();
          cout << setw(10)<< devices[i]->getClassID();
          cout << setw(40)<< devices[i]->getVendorName();
          cout << setw(10)<< devices[i]->getVendorID();
          cout << setw(30)<< devices[i]->getDeviceName();
          cout << setw(10)<< devices[i]->getDeviceID();
          cout << endl;
      }

    cout << endl << endl;

    mhwd::printDetails(mhwd::HW_USB);*/

    cout << endl << endl;
}
