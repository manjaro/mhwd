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

#include "database.h"



mhwd::Database::Database(std::string path) :
    path(path)
{
    valid = readDatabase();
}



bool mhwd::Database::isAvailable(std::string str) {
    str = Vita::string(str).toLower().trim();

    for (std::vector<std::string>::const_iterator iterator = content.begin(); iterator != content.end(); iterator++) {
        if (*iterator == str)
            return true;
    }

    return false;
}



bool mhwd::Database::setContent(std::vector<std::string> content) {
    std::ofstream file(path.c_str(), std::ios::out);

    if (!file.is_open())
        return false;

    Database::content.clear();

    for (std::vector<std::string>::const_iterator iterator = content.begin(); iterator != content.end(); iterator++) {
        Database::content.push_back(Vita::string(*iterator).toLower().trim());
        file << Database::content.back() << std::endl;
    }

    file.close();

    return true;
}



bool mhwd::Database::add(std::string str) {
    str = Vita::string(str).toLower().trim();
    std::vector<std::string> content = Database::content;

    for (std::vector<std::string>::const_iterator iterator = content.begin(); iterator != content.end(); iterator++) {
        if (*iterator == str)
            return true;
    }

    content.push_back(str);

    return setContent(content);
}



bool mhwd::Database::remove(std::string str) {
    str = Vita::string(str).toLower().trim();
    std::vector<std::string> content = Database::content;

    for (std::vector<std::string>::iterator iterator = content.begin(); iterator != content.end(); iterator++) {
        if (*iterator == str)
            content.erase(iterator);
    }

    return setContent(content);
}



// Private



bool mhwd::Database::readDatabase() {
    std::ifstream file(path.c_str(), std::ios::in);
    if (!file.is_open())
        return false;

    Vita::string line;

    while (!file.eof()) {
        getline(file, line);

        size_t pos = line.find_first_of('#');
        if (pos != std::string::npos)
            line.erase(pos);

        if (line.trim().empty())
            continue;

        content.push_back(line.toLower().trim());
    }

    file.close();

    return true;
}
