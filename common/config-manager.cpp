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

#include "common/stdafx.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/util.h"

DECLARE_SINGLETON(Common::ConfigManager);

#if defined(UNIX)
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
	while (isspace(*t))
		t++;
	return t;
}

static char *rtrim(char *t) {
	int l = strlen(t) - 1;
	while (l >= 0 && isspace(t[l]))
		t[l--] = 0;
	return t;
}

static bool isValidDomainName(const Common::String &domain) {
	const char *p = domain.c_str();
	while (*p && (isalnum(*p) || *p == '-' || *p == '_'))
		p++;
	return *p == 0;
}

namespace Common {

#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG))

const String ConfigManager::kApplicationDomain("scummvm");
const String ConfigManager::kTransientDomain("__TRANSIENT");

const String trueStr("true");
const String falseStr("false");

#else

const char *ConfigManager::kApplicationDomain = "scummvm";
const char *ConfigManager::kTransientDomain = "__TRANSIENT";

const char *trueStr = "true";
const char *falseStr = "false";

#endif

#pragma mark -


ConfigManager::ConfigManager() {
	// Ensure the global domain(s) are setup.
	_globalDomains.addKey(kApplicationDomain);
}


void ConfigManager::loadDefaultConfigFile() {
	char configFile[MAXPATHLEN];
#if defined(UNIX)
	const char *home = getenv("HOME");
	if (home != NULL && strlen(home) < MAXPATHLEN)
		snprintf(configFile, MAXPATHLEN, "%s/%s", home, DEFAULT_CONFIG_FILE);
	else
		strcpy(configFile, DEFAULT_CONFIG_FILE);
#else
	#if defined (WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
		GetWindowsDirectory(configFile, MAXPATHLEN);
		strcat(configFile, "\\" DEFAULT_CONFIG_FILE);
	#elif defined(PALMOS_MODE)
		strcpy(configFile,"/PALM/Programs/ScummVM/" DEFAULT_CONFIG_FILE);
	#elif defined(__PLAYSTATION2__)
		strcpy(configFile, "mc0:ScummVM/" DEFAULT_CONFIG_FILE);
	#elif defined(__PSP__)
		strcpy(configFile, "ms0:/" DEFAULT_CONFIG_FILE);
	#elif defined (__SYMBIAN32__)
		strcpy(configFile, SYMBIAN32_DOC_DIR DEFAULT_CONFIG_FILE);
	#else
		strcpy(configFile, DEFAULT_CONFIG_FILE);
	#endif
#endif

	loadConfigFile(configFile);
	flushToDisk();
}

void ConfigManager::loadConfigFile(const String &filename) {
	_globalDomains.clear();
	_gameDomains.clear();
	_transientDomain.clear();

	// Ensure the global domain(s) are setup.
	_globalDomains.addKey(kApplicationDomain);

	_filename = filename;
	_domainSaveOrder.clear();
	loadFile(_filename);
	debug(1, "Switched to configuration %s", _filename.c_str());
}

void ConfigManager::loadFile(const String &filename) {
	File cfg_file;

	if (!cfg_file.open(filename.c_str())) {
		printf("Creating configuration file: %s\n", filename.c_str());
	} else {
		char buf[MAXLINELEN];
		String domain;
		String comment;
		int lineno = 0;

		// TODO: Detect if a domain occurs multiple times (or likewise, if
		// a key occurs multiple times inside one domain).

		while (!cfg_file.eof()) {
			lineno++;
			if (!cfg_file.readLine(buf, MAXLINELEN))
				break;

			if (buf[0] == '#') {
				// Accumulate comments here. Once we encounter either the start
				// of a new domain, or a key-value-pair, we associate the value
				// of the 'comment' variable with that entity.
				comment += buf;
				comment += '\n';
			} else if (buf[0] == '[') {
				// It's a new domain which begins here.
				char *p = buf + 1;
				// Get the domain name, and check whether it's valid (that
				// is, verify that it only consists of alphanumerics,
				// dashes and underscores).
				while (*p && (isalnum(*p) || *p == '-' || *p == '_'))
					p++;

				switch (*p) {
				case '\0':
					error("Config file buggy: missing ] in line %d", lineno);
					break;
				case ']':
					*p = 0;
					domain = buf + 1;
					break;
				default:
					error("Config file buggy: Invalid character '%c' occured in domain name in line %d", *p, lineno);
				}

				// Store domain comment
				if (_globalDomains.contains(domain)) {
					_globalDomains[domain].setDomainComment(comment);
				} else {
					_gameDomains[domain].setDomainComment(comment);
				}
				comment.clear();

				_domainSaveOrder.push_back(domain);
			} else {
				// Skip leading & trailing whitespaces
				char *t = rtrim(ltrim(buf));

				// Skip empty lines
				if (*t == 0)
					continue;

				// If no domain has been set, this config file is invalid!
				if (domain.isEmpty()) {
					error("Config file buggy: Key/value pair found outside a domain in line %d", lineno);
				}

				// Split string at '=' into 'key' and 'value'.
				char *p = strchr(t, '=');
				if (!p)
					error("Config file buggy: Junk found in line line %d: '%s'", lineno, t);
				*p = 0;
				String key = rtrim(t);
				String value = ltrim(p + 1);
				set(key, value, domain);
//printf("key : %s | %s\n");
				// Store comment
				if (_globalDomains.contains(domain)) {
					_globalDomains[domain].setKVComment(key, comment);
				} else {
					_gameDomains[domain].setKVComment(key, comment);
				}
				comment.clear();
			}
		}
	}
}

void ConfigManager::flushToDisk() {
	FILE *cfg_file;

// TODO
//	if (!willwrite)
//		return;

	if (!(cfg_file = fopen(_filename.c_str(), "w"))) {
		warning("Unable to write configuration file: %s", _filename.c_str());
	} else {

		// First write the domains in _domainSaveOrder, in that order.
		// Note: It's possible for _domainSaveOrder to list domains which
		// are not present anymore.
		StringList::const_iterator i;
		for (i = _domainSaveOrder.begin(); i != _domainSaveOrder.end(); ++i) {
			if (_globalDomains.contains(*i)) {
				writeDomain(cfg_file, *i, _globalDomains[*i]);
			} else if (_gameDomains.contains(*i)) {
				writeDomain(cfg_file, *i, _gameDomains[*i]);
			}
		}

		DomainMap::const_iterator d;

		// Now write the global domains which weren't written yet
		for (d = _globalDomains.begin(); d != _globalDomains.end(); ++d) {
			if (!_domainSaveOrder.contains(d->_key))
				writeDomain(cfg_file, d->_key, d->_value);
		}

		// Finally write the remaining game domains
		for (d = _gameDomains.begin(); d != _gameDomains.end(); ++d) {
			if (!_domainSaveOrder.contains(d->_key))
				writeDomain(cfg_file, d->_key, d->_value);
		}

		fclose(cfg_file);
	}
}

void ConfigManager::writeDomain(FILE *file, const String &name, const Domain &domain) {
	if (domain.isEmpty())
		return;		// Don't bother writing empty domains.

	String comment;

	// Write domain comment (if any)
	comment = domain.getDomainComment();
	if (!comment.isEmpty())
		fprintf(file, "%s", comment.c_str());

	// Write domain start
	fprintf(file, "[%s]\n", name.c_str());

	// Write all key/value pairs in this domain, including comments
	Domain::const_iterator x;
	for (x = domain.begin(); x != domain.end(); ++x) {
		const String &value = x->_value;
		if (!value.isEmpty()) {
			// Write comment (if any)
			if (domain.hasKVComment(x->_key)) {
				comment = domain.getKVComment(x->_key);
				fprintf(file, "%s", comment.c_str());
			}
			// Write the key/value pair
			fprintf(file, "%s=%s\n", x->_key.c_str(), value.c_str());
		}
	}
	fprintf(file, "\n");
}

#pragma mark -


bool ConfigManager::hasKey(const String &key) const {
	// Search the domains in the following order:
	// 1) Transient domain
	// 2) Active game domain (if any)
	// 3) All global domains
	// The defaults domain is explicitly *not* checked.

	if (_transientDomain.contains(key))
		return true;

	if (!_activeDomain.isEmpty() && _gameDomains[_activeDomain].contains(key))
		return true;

	DomainMap::const_iterator iter;
	for (iter = _globalDomains.begin(); iter != _globalDomains.end(); ++iter) {
		if (iter->_value.contains(key))
			return true;
	}

	return false;
}

bool ConfigManager::hasKey(const String &key, const String &dom) const {
	assert(!dom.isEmpty());
	assert(isValidDomainName(dom));

	if (dom == kTransientDomain)
		return _transientDomain.contains(key);
	if (_gameDomains.contains(dom))
		return _gameDomains[dom].contains(key);
	if (_globalDomains.contains(dom))
		return _globalDomains[dom].contains(key);

	return false;
}

void ConfigManager::removeKey(const String &key, const String &dom) {
	assert(!dom.isEmpty());
	assert(isValidDomainName(dom));

	if (dom == kTransientDomain)
		_transientDomain.remove(key);
	else if (_gameDomains.contains(dom))
		_gameDomains[dom].remove(key);
	else if (_globalDomains.contains(dom))
		_globalDomains[dom].remove(key);
	else
		error("Removing key '%s' from non-existent domain '%s'", key.c_str(), dom.c_str());
}


#pragma mark -


const String & ConfigManager::get(const String &key, const String &domain) const {
	assert(isValidDomainName(domain));

	// Search the domains in the following order:
	// 1) Transient domain
	// 2) Active game domain (if any)
	// 3) All global domains
	// 4) The defaults

	if ((domain.isEmpty() || domain == kTransientDomain) && _transientDomain.contains(key))
		return _transientDomain[key];

	const String &dom = domain.isEmpty() ? _activeDomain : domain;

	if (!dom.isEmpty() && _gameDomains.contains(dom) && _gameDomains[dom].contains(key))
		return _gameDomains[dom][key];

	DomainMap::const_iterator iter;
	for (iter = _globalDomains.begin(); iter != _globalDomains.end(); ++iter) {
		if (iter->_value.contains(key))
			return iter->_value[key];
	}

	return _defaultsDomain.get(key);
}

int ConfigManager::getInt(const String &key, const String &dom) const {
	String value(get(key, dom));
	char *errpos;

	// For now, be tolerant against missing config keys. Strictly spoken, it is
	// a bug in the calling code to retrieve an int for a key which isn't even
	// present... and a default value of 0 seems rather arbitrary.
	if (value.isEmpty())
		return 0;

	int ivalue = (int)strtol(value.c_str(), &errpos, 10);
	if (value.c_str() == errpos)
		error("Config file buggy: '%s' is not a valid integer", errpos);

	return ivalue;
}

bool ConfigManager::getBool(const String &key, const String &dom) const {
	String value(get(key, dom));

	if ((value == trueStr) || (value == "yes") || (value == "1"))
		return true;
	if ((value == falseStr) || (value == "no") || (value == "0"))
		return false;

	error("Config file buggy: '%s' is not a valid bool", value.c_str());
}


#pragma mark -


void ConfigManager::set(const String &key, const String &value, const String &dom) {
	assert(isValidDomainName(dom));
	if (dom.isEmpty()) {
		// Remove the transient domain value
		_transientDomain.remove(key);

		if (_activeDomain.isEmpty())
			_globalDomains[kApplicationDomain][key] = value;
		else
			_gameDomains[_activeDomain][key] = value;

	} else {

		if (dom == kTransientDomain)
			_transientDomain[key] = value;
		else {
			if (_globalDomains.contains(dom)) {
				_globalDomains[dom][key] = value;
				if (_activeDomain.isEmpty() || !_gameDomains[_activeDomain].contains(key))
					_transientDomain.remove(key);
			} else {
				_gameDomains[dom][key] = value;
				if (dom == _activeDomain)
					_transientDomain.remove(key);
			}
		}
	}
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
	assert(isValidDomainName(domain));
	_activeDomain = domain;
	_gameDomains.addKey(domain);
}

void ConfigManager::removeGameDomain(const String &domain) {
	assert(!domain.isEmpty());
	assert(isValidDomainName(domain));
	_gameDomains.remove(domain);
}

void ConfigManager::renameGameDomain(const String &oldName, const String &newName) {
	if (oldName == newName)
		return;

	assert(!oldName.isEmpty());
	assert(!newName.isEmpty());
	assert(isValidDomainName(oldName));
	assert(isValidDomainName(newName));

	_gameDomains[newName].merge(_gameDomains[oldName]);

	_gameDomains.remove(oldName);
}

bool ConfigManager::hasGameDomain(const String &domain) const {
	assert(!domain.isEmpty());
	return isValidDomainName(domain) && _gameDomains.contains(domain);
}


#pragma mark -


const String &ConfigManager::Domain::get(const String &key) const {
	Node *node = findNode(_root, key);
#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG))
	return node ? node->_value : String::emptyString;
#else
	return node ? node->_value : ConfMan._emptyString;
#endif
}

void ConfigManager::Domain::setDomainComment(const String &comment) {
	_domainComment = comment;
}
const String &ConfigManager::Domain::getDomainComment() const {
	return _domainComment;
}

void ConfigManager::Domain::setKVComment(const String &key, const String &comment) {
	_keyValueComments[key] = comment;
}
const String &ConfigManager::Domain::getKVComment(const String &key) const {
	return _keyValueComments[key];
}
bool ConfigManager::Domain::hasKVComment(const String &key) const {
	return _keyValueComments.contains(key);
}

}	// End of namespace Common
