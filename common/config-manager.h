/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

#include "common/list.h"
#include "common/map.h"
#include "common/singleton.h"
#include "common/str.h"
#include "common/util.h"

namespace Common {

/**
 * The (singleton) configuration manager, used to query & set configuration
 * values using string keys.
 *
 * @todo Implement the callback based notification system (outline below)
 *       which sends out notifications to interested parties whenever the value
 *       of some specific (or any) configuration key changes.
 */
class ConfigManager : public Singleton<ConfigManager> {
	struct IgnoreCaseComparator {
	  int operator()(const String& x, const String& y) const { return scumm_stricmp(x.c_str(), y.c_str()); }
	};
	
public:
	class Domain : public Map<String, String, IgnoreCaseComparator> {
	public:
		const String &get(const String &key) const {
			Node *node = findNode(_root, key);
			return node ? node->_value : String::emptyString;
		}
	};

	typedef Map<String, Domain, IgnoreCaseComparator> DomainMap;

	/** The name of the application domain (normally 'scummvm'). */
	static const String kApplicationDomain;

	/** The transient (pseudo) domain. */
	static const String kTransientDomain;

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

//	void				addDomain(const String &name);
	void				removeGameDomain(const String &name);
	void				renameGameDomain(const String &oldName, const String &newName);
	bool				hasGameDomain(const String &domain) const;
	const DomainMap &	getGameDomains() const { return _gameDomains; }

/*
	TODO: Callback/change notification system
	typedef void (*ConfigCallback)(const ConstString &key, void *refCon);

	void   registerCallback(const ConstString &key, ConfigCallback cfgc, void *refCon)
	void unregisterCallback(const ConstString &key, ConfigCallback cfgc)
*/

private:
	friend class Singleton<ConfigManager>;
	ConfigManager();

	void			loadFile(const String &filename);
	void			writeDomain(FILE *file, const String &name, const Domain &domain);
	
	Domain			_transientDomain;
	DomainMap		_gameDomains;
	DomainMap		_globalDomains;
	Domain			_defaultsDomain;
	
	List<Domain *>	_searchOrder;

	String			_activeDomain;
	String			_filename;
};

}	// End of namespace Common

/** Shortcut for accessing the configuration manager. */
#define ConfMan		Common::ConfigManager::instance()

#endif
