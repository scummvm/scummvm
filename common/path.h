/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_PATH_H
#define COMMON_PATH_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/str-array.h"

#ifdef CXXTEST_RUNNING
class PathTestSuite;
#endif

namespace Common {

/**
 * @defgroup common_path Path
 * @ingroup common
 *
 * @brief API for working with paths.
 *
 * @{
 */

/**
 * Simple path class. Allows simple conversion to/from path strings with
 * arbitrary directory separators, providing a common representation.
 *
 * Internally, this is just a simple wrapper around a String, using
 * "/" as a directory separator.
 * It escapes it using "|" if / is used inside a path component.
 */
class Path {
#ifdef CXXTEST_RUNNING
	friend class ::PathTestSuite;
#endif

private:
#ifndef RELEASE_BUILD
	static bool _shownSeparatorCollisionWarning;
#endif

	// The directory separator
	static const char SEPARATOR = '/';
	// The escape prefix character
	static const char ESCAPE = '|';
	// How the prefix character is escaped (doubling)
	static const char ESCAPED_ESCAPE = '|';
	// How the separator is escaped
	static const char ESCAPED_SEPARATOR  = '\\';

	String _str;

	/**
	 * Escapes a path:
	 * - all ESCAPE are encoded to ESCAPE ESCAPED_ESCAPE
	 * - all SEPARATOR are encoded to ESCAPE ESCAPED_SEPARATOR if srcSeparator is not SEPARATOR
	 * - replaces all srcSeparator to SEPARATOR
	 *
	 * @param dst The String object to append result to.
	 * @param srcSeparator The directory separator used in source string.
	 * @param begin Begin of range
	 * @param end End of the range excluded.
	 */
	static void escape(String &dst, char srcSeparator, const char *src, const char *end = nullptr);

	/**
	 * Invert of escape
	 * - all ESCAPE ESCAPED_ESCAPE sequences are restored to ESCAPE
	 * - all ESCAPE ESCAPED_SEPARATOR sequences are restored to SEPARATOR
	 * - all SEPARATOR are replaced by dstSeparator
	 * Collisions in resulting path are checked and warned once.
	 *
	 * @param dstSeparator The directory separator to use in resulting string.
	 * @param begin Start of the range.
	 * @param end End of the range excluded.
	 */
	static String unescape(char dstSeparator, const char *src, const char *end = nullptr);

	/**
	 * Checks if the provided range can be unescaped
	 *
	 * @param atBegin Whether this range will end up at the begin of path or not.
	 * @param begin Start of the range.
	 * @param end End of the range excluded.
	 *
	 */
	static bool canUnescape(bool willBeAtBegin, bool wasAtBegin,
	                        const char *begin, const char *end = nullptr);

	/**
	 * Determines if the provided string will need to be encode and can't be copied as is
	 *
	 * @param str The string to check
	 * @param separator The directory separator used in str
	 */
	static inline bool needsEncoding(const char *str, char separator) {
		return *str && // String is not empty and
		       (separator != SEPARATOR || // separator is not the one we use
		        *str == ESCAPE); // or string begins with ESCAPE
	}

	/**
	 * Encodes a path:
	 * - escapes when needed
	 * - all separators are changed to SEPARATOR
	 * - a ESCAPE is prepended to string if escaping took place
	 * - if separator is \x00 str is assumed to be only one component
	 * - using ESCAPE as separator is not supported
	 *
	 * @param str The string to encode
	 * @param separator The directory separator used in str
	 */
	static String encode(const char *str, char separator);

	/**
	 * Creates a path using [begin, end) path data from ourselves.
	 * begin and end are assumed to lie inside _str
	 *
	 * @param begin Start of the range.
	 * @param end End of the range excluded.
	 *
	 */
	Path extract(const char *begin, const char *end = nullptr) const;

	/** Finds the last separator before @p last. */
	size_t findLastSeparator(size_t last = String::npos) const;

	/**
	 * Apply a reduction function on every path components
	 * and returns the reducer result
	 *
	 * @param reducer The function which reduces the path components
	 * @param value The initial value passed to reducer
	 */
	template<typename T>
	T reduceComponents(T (*reducer)(T value, const String &element, bool last), T value) const;

	/**
	 * Compares a path component by component by calling a comparator function
	 * The function bails out as soon as the comparison fails.
	 *
	 * @param comparator The function which compares the path components
	 * @param other The other path to compare with
	 */
	bool compareComponents(bool (*comparator)(const String &x, const String &y), const Path &other) const;

	/**
	 * Determines if the path is escaped
	 */
	inline bool isEscaped() const {
		// c_str() returns a null terminated string, if _str is empty we get \0
		return *_str.c_str() == ESCAPE;
	}

	/**
	 * Returns the suffix in this path after @p other path
	 * Returns nullptr if @p other isn't a prefix
	 */
	const char *getSuffix(const Common::Path &other) const;

public:
	/**
	 * A separator to use when building path conataining only base names
	 * This must not be used with toString(), use baseName() instead.
	 */
	static const char kNoSeparator = '\x00';

	/**
	 * The platform native separator.
	 * This is used when accessing files on disk
	 */
#if defined(WIN32)
	static const char kNativeSeparator = '\\';
#else
	static const char kNativeSeparator = '/';
#endif

	/**
	 * Hash and comparator for Path with following changes:
	 * * case-insensitive
	 * * decoding of punycode
	 * * Matching ':' and '/' inside path components to
	 * This allows a path "Sound Manager 3.1 / SoundLib<separator>Sound"
	 * to match both "xn--Sound Manager 3.1  SoundLib-lba84k/Sound"
	 * and "Sound Manager 3.1 : SoundLib/Sound"
	 */
	struct IgnoreCaseAndMac_EqualTo {
		bool operator()(const Path &x, const Path &y) const { return x.equalsIgnoreCaseAndMac(y); }
	};

	struct IgnoreCaseAndMac_Hash {
		uint operator()(const Path &x) const { return x.hashIgnoreCaseAndMac(); }
	};

	/**
	 * Hash and comparator for Path with case-insensitivity
	 * This may not be used when punycoded and Mac files could be found
	 * but this should be safe in other places and costs less
	 */
	struct IgnoreCase_EqualTo {
		bool operator()(const Path &x, const Path &y) const { return x.equalsIgnoreCase(y); }
	};

	struct IgnoreCase_Hash {
		uint operator()(const Path &x) const { return x.hashIgnoreCase(); }
	};

	struct EqualTo {
		bool operator()(const Path &x, const Path &y) const { return x.equals(y); }
	};

	struct Hash {
		uint operator()(const Path &x) const { return x.hash(); }
	};

	/** Construct a new empty path. */
	Path() {}

	/** Construct a copy of the given path. */
	Path(const Path &path) : _str(path._str) { }

	/**
	 * Construct a new path from the given NULL-terminated C string.
	 *
	 * @param str       A NULL-terminated C string representing a path,
	 *                  e.g. "foo/bar/baz"
	 * @param separator The directory separator used in the path string.
	 *                  Use kNoSeparator if there is no separator.
	 *                  Defaults to '/'.
	 */
	Path(const char *str, char separator = '/') :
		_str(needsEncoding(str, separator) ? encode(str, separator) : str) { }

	/**
	 * Construct a new path from the given String.
	 * This is explicit to limit mixing strings and paths in the codebase.
	 *
	 * @param str       A String representing a path, e.g. "foo/bar/baz"
	 * @param separator The directory separator used in the path string.
	 *                  Use kNoSeparator if there is no separator.
	 *                  Defaults to '/'.
	 */
	explicit Path(const String &str, char separator = '/') :
		_str(needsEncoding(str.c_str(), separator) ? encode(str.c_str(), separator) : str) { }

	/**
	 * Converts a path to a string using the given directory separator.
	 * Collisions in resulting path are checked and warned once.
	 *
	 * @param separator The character used to separate directory names.
	 *                  kNoSeparator must not be used here.
	 *                  Defaults to '/'.
	 */
	String toString(char separator = '/') const;

	/**
	 * Clears the path object
	 */
	void clear() { _str.clear(); }

	/**
	 * Returns the Path for the parent directory of this path.
	 *
	 * Appending the getLastComponent() of a path to getParent() returns a path
	 * identical to the original path.
	 */
	Path getParent() const;

	/**
	 * Returns the last component of this path.
	 *
	 * Appending the getLastComponent() of a path to getParent() returns a path
	 * identical to the original path.
	 */
	Path getLastComponent() const;

	/**
	 * Returns the last non-empty component of this path.
	 * Compared to getLastComponent(), baseName() doesn't
	 * return the trailing / if any.
	 */
	String baseName() const;

	/**
	 * Returns number of components in this path,
	 */
	int numComponents() const;

	/** Check whether this path is identical to path @p x. */
	bool operator==(const Path &x) const {
		return _str == x._str;
	}

	/** Check whether this path is different than path @p x. */
	bool operator!=(const Path &x) const {
		return _str != x._str;
	}

	/**
	 * Check whether this path is identical to path @p x.
	 */
	bool equals(const Path &x) const {
		return _str.equals(x._str);
	}
	/**
	 * Check whether this path is identical to path @p x.
	 * Ignores case
	 */
	bool equalsIgnoreCase(const Path &x) const;
	/**
	 * Check whether this path is identical to path @p x.
	 * Ignores case, punycode and Mac path separator.
	 */
	bool equalsIgnoreCaseAndMac(const Path &x) const;

	/**
	 * Calculate a case sensitive hash of path
	 */
	uint hash() const;
	/**
	 * Calculate a case insensitive hash of path
	 */
	uint hashIgnoreCase() const;
	/**
	 * Calculate a hash of path which is case insensitive.
	 * Ignores case, punycode and Mac path separator.
	 */
	uint hashIgnoreCaseAndMac() const;

	bool operator<(const Path &x) const;

	/** Return if this path is empty */
	bool empty() const {
		return _str.empty();
	}

	/** Assign a given path to this path. */
	Path &operator=(const Path &path) {
		_str = path._str;
		return *this;
	}

	/** @overload */
	Path &operator=(const char *str) {
		set(str);
		return *this;
	}

	/** @overload */
	Path &operator=(const String &str) {
		set(str.c_str());
		return *this;
	}

	void set(const char *str, char separator = '/') {
		if (needsEncoding(str, separator)) {
			_str = encode(str, separator);
		} else {
			_str = str;
		}
	}

	/**
	 * Appends the given path to this path (in-place).
	 * Does not automatically add a directory separator.
	 */
	Path &appendInPlace(const Path &x);

	/** @overload */
	Path &appendInPlace(const String &str, char separator = '/') {
		appendInPlace(str.c_str(), separator);
		return *this;
	}

	/** @overload */
	Path &appendInPlace(const char *str, char separator = '/');

	/**
	 * Returns this path with the given path appended (out-of-place).
	 * Does not automatically add a directory separator.
	 */
	WARN_UNUSED_RESULT Path append(const Path &x) const {
		Path temp(*this);
		temp.appendInPlace(x);
		return temp;
	}

	/** @overload */
	WARN_UNUSED_RESULT Path append(const String &str, char separator = '/') const {
		return append(str.c_str(), separator);
	}

	/** @overload */
	WARN_UNUSED_RESULT Path append(const char *str, char separator = '/') const {
		Path temp(*this);
		temp.appendInPlace(str, separator);
		return temp;
	}

	/**
	 * Appends exactly one component, without any separators
	 * and prepends a separator if necessarry
	 */
	WARN_UNUSED_RESULT Path appendComponent(const char *str) const;

	/** @overload */
	WARN_UNUSED_RESULT Path appendComponent(const String &x) const {
		return appendComponent(x.c_str());
	}

	/**
	 * Joins the given path to this path (in-place).
	 * Automatically adds a directory separator.
	 */
	Path &joinInPlace(const Path &x);

	/** @overload */
	Path &joinInPlace(const String &str, char separator = '/') {
		return joinInPlace(str.c_str(), separator);
	}

	/** @overload */
	Path &joinInPlace(const char *str, char separator = '/');

	/**
	 * Returns this path joined with the given path (out-of-place).
	 * Automatically adds a directory separator.
	 */
	WARN_UNUSED_RESULT Path join(const Path &x) const {
		Path temp(*this);
		temp.joinInPlace(x);
		return temp;
	}

	/** @overload */
	WARN_UNUSED_RESULT Path join(const String &str, char separator = '/') const {
		return join(str.c_str(), separator);
	}

	/** @overload */
	WARN_UNUSED_RESULT Path join(const char *str, char separator = '/') const {
		Path temp(*this);
		temp.joinInPlace(str, separator);
		return temp;
	}

	/**
	 * Removes the trainling separators if any in this path (in-place).
	 */
	Path &removeTrailingSeparators();

	/**
	 * Returns whether this path ends with a separator
	 */
	bool isSeparatorTerminated() const { return _str.lastChar() == SEPARATOR; }

	/**
	 * Returns whether this path begins with @p other path
	 */
	bool isRelativeTo(const Common::Path &other) const { return getSuffix(other) != nullptr; }

	/**
	 * Returns a new path relative to the @p other one.
	 * Returns a copy this if it wasn't relative to.
	 */
	Path relativeTo(const Common::Path &other) const;

	/**
	 * Convert path from Punycode
	 */
	Path punycodeDecode() const;

	/**
	* Convert path to Punycode
	 */
	Path punycodeEncode() const;

	/**
	 * Returns whether the path will need to be Punycoded
	 */
	bool punycodeNeedsEncode() const;

	/**
	 * Convert all characters in the path to lowercase.
	 *
	 * Be aware that this only affects the case of ASCII characters. All
	 * other characters will not be touched at all.
	 */
	void toLowercase() {
		// Escapism is not changed by changing case
		_str.toLowercase();
	}

	/**
	 * Convert all characters in the path to uppercase.
	 *
	 * Be aware that this only affects the case of ASCII characters. All
	 * other characters will not be touched at all.
	 */
	void toUppercase() {
		// Escapism is not changed by changing case
		_str.toUppercase();
	}

	/**
	* Check pattern match similar matchString
	 */
	bool matchPattern(const Path &pattern) const;

	/**
	 * Normalize path to a canonical form. In particular:
	 * - trailing separators are removed:  /foo/bar/ -> /foo/bar
	 * - double separators (= empty components) are removed:   /foo//bar -> /foo/bar
	 * - dot components are removed:  /foo/./bar -> /foo/bar
	 * - double dot components are removed:  /foo/baz/../bar -> /foo/bar
	 *
	 * @return      the normalized path
	 */
	Path normalize() const;

	/**
	 * Splits into path components. After every component except
	 * last there is an implied separator. First component is empty
	 * if path starts with a separator. Last component is empty if
	 * the path ends with a separator. Other components may be empty if
	 * 2 separots follow each other
	 */
	StringArray splitComponents() const;

	/**
	 * Opposite of splitComponents
	 */
	static Path joinComponents(StringArray::const_iterator begin, StringArray::const_iterator end);
	static Path joinComponents(const StringArray &c) {
		return joinComponents(c.begin(), c.end());
	}

	/**
	 * Use by ConfigManager to store a path in a protected fashion
	 * All components are punyencoded and / is used as a delimiter for all platforms
	 * Under Windows don't encode when it's not needed and make use of \ separator
	 * in this case
	 */
	String toConfig() const;

	/**
	 * Used by ConfigManager to parse a configuration value in a backwards compatible way
	 */
	static Path fromConfig(const String &value);

	/**
	 * Creates a path from a string given by the user
	 */
	static Path fromCommandLine(const String &value);
};

/** @} */

} // End of namespace Common

#endif
