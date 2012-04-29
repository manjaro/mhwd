/**
 * @file config.hpp
 * Vita::config declaration.
 *
 * Licensed under the terms of the MIT/X11 license.
 * Copyright (c) 2008 Vita Smid <me@ze.phyr.us>
 *
 * $Id: config.hpp 17 2008-08-11 17:46:13Z zephyr $
 */

#ifndef INC_VITA_CONFIG_HPP
#define INC_VITA_CONFIG_HPP

#include <map>
#include <string.hpp>

namespace Vita {

	/**
	 * Simple tool for reading and writing UNIX-like configuration files.
	 *
	 * Comments are delimited by the "#" sign. Values are written in the form <tt>key = value</tt>,
	 * one value per line. Whitespace is ignored.
	 */
	class config {
		protected:
			/**
			 * The key-value map.
			 */
			std::map<string, string> values;

			/**
			 * Remove comments (if any) from a line.
			 */
			void stripComment(string& line);

		public:
			/**
			 * Single-line comment delimiter.
			 */
			static const char COMMENT_DELIM = '#';

			/**
			 * Default constructor. Does nothing.
			 */
			config() {};

			/**
			 * Construct the object and load values from a file.
			 *
			 * @see loadFile()
			 */
			config(string path) { loadFile(path); };

			/**
			 * Load values from a file.
			 *
			 * @param path The file to load.
			 * @return true on success, false otherwise.
			 */
			bool loadFile(string path);

			/**
			 * Write values to a file.
			 *
			 * Neither original formatting nor comments are preserved.
			 *
			 * @param path The file to write.
			 * @return true on success, false otherwise.
			 */
			bool saveFile(string path) const;

			/**
			 * Get a configuration value.
			 *
			 * @param key The value's key.
			 * @return The value.
			 */
			string get(string key);

			/**
			 * Set a configuration value.
			 *
			 * @param key The key to be altered.
			 * @param value The new value.
			 */
			void set(string key, string value);

			/**
			 * Clear all configuration values.
			 */
			void clear();

			/**
			 * Return a dump of all values in a single string.
			 *
			 * The dump has the format <tt>key1=value1;key2=value2;...</tt>
			 */
			string dump() const;
	};

} // namespace Vita

#endif // INC_VITA_CONFIG_HPP
