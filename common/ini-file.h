/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_INI_FILE_H
#define COMMON_INI_FILE_H

#include "common/hash-str.h"
#include "common/list.h"
#include "common/str.h"

namespace Common {

/**
 * @defgroup common_ini_file INI files
 * @ingroup common
 *
 * @brief API for operations on INI configuration files.
 *
 *		
 * @{
 */

class SeekableReadStream;
class WriteStream;

/**
 * This class allows for reading and writing INI-style config files.
 *
 * Lines starting with a '#' are ignored (i.e. treated as comments).
 * Some effort is made to preserve comments, though.
 *
 * This class makes no attempts to provide fast access to key/value pairs.
 * In particular, it stores all sections and key/value pairs in lists, not
 * in dictionaries/maps. This makes it very easy to read/write the data
 * from/to files, but of course is not appropriate for fast access.
 * The main reason is that this class is indeed geared toward doing precisely
 * that.
 */
class INIFile {
public:
	struct KeyValue {
		String key;     /*!< Key of the configuration entry. */
		String value;   /*!< Value of the configuration entry. */
		String comment; /*!< Comment within an INI file. */
	};

	typedef List<KeyValue> SectionKeyList; /*!< A list of all key/value pairs in this section. */

	/** A section in an INI file.
	 *
	 * Corresponds to the following:
	 * @code
	 * [mySection]
	 * key=value
	 * @endcode
	 * Comments are also stored, for convenience of users who prefer to edit
	 * INI files manually.
	 */
	struct Section {
		String name;         /*!< Name of the section. */
		List<KeyValue> keys; /*!< List of all keys in this section. */
		String comment;      /*!< Comment within the section. */

		bool hasKey(const String &key) const; /*!< Check whether the section has a @p key. */
		const KeyValue* getKey(const String &key) const; /*!< Get the value assigned to a @p key. */
		void setKey(const String &key, const String &value); /*!< Assign a @p value to a @p key. */
		void removeKey(const String &key); /*!< Remove a @p key from this section. */
		const SectionKeyList getKeys() const { return keys; } /*!< Get a list of all keys in the section. */
	};

	typedef List<Section> SectionList; /*!< A list of all sections in this INI file. */

public:
	INIFile();
	~INIFile() {}

	// TODO: Maybe add a copy constructor etc.?

	/**
	 * Check whether the given string is a valid section or key name.
	 * For that, it must only consist of letters, numbers, dashes, and
	 * underscores. In particular, whitespace and "#", "=", "[", "]"
	 * are not valid.
	 */
	bool isValidName(const String &name) const;

	/** Reset everything stored in this INI file. */
	void	clear();

	bool	loadFromFile(const String &filename); /*!< Load configuration from a file. */
	bool	loadFromSaveFile(const String &filename); /*!< Load configuration from a save file. */
	bool	loadFromStream(SeekableReadStream &stream); /*!< Load configuration from a @ref SeekableReadStream. */
	bool	saveToFile(const String &filename); /*!< Save the current configuration to a file. */
	bool	saveToSaveFile(const String &filename); /*!< Save the current configuration to a save file. */
	bool	saveToStream(WriteStream &stream); /*!< Save the current configuration to a @ref WriteStream. */

	bool	hasSection(const String &section) const; /*!< Check whether the INI file has a section with the specified name. */
	void	addSection(const String &section); /*!< Add a section with the specified name to the INI file. */
	void	removeSection(const String &section); /*!< Remove the @p section from the INI file. */
	void	renameSection(const String &oldName, const String &newName); /*!< Rename the INI file from @p oldName to @p newName. */

	void	setDefaultSectionName(const String &name); /*!< Set initial section name for section-less INI files. */

	bool	hasKey(const String &key, const String &section) const; /*!< Check whether the @p section has a @p key. */
	bool	getKey(const String &key, const String &section, String &value) const; /*!< Get the @p value of a @p key in a @p section. */
	void	setKey(const String &key, const String &section, const String &value); /*!< Assign a @p value to a @p key in a @p section. */
	void	removeKey(const String &key, const String &section); /*!< Remove a @p key from this @p section. */

	const SectionList getSections() const { return _sections; } /*!< Get a list of sections in this INI file. */
	const SectionKeyList getKeys(const String &section) const; /*!< Get a list of keys in a @p section. */

	void listKeyValues(StringMap &kv); /*!< Get a list of all key/value pairs in this INI file. */

	void allowNonEnglishCharacters(); /*!< Allow non-English characters in this INI file. */

private:
	String		_defaultSectionName;
	SectionList _sections;
	bool _allowNonEnglishCharacters;

	Section *getSection(const String &section);
	const Section *getSection(const String &section) const;
};

/** @} */

} // End of namespace Common

#endif
