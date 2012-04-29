/**
 * @file config.cpp
 * Vita::config definition.
 *
 * Licensed under the terms of the MIT/X11 license.
 * Copyright (c) 2008 Vita Smid <me@ze.phyr.us>
 *
 * $Id: config.cpp 17 2008-08-11 17:46:13Z zephyr $
 */

#include <fstream>
#include <iomanip>
#include <iostream>

#include <config.hpp>

namespace Vita {

void config::stripComment(string& line) {
    size_t pos = line.find_first_of(config::COMMENT_DELIM);
    if (pos != string::npos) {
        line.erase(pos);
    }
}

bool config::loadFile(string path) {
    std::ifstream file(path.c_str(), std::ios::in);

    if (!file.is_open()) {
        return false;
    }

    string line, key, value;
    std::vector<string> parts;

    while (!file.eof()) {
        std::getline(file, line);
        stripComment(line);

        if (line.trim().empty()) {
            continue;
        }

        parts = line.explode("=");
        key = parts.front().trim();
        value = parts.back().trim();
        values[key] = value;
    }

    file.close();
    return true;
}

	bool config::saveFile(string path) const {
		std::ofstream file(path.c_str(), std::ios::out);

		if (!file.is_open()) {
			return false;
		}

		std::map<string, string>::const_iterator iter;

		for (iter = values.begin(); iter != values.end(); iter++){
			file << iter->first << " = " << iter->second << std::endl;
		}

		file.close();
		return true;
	}

	string config::get(string key) {
		return values[key];
	}

	void config::set(string key, string value) {
		values[key] = value;
	}

	void config::clear() {
		values.clear();
	}

	string config::dump() const {
		std::map<string, string>::const_iterator iter;
		string result;
		for (iter = values.begin(); iter != values.end(); iter++){
			result += iter->first + "=" + iter->second + ";";
		}
		return result;
	}

} // namespace Vita
