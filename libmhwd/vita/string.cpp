/**
 * @file string.cpp
 * Vita::string definition.
 *
 * Licensed under the terms of the MIT/X11 license.
 * Copyright (c) 2008 Vita Smid <me@ze.phyr.us>
 *
 * $Id: string.cpp 17 2008-08-11 17:46:13Z zephyr $
 */

#include <string.hpp>

namespace Vita {

	string string::toLower() const {
		string result;
		for (size_t i = 0; i < this->length(); i++) {
			result += tolower((*this)[i]);
		}
		return result;
	}

	string string::toUpper() const {
		string result;
		for (size_t i = 0; i < this->length(); i++) {
			result += toupper((*this)[i]);
		}
		return result;
	}

	string string::ucfirst() const {
		string result = *this;
		result[0] = toupper(result[0]);
		return result;
	}

	string string::lcfirst() const {
		string result = *this;
		result[0] = tolower(result[0]);
		return result;
	}

	string string::operator+(int operand) const {
		return (*this) + string::toStr<int>(operand);
	}

	string string::operator+(long int operand) const {
		return (*this) + string::toStr<long int>(operand);
	}

	string string::operator+(double operand) const {
		return (*this) + string::toStr<double>(operand);
	}

	string string::operator+(float operand) const {
		return (*this) + string::toStr<float>(operand);
	}

	string string::replace(const string& search, const string& replace, size_t limit) const {
		string result;
		size_t previous = 0, current;

		current = this->find(search);

		while (current != npos && limit) {
			result += this->substr(previous, current - previous);
			result += replace;
			previous = current + search.length();
			current = this->find(search, previous);
			limit--;
		}
		result += this->substr(previous);
		return result;
	}

	std::vector<string> string::explode(const string& delimiter) const {
		std::vector<string> result;
		size_t previous = 0, current;

		current = this->find(delimiter);

		while (current != npos) {
			result.push_back(this->substr(previous, current - previous));
			previous = current + delimiter.length();
			current = this->find(delimiter, previous);
		}
		result.push_back(this->substr(previous));
		return result;
	}

	string string::trim(const string& what) const {
		string result = *this;
		size_t pos = result.find_first_not_of(what);
		result.erase(0, pos);
		pos = result.find_last_not_of(what);
		result.erase(pos + 1);
		return result;
	}

} // namespace Vita
