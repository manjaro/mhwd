/*
 *  mhwd - Manjaro Hardware Detection
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

#include "config.h"



mhwd::Config::Config(string path)
{
    priority = 0;
    freedriver = true;
    Config::path = path;

    configValid = readConfig(path);
}



bool mhwd::Config::operator==(const mhwd::Config& compare) {
    return (path == compare.path);
}




// Private



bool mhwd::Config::readConfig(const Vita::string path) {
    if (IDs.empty())
        addNewIDsGroup();

    ifstream file(path.c_str(), ios::in);

    if (!file.is_open())
        return false;

    Vita::string line, key, value;
    vector<Vita::string> parts;

    while (!file.eof()) {
        getline(file, line);

        size_t pos = line.find_first_of('#');
        if (pos != string::npos)
            line.erase(pos);

        if (line.trim().empty())
            continue;

        parts = line.explode("=");
        key = parts.front().trim().toLower();
        value = parts.back().trim("\"").trim();

        // Read in extern file
        if (value.size() > 1 && value.substr(0, 1) == ">") {
            ifstream file(getRightPath(value.substr(1)).c_str(), ios::in);
            if (!file.is_open())
                return false;

            Vita::string line;
            value.clear();

            while (!file.eof()) {
                getline(file, line);

                size_t pos = line.find_first_of('#');
                if (pos != string::npos)
                    line.erase(pos);

                if (line.trim().empty())
                    continue;

                value += " " + line.trim();
            }

            file.close();

            value = value.trim();

            // remove all multiple spaces
            while (value.find("  ") != string::npos) {
                value = value.replace("  ", " ");
            }
        }


        if (key == "include") {
            readConfig(getRightPath(value));
        }
        else if (key == "name") {
            name = value;
        }
        else if (key == "info") {
            info = value;
        }
        else if (key == "priority") {
            priority = value.convert<int>();
        }
        else if (key == "freedriver") {
            value = value.toLower();

            if (value == "false")
                freedriver = false;
            else if (value == "true")
                freedriver = true;
        }
        else if (key == "classids") {
            if (!IDs.back().classIDs.empty())
                addNewIDsGroup();

            IDs.back().classIDs = getIDs(value);
        }
        else if (key == "vendorids") {
            if (!IDs.back().vendorIDs.empty())
                addNewIDsGroup();

            IDs.back().vendorIDs = getIDs(value);
        }
        else if (key == "deviceids") {
            if (!IDs.back().deviceIDs.empty())
                addNewIDsGroup();

            IDs.back().deviceIDs = getIDs(value);
        }
    }

    file.close();

    // Append * to all empty vectors
    for (vector<IDsGroup>::iterator iterator = IDs.begin(); iterator != IDs.end(); iterator++) {
        if ((*iterator).classIDs.empty())
            (*iterator).classIDs.push_back("*");

        if ((*iterator).vendorIDs.empty())
            (*iterator).vendorIDs.push_back("*");

        if ((*iterator).deviceIDs.empty())
            (*iterator).deviceIDs.push_back("*");
    }

    return true;
}



vector<string> mhwd::Config::getIDs(Vita::string str) {
    vector<Vita::string> work = str.toLower().explode(" ");
    vector<string> final;

    for (vector<Vita::string>::const_iterator iterator = work.begin(); iterator != work.end(); iterator++) {
        if (*iterator != "")
            final.push_back(*iterator);
    }

    return final;
}



void mhwd::Config::addNewIDsGroup() {
    IDsGroup group;
    IDs.push_back(group);
}



Vita::string mhwd::Config::getRightPath(Vita::string str) {
    str = str.trim();

    if (str.size() <= 0 || str.substr(0, 1) == "/")
        return str;

    return path.substr(0, path.find_last_of('/')) + "/" + str;
}
