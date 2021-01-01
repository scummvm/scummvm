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

#ifndef COMMON_CONFIG_MANAGER_H
#define COMMON_CONFIG_MANAGER_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/singleton.h"
#include "common/str.h"
#include "common/hash-str.h"

namespace Common {

/**
 * @defgroup common_config Configuration manager
 * @ingroup common
 *
 * @brief  The (singleton) configuration manager, used to query and set configuration
 *         values using string keys.
 *
 * @{
 */

class WriteStream;
class SeekableReadStream;

/**
 * The (singleton) configuration manager, used to query & set configuration
 * values using string keys.
 *
 * TBD: Implement the callback based notification system (outlined below)
 *       which sends out notifications to interested parties whenever the value
 *       of some specific (or any) configuration key changes.
 */
class ConfigManager : public Singleton<ConfigManager> {

public:

	class Domain {
	private:
		StringMap _entries;
		StringMap _keyValueComments;
		String    _domainComment;

	public:
		typedef StringMap::const_iterator const_iterator;
		const_iterator begin() const { return _entries.begin(); } /*!< Return the beginning position of configuration entries. */
		const_iterator end()   const { return _entries.end(); }   /*!< Return the ending position of configuration entries. */

		bool           empty() const { return _entries.empty(); } /*!< Return true if the configuration is empty, i.e. has no [key, value] pairs, and false otherwise. */

		bool           contains(const String &key) const { return _entries.contains(key); } /*!< Check whether the domain contains a @p key. */
        /** Return the configuration value for the given key.
		 *  If no entry exists for the given key in the configuration, it is created.
		 */
		String        &operator[](const String &key) { return _entries[key]; }
		/** Return the configuration value for the given key.
		 *  @note This function does *not* create a configuration entry
		 *  for the given key if it does not exist.
		 */
		const String  &operator[](const String &key) const { return _entries[key]; }

		void           setVal(const String &key, const String &value) { _entries.setVal(key, value); } /*!< Assign a @p value to a @p key. */

		String        &getVal(const String &key) { return _entries.getVal(key); } /*!< Retrieve the value of a @p key. */
		const String  &getVal(const String &key) const { return _entries.getVal(key); } /*!< @overload */
         /**
          * Retrieve the value of @p key if it exists and leave the referenced variable unchanged if the key does not exist.
          * @return True if the key exists, false otherwise.
          * You can use this method if you frequently attempt to access keys that do not exist.
          */
		bool          tryGetVal(const String &key, String &out) const { return _entries.tryGetVal(key, out); }

		void           clear() { _entries.clear(); } /*!< Clear all configuration entries in the domain. */

		void           erase(const String &key) { _entries.erase(key); } /*!< Remove a key from the domain. */

		void           setDomainComment(const String &comment); /*!< Add a @p comment for this configuration domain. */
		const String  &getDomainComment() const; /*!< Retrieve the comment of this configuration domain. */

		void           setKVComment(const String &key, const String &comment); /*!< Add a key-value @p comment to a @p key. */
		const String  &getKVComment(const String &key) const; /*!< Retrieve the key-value comment of a @p key. */
		bool           hasKVComment(const String &key) const; /*!< Check whether a @p key has a key-value comment. */
	};
    
	/** A hash map of existing configuration domains. */
	typedef HashMap<String, Domain, IgnoreCase_Hash, IgnoreCase_EqualTo> DomainMap;

	/** The name of the application domain (normally 'scummvm'). */
	static char const *const kApplicationDomain;

	/** The transient (pseudo) domain. */
	static char const *const kTransientDomain;

	/** The name of keymapper domain used to store the key maps. */
	static char const *const kKeymapperDomain;

#ifdef USE_CLOUD
	/** The name of cloud domain used to store the user's tokens. */
	static char const *const kCloudDomain;
#endif

	void                     loadDefaultConfigFile(); /*!< Load the default configuration file. */
	void                     loadConfigFile(const String &filename); /*!< Load a specific configuration file. */

	/**
	 * Retrieve the config domain with the given name.
	 * @param domName Name of the domain to retrieve.
	 * @return Pointer to the domain, or 0 if the domain does not exist.
	 */
	Domain *			getDomain(const String &domName);
	const Domain            *getDomain(const String &domName) const; /*!< @overload */


    /**
	 * @name Generic access methods
	 * @brief No domain specified, use the values from the
	 *        various domains in the order of their priority.
	 * @{
	 */

	bool                     hasKey(const String &key) const; /*!< Check if a given @p key exists. */
	const String            &get(const String &key) const;    /*!< Get the value of a @p key. */
	void                     set(const String &key, const String &value); /*!< Assign a @p value to a @p key. */
    /** @} */

	/**
	 * Update a configuration entry for the active domain and flush
	 * the configuration file to disk if the value changed.
	 */
	void				setAndFlush(const String &key, const Common::String &value);

#if 1
    /**
	 * @name Domain-specific access methods
	 * @brief Access one specific domain and modify it.
	 *
	 * TBD: Get rid of most of those if possible, or at least reduce
	 * their usage, by using getDomain as often as possible. For example in the
	 * options dialog code.
	 * @{
	 */

	bool                     hasKey(const String &key, const String &domName) const; /*!< Check if a given @p key exists in the @p domName domain. */
	const String            &get(const String &key, const String &domName) const; /*!< Get the value of a @p key from the @p domName domain. */
	void                     set(const String &key, const String &value, const String &domName); /*!< Assign a @p value to a @p key in the @p domName domain. */

	void                     removeKey(const String &key, const String &domName); /*!< Remove a @p key to a @p key from the @p domName domain. */
	/** @} */
#endif

    /**
	 * @name Additional convenience accessors
	 * @{
	 */

	int                      getInt(const String &key, const String &domName = String()) const; /*!< Get integer value. */
	bool                     getBool(const String &key, const String &domName = String()) const; /*!< Get Boolean value. */
	void                     setInt(const String &key, int value, const String &domName = String()); /*!< Set integer value. */
	void                     setBool(const String &key, bool value, const String &domName = String()); /*!< Set Boolean value. */

	void                     registerDefault(const String &key, const String &value); /*!< Register a value as the default. */
	void                     registerDefault(const String &key, const char *value); /*!< @overload */
	void                     registerDefault(const String &key, int value); /*!< @overload */
	void                     registerDefault(const String &key, bool value); /*!< @overload */

	void                     flushToDisk(); /*!< Flush configuration to disk. */

	void                     setActiveDomain(const String &domName); /*!< Set the given domain as active. */
	Domain                  *getActiveDomain() { return _activeDomain; } /*!< Get the active domain. */
	const Domain            *getActiveDomain() const { return _activeDomain; } /*!< @overload */
	const String            &getActiveDomainName() const { return _activeDomainName; } /*!< Get the name of the active domain. */

	void                     addGameDomain(const String &domName); /*!< Add a new game domain. */
	void                     removeGameDomain(const String &domName); /*!< Remove a game domain. */
	void                     renameGameDomain(const String &oldName, const String &newName); /*!< Rename a game domain. */

	void                     addMiscDomain(const String &domName); /*!< Add a miscellaneous domain. */
	void                     removeMiscDomain(const String &domName); /*!< Remove a miscellaneous domain. */
	void                     renameMiscDomain(const String &oldName, const String &newName); /*!< Rename a miscellaneous domain. */

	bool                     hasGameDomain(const String &domName) const; /*!< Check if a specific game domain exists in the DomainMap. */
	bool                     hasMiscDomain(const String &domName) const; /*!< Check if a specific miscellaneous domain exists in the DomainMap. */

	const DomainMap         &getGameDomains() const { return _gameDomains; } /*!< Return all game domains in the DomainMap. */
	DomainMap::iterator      beginGameDomains() { return _gameDomains.begin(); } /*!< Return the beginning position of game domains. */
	DomainMap::iterator      endGameDomains() { return _gameDomains.end(); } /*!< Return the ending position of game domains. */

	static void              defragment(); /*!< Move the configuration in memory to reduce fragmentation. */
	void                     copyFrom(ConfigManager &source); /*!< Copy from a ConfigManager instance. */
	/** @} */
private:
	friend class Singleton<SingletonBaseType>;
	ConfigManager();

	void			loadFromStream(SeekableReadStream &stream);
	void			addDomain(const String &domainName, const Domain &domain);
	void			writeDomain(WriteStream &stream, const String &name, const Domain &domain);
	void			renameDomain(const String &oldName, const String &newName, DomainMap &map);

	Domain			_transientDomain;
	DomainMap		_gameDomains;
	DomainMap		_miscDomains; // Any other domains
	Domain			_appDomain;
	Domain			_defaultsDomain;

	Domain			_keymapperDomain;

#ifdef USE_CLOUD
	Domain			_cloudDomain;
#endif

	Array<String>	_domainSaveOrder;

	String			_activeDomainName;
	Domain *		_activeDomain;

	String			_filename;
};

/** @} */

} // End of namespace Common

/** Shortcut for accessing the configuration manager. */
#define ConfMan		Common::ConfigManager::instance()

#endif
