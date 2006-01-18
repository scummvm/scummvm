/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#ifndef COMMON_CONFIG_MANAGER_H
#define COMMON_CONFIG_MANAGER_H

#include "common/array.h"
//#include "common/config-file.h"
#include "common/map.h"
#include "common/singleton.h"
#include "common/str.h"

namespace Common {

struct IgnoreCaseComparator {
  int operator()(const String& x, const String& y) const { return scumm_stricmp(x.c_str(), y.c_str()); }
};

typedef Map<String, String, IgnoreCaseComparator> StringMap;

/**
 * The (singleton) configuration manager, used to query & set configuration
 * values using string keys.
 *
 * @todo Implement the callback based notification system (outlined below)
 *       which sends out notifications to interested parties whenever the value
 *       of some specific (or any) configuration key changes.
 */
class ConfigManager : public Singleton<ConfigManager> {

public:

	class Domain : public StringMap {
	private:
		StringMap _keyValueComments;
		String _domainComment;

	public:
		const String &get(const String &key) const;

		void setDomainComment(const String &comment);
		const String &getDomainComment() const;

		void setKVComment(const String &key, const String &comment);
		const String &getKVComment(const String &key) const;
		bool hasKVComment(const String &key) const;
	};

	typedef Map<String, Domain, IgnoreCaseComparator> DomainMap;

#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
	/** The name of the application domain (normally 'scummvm'). */
	static const String kApplicationDomain;

	/** The transient (pseudo) domain. */
	static const String kTransientDomain;
#else
	static const char *kApplicationDomain;
	static const char *kTransientDomain;
	
	const String _emptyString;
#endif

	void				loadDefaultConfigFile();
	void				loadConfigFile(const String &filename);

	bool				hasKey(const String &key) const;
	bool				hasKey(const String &key, const String &dom) const;

	void				removeKey(const String &key, const String &dom);

	const String &		get(const String &key, const String &dom = String::emptyString) const;
	int					getInt(const String &key, const String &dom = String::emptyString) const;
	bool				getBool(const String &key, const String &dom = String::emptyString) const;

	void				set(const String &key, const String &value, const String &dom = String::emptyString);
	void				set(const String &key, const char *value, const String &dom = String::emptyString);
	void				set(const String &key, int value, const String &dom = String::emptyString);
	void				set(const String &key, bool value, const String &dom = String::emptyString);

	void				registerDefault(const String &key, const String &value);
	void				registerDefault(const String &key, const char *value);
	void				registerDefault(const String &key, int value);
	void				registerDefault(const String &key, bool value);
//	...

	void				flushToDisk();

	void				setActiveDomain(const String &domain);
	const String &		getActiveDomain() const { return _activeDomain; }

//	void				addDomain(const String &name);
	void				removeGameDomain(const String &name);
	void				renameGameDomain(const String &oldName, const String &newName);
	bool				hasGameDomain(const String &domain) const;
	const DomainMap &	getGameDomains() const { return _gameDomains; }

/*
	TODO: Callback/change notification system
	typedef void (*ConfigCallback)(const ConstString &key, void *refCon);

	void   registerCallback(ConfigCallback cfgc, void *refCon, const ConstString &key = String::emptyString)
	void unregisterCallback(ConfigCallback cfgc, const ConstString &key = String::emptyString)
*/

private:
	friend class Singleton<SingletonBaseType>;
	ConfigManager();

	void			loadFile(const String &filename);
	void			writeDomain(FILE *file, const String &name, const Domain &domain);

	Domain			_transientDomain;
	DomainMap		_gameDomains;
	DomainMap		_globalDomains;
	Domain			_defaultsDomain;

	StringList		_domainSaveOrder;

	String			_activeDomain;
	String			_filename;
};

}	// End of namespace Common

/** Shortcut for accessing the configuration manager. */
#define ConfMan		Common::ConfigManager::instance()

#endif
