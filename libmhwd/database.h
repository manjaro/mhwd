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

#ifndef DATABASE_H
#define DATABASE_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "string.hpp"


namespace mhwd {
    class Database
    {
    public:
        Database(std::string path);

        std::vector<std::string> getContent() { return content; }
        bool setContent(std::vector<std::string> content);
        bool add(std::string str);
        bool remove(std::string str);
        bool isAvailable(std::string str);
        bool isValid() { return valid; }

    private:
        std::string path;
        bool valid;
        std::vector<std::string> content;

        bool readDatabase();
    };
}

#endif // DATABASE_H
