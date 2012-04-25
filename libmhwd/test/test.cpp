#include <iostream>
#include "mhwd.h"


using namespace std;

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "mhwd.h"

using namespace std;


int main (int argc, char *argv[])
{
    vector<mhwd::Device*> devices = mhwd::getDevices(mhwd::HW_PCI);

    for(unsigned int i = 0; i < devices.size(); i++) {
        cout << setw(30) << devices[i]->getClassName();
        cout << setw(10)<< devices[i]->getClassID();
        cout << setw(40)<< devices[i]->getVendorName();
        cout << setw(10)<< devices[i]->getVendorID();
        cout << setw(30)<< devices[i]->getDeviceName();
        cout << setw(10)<< devices[i]->getDeviceID();
        cout << endl;
    }

    cout << endl << endl;

    mhwd::printDetails(mhwd::HW_USB);
}
