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

#include "stdafx.h"

#include "common/config-manager.h"

#if defined(UNIX)
#include <sys/param.h>
#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif
#ifdef MACOSX
#define DEFAULT_CONFIG_FILE "Library/Preferences/ScummVM Preferences"
#else
#define DEFAULT_CONFIG_FILE ".scummvmrc"
#endif
#else
#define DEFAULT_CONFIG_FILE "scummvm.ini"
#endif

#define MAXLINELEN 256

static char *ltrim(char *t) {
	while (*t == ' ')
		t++;
	return t;
}

static char *rtrim(char *t) {
	int l = strlen(t) - 1;
	while (l >= 0 && t[l] == ' ')
		t[l--] = 0;
	return t;
}

namespace Common {

const String ConfigManager::kApplicationDomain("scummvm");

const String trueStr("true");
const String falseStr("false");


#pragma mark -


ConfigManager::ConfigManager() {

#if defined(UNIX)
	char configFile[MAXPATHLEN];
	if(getenv("HOME") != NULL)
		sprintf(configFile,"%s/%s", getenv("HOME"), DEFAULT_CONFIG_FILE);
	else strcpy(configFile,DEFAULT_CONFIG_FILE);
#else
	char configFile[256];
	#if defined (WIN32) && !defined(_WIN32_WCE)
		GetWindowsDirectory(configFile, 256);
		strcat(configFile, "\\");
		strcat(configFile, DEFAULT_CONFIG_FILE);
	#elif defined(__PALM_OS__)
		strcpy(configFile,"/PALM/Programs/ScummVM/");
		strcat(configFile, DEFAULT_CONFIG_FILE);
	#else
		strcpy(configFile, DEFAULT_CONFIG_FILE);
	#endif
#endif

	// Ensure the global domain(s) are setup.
	_globalDomains.addKey(kApplicationDomain);
#ifdef _WIN32_WCE
	// WinCE for some reasons uses additional global domains.
	_globalDomains.addKey("wince");
	_globalDomains.addKey("smartfon-keys");
#endif

	_filename = configFile;
	loadFile(_filename);
}

void ConfigManager::loadFile(const String &filename) {
	FILE *cfg_file;
	char buf[MAXLINELEN];
	char *t;
	String domain;

	if (!(cfg_file = fopen(filename.c_str(), "r"))) {
		debug(1, "Unable to open configuration file: %s.\n", filename.c_str());
	} else {
		while (!feof(cfg_file)) {
			t = buf;
			if (!fgets(t, MAXLINELEN, cfg_file))
				continue;
			if (t[0] && t[0] != '#') {
				if (t[0] == '[') {
					// It's a new domain which begins here.
					char *p = strchr(t, ']');
					if (!p) {
						error("Config file buggy: no ] at the end of the domain name.\n");
					} else {
						*p = 0;
						// TODO: Some kind of domain name verification might be nice.
						// E.g. restrict to only a-zA-Z0-9 and maybe -_  or so...
						domain = t + 1;
					}
				} else {
					// Skip leading whitespaces
					while (*t && isspace(*t)) {
						t++;
					}
					// Skip empty lines
					if (*t == 0)
						continue;

					// If no domain has been set, this config file is invalid!
					if (domain.isEmpty()) {
						error("Config file buggy: we have a key without a domain first.\n");
					}

					// It's a new key in the domain.
					char *p = strchr(t, '\n');
					if (p)
						*p = 0;
					p = strchr(t, '\r');
					if (p)
						*p = 0;

					if (!(p = strchr(t, '='))) {
						if (strlen(t))
							warning("Config file buggy: there is junk: %s\n", t);
					} else {
						*p = 0;
						String key = ltrim(rtrim(t));
						String value = ltrim(p + 1);
						set(key, value, domain);
					}
				}
			}
		}
		fclose(cfg_file);
	}
}

void ConfigManager::flushToDisk() {
	FILE *cfg_file;

// TODO
//	if (!willwrite)
//		return;

	if (!(cfg_file = fopen(_filename.c_str(), "w"))) {
		warning("Unable to write configuration file: %s.\n", _filename.c_str());
	} else {
		DomainMap::ConstIterator d;

		// First write the global domains
		for (d = _globalDomains.begin(); d != _globalDomains.end(); ++d) {
			writeDomain(cfg_file, d->_key, d->_value);
		}
		
		// Second, write the game domains
		for (d = _gameDomains.begin(); d != _gameDomains.end(); ++d) {
			writeDomain(cfg_file, d->_key, d->_value);
		}

		fclose(cfg_file);
	}
}

void ConfigManager::writeDomain(FILE *file, const String &name, const Domain &domain) {
	if (domain.isEmpty())
		return;		// Don't bother writing empty domains.
	
	fprintf(file, "[%s]\n", name.c_str());

	Domain::ConstIterator x;
	for (x = domain.begin(); x != domain.end(); ++x) {
		const String &value = x->_value;
		if (!value.isEmpty())
			fprintf(file, "%s=%s\n", x->_key.c_str(), value.c_str());
	}
	fprintf(file, "\n");
}

#pragma mark -


bool ConfigManager::hasKey(const String &key) const {
	// Search the domains in the following order:
	// 1) Run time domain
	// 2) Active game domain (if any)
	// 3) All global domains
	// The defaults domain is explicitly *not* checked.
	
//	if (_transientDomain.contain(key))
//		return true;
	
	if (!_activeDomain.isEmpty() && _gameDomains[_activeDomain].contains(key))
		return true;
	
	DomainMap::ConstIterator iter;
	for (iter = _globalDomains.begin(); iter != _globalDomains.end(); ++iter) {
		if (iter->_value.contains(key))
			return true;
	}
	
	return false;
}

bool ConfigManager::hasKey(const String &key, const String &dom) const {
	assert(!dom.isEmpty());

	if (_gameDomains.contains(dom))
		return _gameDomains[dom].contains(key);
	if (_globalDomains.contains(dom))
		return _globalDomains[dom].contains(key);
	
	return false;
}

void ConfigManager::removeKey(const String &key, const String &dom) {
	assert(!dom.isEmpty());

	if (_gameDomains.contains(dom))
		_gameDomains[dom].remove(key);
	else if (_globalDomains.contains(dom))
		_globalDomains[dom].remove(key);
	else
		error("Removing key '%s' from non-existent domain '%s'", key.c_str(), dom.c_str());
}


#pragma mark -


const String & ConfigManager::get(const String &key) const {
	// Search the domains in the following order:
	// 1) Run time domain
	// 2) Active game domain (if any)
	// 3) All global domains
	// 4) The defaults 

//	if (_transientDomain.contain(key))
//		return true;
	
	if (!_activeDomain.isEmpty() && _gameDomains[_activeDomain].contains(key))
		return _gameDomains[_activeDomain][key];
	
	DomainMap::ConstIterator iter;
	for (iter = _globalDomains.begin(); iter != _globalDomains.end(); ++iter) {
		if (iter->_value.contains(key))
			return iter->_value[key];
	}
	
	return _defaultsDomain.get(key);
}

const String & ConfigManager::get(const String &key, const String &dom) const {
	if (dom.isEmpty())
		return get(key);

	// TODO: How exactly should we handle the case were the domain 'dom'
	// is not found, or were dom is found, but doesn't contain 'key' ?
	// Right now we just return an empty string. But might want to print
	// out a warning, or even error out?
	if (_gameDomains.contains(dom)) {
		// Return the value, if any; defaults to the empty string if the key is
		// not present in the domain. We purposely do not return the registered
		// default value.
		// 
		return _gameDomains[dom].get(key);
	} else if (_globalDomains.contains(dom)) {
		if (_globalDomains[dom].contains(key))
			return _globalDomains[dom].get(key);
		// For global domains, we *do* use the registered default value.
		return _defaultsDomain.get(key);
	} else {
		// Domain was not found. Do *not* return the registered default
		// value, see above for the reasons.
		return String::emptyString;
	}
}

int ConfigManager::getInt(const String &key, const String &dom) const {
	String value(get(key, dom));
	// Convert the string to an integer.
	// TODO: We should perform some error checking.
	if (value.c_str())
		return (int)strtol(value.c_str(), 0, 10);
	else
		return 0;
}

bool ConfigManager::getBool(const String &key, const String &dom) const {
	String value(get(key, dom));
	// '1', 'true' and 'yes' are accepted as true values; everything else
	// maps to value 'false'.
	return (value == trueStr) || (value == "yes") || (value == "1");
}


#pragma mark -


void ConfigManager::set(const String &key, const String &value) {
#if 0
	// TODO ?!?
//	_transientDomain[key] = value;
#else
	if (_activeDomain.isEmpty())
		_globalDomains[kApplicationDomain][key] = value;
	else
		_gameDomains[_activeDomain][key] = value;
#endif
}

void ConfigManager::set(const String &key, const String &value, const String &dom) {
	if (dom.isEmpty()) {
		set(key, value);
		return;
	}

	if (_globalDomains.contains(dom))
		_globalDomains[dom][key] = value;
	else
		_gameDomains[dom][key] = value;
}

void ConfigManager::set(const String &key, const char *value, const String &dom) {
	set(key, String(value), dom);
}

void ConfigManager::set(const String &key, int value, const String &dom) {
	char tmp[128];
	snprintf(tmp, sizeof(tmp), "%i", value);
	set(key, String(tmp), dom);
}

void ConfigManager::set(const String &key, bool value, const String &dom) {
	set(key, value ? trueStr : falseStr, dom);
}


#pragma mark -


void ConfigManager::registerDefault(const String &key, const String &value) {
	_defaultsDomain[key] = value;
}

void ConfigManager::registerDefault(const String &key, const char *value) {
	registerDefault(key, String(value));
}

void ConfigManager::registerDefault(const String &key, int value) {
	char tmp[128];
	snprintf(tmp, sizeof(tmp), "%i", value);
	registerDefault(key, tmp);
}

void ConfigManager::registerDefault(const String &key, bool value) {
	registerDefault(key, value ? trueStr : falseStr);
}


#pragma mark -


void ConfigManager::setActiveDomain(const String &domain) {
	assert(!domain.isEmpty());
	_activeDomain = domain;
	_gameDomains.addKey(domain);
}

void ConfigManager::removeGameDomain(const String &domain) {
	assert(!domain.isEmpty());
	_gameDomains.remove(domain);
}

void ConfigManager::renameGameDomain(const String &oldName, const String &newName) {
	if (oldName == newName)
		return;

	assert(!oldName.isEmpty());
	assert(!newName.isEmpty());

	_gameDomains[newName].merge(_gameDomains[oldName]);
	
	_gameDomains.remove(oldName);
}

bool ConfigManager::hasGameDomain(const String &domain) const {
	assert(!domain.isEmpty());
	return _gameDomains.contains(domain);
}

}	// End of namespace Common
