#include <iostream>
#include "mhwd.h"


using namespace std;

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "mhwd.h"
#include "config.h"

using namespace std;


int main (int argc, char *argv[])
{
    mhwd::HWD hwd;
    vector<mhwd::Device>* devices = hwd.getPCIDevices();

    for (vector<mhwd::Device>::iterator iterator = devices->begin(); iterator != devices->end(); iterator++) {
        vector<mhwd::Config> configs = (*iterator).getConfigs();
        vector<mhwd::Config> installedConfigs = (*iterator).getInstalledConfigs();

        if (!configs.empty())
            cout << endl << endl << (*iterator).getClassName() << " " << (*iterator).getVendorName() << ":" << endl << "Available: ";

        for (vector<mhwd::Config>::const_iterator iterator = configs.begin(); iterator != configs.end(); iterator++) {
            mhwd::Config config = (*iterator);

            cout << config.getName() << "-" << config.getVersion() << "  ";
        }

        if (!installedConfigs.empty())
            cout << endl <<"Installed: ";

        for (vector<mhwd::Config>::const_iterator iterator = installedConfigs.begin(); iterator != installedConfigs.end(); iterator++) {
            mhwd::Config config = (*iterator);

            cout << config.getName() << "-" << config.getVersion() << "  ";
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
