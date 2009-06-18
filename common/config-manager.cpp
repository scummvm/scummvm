/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/util.h"
#include "common/system.h"

DECLARE_SINGLETON(Common::ConfigManager);

static bool isValidDomainName(const Common::String &domName) {
	const char *p = domName.c_str();
	while (*p && (isalnum(*p) || *p == '-' || *p == '_'))
		p++;
	return *p == 0;
}

namespace Common {

#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))

const String ConfigManager::kApplicationDomain("residual");
const String ConfigManager::kTransientDomain("__TRANSIENT");

#ifdef ENABLE_KEYMAPPER
const String ConfigManager::kKeymapperDomain("keymapper");
#endif

#else

const char *ConfigManager::kApplicationDomain = "residual";
const char *ConfigManager::kTransientDomain = "__TRANSIENT";

#ifdef ENABLE_KEYMAPPER
const char *ConfigManager::kKeymapperDomain = "keymapper";
#endif

#endif

#pragma mark -


ConfigManager::ConfigManager()
 : _activeDomain(0) {
}


void ConfigManager::loadDefaultConfigFile() {
	// Open the default config file
	assert(g_system);
	SeekableReadStream *stream = g_system->createConfigReadStream();
	_filename.clear();	// clear the filename to indicate that we are using the default config file

	// ... load it, if available ...
	if (stream)
		loadFromStream(*stream);

	// ... and close it again.
	delete stream;

	flushToDisk();
}

void ConfigManager::loadConfigFile(const String &filename) {
	_filename = filename;

	FSNode node(filename);
	File cfg_file;
	if (!cfg_file.open(node)) {
		printf("Creating configuration file: %s\n", filename.c_str());
	} else {
		printf("Using configuration file: %s\n", _filename.c_str());
		loadFromStream(cfg_file);
	}
}

void ConfigManager::loadFromStream(SeekableReadStream &stream) {
	String domain;
	String comment;
	int lineno = 0;

	_appDomain.clear();
	_gameDomains.clear();
	_transientDomain.clear();
	_domainSaveOrder.clear();

#ifdef ENABLE_KEYMAPPER
	_keymapperDomain.clear();
#endif

	// TODO: Detect if a domain occurs multiple times (or likewise, if
	// a key occurs multiple times inside one domain).

	while (!stream.eos() && !stream.err()) {
		lineno++;

		// Read a line
		String line = stream.readLine();

		if (line.size() == 0) {
			// Do nothing
		} else if (line[0] == '#') {
			// Accumulate comments here. Once we encounter either the start
			// of a new domain, or a key-value-pair, we associate the value
			// of the 'comment' variable with that entity.
			comment += line;
#ifdef _WIN32
			comment += "\r\n";
#else
			comment += "\n";
#endif
		} else if (line[0] == '[') {
			// It's a new domain which begins here.
			const char *p = line.c_str() + 1;
			// Get the domain name, and check whether it's valid (that
			// is, verify that it only consists of alphanumerics,
			// dashes and underscores).
			while (*p && (isalnum(*p) || *p == '-' || *p == '_'))
				p++;

			if (*p == '\0')
				error("Config file buggy: missing ] in line %d", lineno);
			else if (*p != ']')
				error("Config file buggy: Invalid character '%c' occurred in section name in line %d", *p, lineno);

			domain = String(line.c_str() + 1, p);

			// Store domain comment
			if (domain == kApplicationDomain) {
				_appDomain.setDomainComment(comment);
#ifdef ENABLE_KEYMAPPER
			} else if (domain == kKeymapperDomain) {
				_keymapperDomain.setDomainComment(comment);
#endif
			} else {
				_gameDomains[domain].setDomainComment(comment);
			}
			comment.clear();

			_domainSaveOrder.push_back(domain);
		} else {
			// This line should be a line with a 'key=value' pair, or an empty one.

			// Skip leading whitespaces
			const char *t = line.c_str();
			while (isspace(*t))
				t++;

			// Skip empty lines / lines with only whitespace
			if (*t == 0)
				continue;

			// If no domain has been set, this config file is invalid!
			if (domain.empty()) {
				error("Config file buggy: Key/value pair found outside a domain in line %d", lineno);
			}

			// Split string at '=' into 'key' and 'value'. First, find the "=" delimeter.
			const char *p = strchr(t, '=');
			if (!p)
				error("Config file buggy: Junk found in line line %d: '%s'", lineno, t);

			// Extract the key/value pair
			String key(t, p);
			String value(p + 1);

			// Trim of spaces
			key.trim();
			value.trim();

			// Finally, store the key/value pair in the active domain
			set(key, value, domain);

			// Store comment
			if (domain == kApplicationDomain) {
				_appDomain.setKVComment(key, comment);
#ifdef ENABLE_KEYMAPPER
			} else if (domain == kKeymapperDomain) {
				_keymapperDomain.setKVComment(key, comment);
#endif
			} else {
				_gameDomains[domain].setKVComment(key, comment);
			}
			comment.clear();
		}
	}
}

void ConfigManager::flushToDisk() {
#ifndef __DC__
	WriteStream *stream;

	if (_filename.empty()) {
		// Write to the default config file
		assert(g_system);
		stream = g_system->createConfigWriteStream();
		if (!stream)	// If writing to the config file is not possible, do nothing
			return;
	} else {
		DumpFile *dump = new DumpFile();
		assert(dump);

		if (!dump->open(_filename)) {
			warning("Unable to write configuration file: %s", _filename.c_str());
			delete dump;
			return;
		}

		stream = dump;
	}

	// First write the domains in _domainSaveOrder, in that order.
	// Note: It's possible for _domainSaveOrder to list domains which
	// are not present anymore.
	StringList::const_iterator i;
	for (i = _domainSaveOrder.begin(); i != _domainSaveOrder.end(); ++i) {
		if (kApplicationDomain == *i) {
			writeDomain(*stream, *i, _appDomain);
#ifdef ENABLE_KEYMAPPER
		} else if (kKeymapperDomain == *i) {
			writeDomain(*stream, *i, _keymapperDomain);
#endif
		} else if (_gameDomains.contains(*i)) {
			writeDomain(*stream, *i, _gameDomains[*i]);
		}
	}

	DomainMap::const_iterator d;


	// Now write the domains which haven't been written yet
	if (find(_domainSaveOrder.begin(), _domainSaveOrder.end(), kApplicationDomain) == _domainSaveOrder.end())
		writeDomain(*stream, kApplicationDomain, _appDomain);
#ifdef ENABLE_KEYMAPPER
	if (find(_domainSaveOrder.begin(), _domainSaveOrder.end(), kKeymapperDomain) == _domainSaveOrder.end())
		writeDomain(*stream, kKeymapperDomain, _keymapperDomain);
#endif
	for (d = _gameDomains.begin(); d != _gameDomains.end(); ++d) {
		if (find(_domainSaveOrder.begin(), _domainSaveOrder.end(), d->_key) == _domainSaveOrder.end())
			writeDomain(*stream, d->_key, d->_value);
	}

	delete stream;

#endif // !__DC__
}

void ConfigManager::writeDomain(WriteStream &stream, const String &name, const Domain &domain) {
	if (domain.empty())
		return;		// Don't bother writing empty domains.

	// WORKAROUND: Fix for bug #1972625 "ALL: On-the-fly targets are
	// written to the config file": Do not save domains that came from
	// the command line
	if (domain.contains("id_came_from_command_line"))
		return;

	String comment;

	// Write domain comment (if any)
	comment = domain.getDomainComment();
	if (!comment.empty())
		stream.writeString(comment);

	// Write domain start
	stream.writeByte('[');
	stream.writeString(name);
	stream.writeByte(']');
#ifdef _WIN32
	stream.writeByte('\r');
	stream.writeByte('\n');
#else
	stream.writeByte('\n');
#endif

	// Write all key/value pairs in this domain, including comments
	Domain::const_iterator x;
	for (x = domain.begin(); x != domain.end(); ++x) {
		if (!x->_value.empty()) {
			// Write comment (if any)
			if (domain.hasKVComment(x->_key)) {
				comment = domain.getKVComment(x->_key);
				stream.writeString(comment);
			}
			// Write the key/value pair
			stream.writeString(x->_key);
			stream.writeByte('=');
			stream.writeString(x->_value);
#ifdef _WIN32
			stream.writeByte('\r');
			stream.writeByte('\n');
#else
			stream.writeByte('\n');
#endif
		}
	}
#ifdef _WIN32
	stream.writeByte('\r');
	stream.writeByte('\n');
#else
	stream.writeByte('\n');
#endif
}


#pragma mark -


const ConfigManager::Domain *ConfigManager::getDomain(const String &domName) const {
	assert(!domName.empty());
	assert(isValidDomainName(domName));

	if (domName == kTransientDomain)
		return &_transientDomain;
	if (domName == kApplicationDomain)
		return &_appDomain;
#ifdef ENABLE_KEYMAPPER
	if (domName == kKeymapperDomain)
		return &_keymapperDomain;
#endif
	if (_gameDomains.contains(domName))
		return &_gameDomains[domName];

	return 0;
}

ConfigManager::Domain *ConfigManager::getDomain(const String &domName) {
	assert(!domName.empty());
	assert(isValidDomainName(domName));

	if (domName == kTransientDomain)
		return &_transientDomain;
	if (domName == kApplicationDomain)
		return &_appDomain;
#ifdef ENABLE_KEYMAPPER
	if (domName == kKeymapperDomain)
		return &_keymapperDomain;
#endif
	if (_gameDomains.contains(domName))
		return &_gameDomains[domName];

	return 0;
}


#pragma mark -


bool ConfigManager::hasKey(const String &key) const {
	// Search the domains in the following order:
	// 1) the transient domain,
	// 2) the active game domain (if any),
	// 3) the application domain.
	// The defaults domain is explicitly *not* checked.

	if (_transientDomain.contains(key))
		return true;

	if (_activeDomain && _activeDomain->contains(key))
		return true;

	if (_appDomain.contains(key))
		return true;

	return false;
}

bool ConfigManager::hasKey(const String &key, const String &domName) const {
	// FIXME: For now we continue to allow empty domName to indicate
	// "use 'default' domain". This is mainly needed for the SCUMM ConfigDialog
	// and should be removed ASAP.
	if (domName.empty())
		return hasKey(key);

	const Domain *domain = getDomain(domName);

	if (!domain)
		return false;
	return domain->contains(key);
}

void ConfigManager::removeKey(const String &key, const String &domName) {
	Domain *domain = getDomain(domName);

	if (!domain)
		error("ConfigManager::removeKey(%s, %s) called on non-existent domain",
					key.c_str(), domName.c_str());

	domain->erase(key);
}


#pragma mark -


const String & ConfigManager::get(const String &key) const {
	if (_transientDomain.contains(key))
		return _transientDomain[key];
	else if (_activeDomain && _activeDomain->contains(key))
		return (*_activeDomain)[key];
	else if (_appDomain.contains(key))
		return _appDomain[key];
	else if (_defaultsDomain.contains(key))
		return _defaultsDomain[key];

#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
	return String::emptyString;
#else
	return ConfMan._emptyString;
#endif
}

const String & ConfigManager::get(const String &key, const String &domName) const {
	// FIXME: For now we continue to allow empty domName to indicate
	// "use 'default' domain". This is mainly needed for the SCUMM ConfigDialog
	// and should be removed ASAP.
	if (domName.empty())
		return get(key);

	const Domain *domain = getDomain(domName);

	if (!domain)
		error("ConfigManager::get(%s,%s) called on non-existent domain",
								key.c_str(), domName.c_str());

	if (domain->contains(key))
		return (*domain)[key];

	return _defaultsDomain.get(key);

	if (!domain->contains(key)) {
#if 1
#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
	return String::emptyString;
#else
	return ConfMan._emptyString;
#endif
#else
		error("ConfigManager::get(%s,%s) called on non-existent key",
					key.c_str(), domName.c_str());
#endif
	}

	return (*domain)[key];
}

int ConfigManager::getInt(const String &key, const String &domName) const {
	String value(get(key, domName));
	char *errpos;

	// For now, be tolerant against missing config keys. Strictly spoken, it is
	// a bug in the calling code to retrieve an int for a key which isn't even
	// present... and a default value of 0 seems rather arbitrary.
	if (value.empty())
		return 0;

	// We use the special value '0' for the base passed to strtol. Doing that
	// makes it possible to enter hex values as "0x1234", but also decimal
	// values ("123") are still valid.
	int ivalue = (int)strtol(value.c_str(), &errpos, 0);
	if (value.c_str() == errpos)
		error("ConfigManager::getInt(%s,%s): '%s' is not a valid integer",
					key.c_str(), domName.c_str(), errpos);

	return ivalue;
}

bool ConfigManager::getBool(const String &key, const String &domName) const {
	String value(get(key, domName));

	if ((value == "true") || (value == "yes") || (value == "1"))
		return true;
	if ((value == "false") || (value == "no") || (value == "0"))
		return false;

	error("ConfigManager::getBool(%s,%s): '%s' is not a valid bool",
				key.c_str(), domName.c_str(), value.c_str());
	return false;
}


#pragma mark -


void ConfigManager::set(const String &key, const String &value) {
	// Remove the transient domain value, if any.
	_transientDomain.erase(key);

	// Write the new key/value pair into the active domain, resp. into
	// the application domain if no game domain is active.
	if (_activeDomain)
		(*_activeDomain)[key] = value;
	else
		_appDomain[key] = value;
}

void ConfigManager::set(const String &key, const String &value, const String &domName) {
	// FIXME: For now we continue to allow empty domName to indicate
	// "use 'default' domain". This is mainly needed for the SCUMM ConfigDialog
	// and should be removed ASAP.
	if (domName.empty()) {
		set(key, value);
		return;
	}

	Domain *domain = getDomain(domName);

	if (!domain)
		error("ConfigManager::set(%s,%s,%s) called on non-existent domain",
					key.c_str(), value.c_str(), domName.c_str());

	(*domain)[key] = value;

	// TODO/FIXME: We used to erase the given key from the transient domain
	// here. Do we still want to do that?
	// It was probably there to simplify the options dialogs code:
	// Imagine you are editing the current options (via the SCUMM ConfigDialog,
	// for example). If you edit the game domain for that, but a matching
	// entry in the transient domain is present, than your changes may not take
	// effect. So you want to remove the key from the transient domain before
	// adding it to the active domain.
	// But doing this here seems rather evil... need to comb the options dialog
	// code to find out if it's still necessary, and if that's the case, how
	// to replace it in a clean fashion...
/*
	if (domName == kTransientDomain)
		_transientDomain[key] = value;
	else {
		if (domName == kApplicationDomain) {
			_appDomain[key] = value;
			if (_activeDomainName.empty() || !_gameDomains[_activeDomainName].contains(key))
				_transientDomain.erase(key);
		} else {
			_gameDomains[domName][key] = value;
			if (domName == _activeDomainName)
				_transientDomain.erase(key);
		}
	}
*/
}

void ConfigManager::setInt(const String &key, int value, const String &domName) {
	char tmp[128];
	snprintf(tmp, sizeof(tmp), "%i", value);
	set(key, String(tmp), domName);
}

void ConfigManager::setBool(const String &key, bool value, const String &domName) {
	set(key, String(value ? "true" : "false"), domName);
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
	registerDefault(key, value ? "true" : "false");
}


#pragma mark -


void ConfigManager::setActiveDomain(const String &domName) {
	if (domName.empty()) {
		_activeDomain = 0;
	} else {
		assert(isValidDomainName(domName));
		_activeDomain = & _gameDomains[domName];
	}
	_activeDomainName = domName;
}

void ConfigManager::addGameDomain(const String &domName) {
	assert(!domName.empty());
	assert(isValidDomainName(domName));

	// TODO: Do we want to generate an error/warning if a domain with
	// the given name already exists?

	_gameDomains[domName];

	// Add it to the _domainSaveOrder, if it's not already in there
	if (find(_domainSaveOrder.begin(), _domainSaveOrder.end(), domName) == _domainSaveOrder.end())
		_domainSaveOrder.push_back(domName);
}

void ConfigManager::removeGameDomain(const String &domName) {
	assert(!domName.empty());
	assert(isValidDomainName(domName));
	_gameDomains.erase(domName);
}

void ConfigManager::renameGameDomain(const String &oldName, const String &newName) {
	if (oldName == newName)
		return;

	assert(!oldName.empty());
	assert(!newName.empty());
	assert(isValidDomainName(oldName));
	assert(isValidDomainName(newName));

//	_gameDomains[newName].merge(_gameDomains[oldName]);
	Domain &oldDom = _gameDomains[oldName];
	Domain &newDom = _gameDomains[newName];
	Domain::const_iterator iter;
	for (iter = oldDom.begin(); iter != oldDom.end(); ++iter)
		newDom[iter->_key] = iter->_value;

	_gameDomains.erase(oldName);
}

bool ConfigManager::hasGameDomain(const String &domName) const {
	assert(!domName.empty());
	return isValidDomainName(domName) && _gameDomains.contains(domName);
}


#pragma mark -


const String &ConfigManager::Domain::get(const String &key) const {
	const_iterator iter(find(key));
	if (iter != end())
		return iter->_value;

#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
	return String::emptyString;
#else
	return ConfMan._emptyString;
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
