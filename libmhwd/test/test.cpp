
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "mhwd.h"

using namespace std;


int main (int argc, char *argv[])
{
      mhwd::Data data;
      mhwd::fillData(&data);

      for (vector<mhwd::Device>::iterator dev_iter = data.PCIDevices.begin(); dev_iter != data.PCIDevices.end(); dev_iter++) {
          if (!(*dev_iter).availableConfigs.empty())
              cout << endl << endl << (*dev_iter).className << " " << (*dev_iter).vendorName << ":" << endl << "Available: ";

          for (vector<mhwd::Config>::const_iterator iterator = (*dev_iter).availableConfigs.begin(); iterator != (*dev_iter).availableConfigs.end(); iterator++) {
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
