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
#include "common/config-file.h"
#include "base/engine.h" // for debug()

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

// The config-class itself.

Config::Config (const String &cfg, const String &d)
 : filename(cfg), defaultDomain(d), willwrite(false) {
	FILE *cfg_file;
	char t[MAXLINELEN];

	if (!(cfg_file = fopen(filename.c_str(), "r"))) {
		debug(1, "Unable to open configuration file: %s.\n", filename.c_str());
	} else {
		while (!feof(cfg_file)) {
			if (!fgets(t, MAXLINELEN, cfg_file))
				continue;
			if (t[0] != '#') {
				if (t[0] == '[') {
					// It's a new domain which begins here.
					char *p = strchr(t, ']');
					if (!p) {
						debug(1, "Config file buggy: no ] at the end of the domain name.\n");
					} else {
						*p = 0;
						set_domain(t + 1);
					}
				} else {
					// It's a new key in the domain.
					if (defaultDomain.isEmpty()) {
						debug(1, "Config file buggy: we have a key without a domain first.\n");
					}
					char *p = strchr(t, '\n');
					if (p)
						*p = 0;
					p = strchr(t, '\r');
					if (p)
						*p = 0;

					if (!(p = strchr(t, '='))) {
						if (strlen(t))
							debug(1, "Config file buggy: there is junk: %s\n", t);
					} else {
						char *key, *value;
						*p = 0;
						key = ltrim(rtrim(t));
						value = ltrim(p + 1);
						set(key, value);
					}
				}
			}
		}
		set_domain(d);
		fclose(cfg_file);
	}
}

const char *Config::get(const String &key, const String &d) const {
	String domain;

	if (d.isEmpty())
		domain = defaultDomain;
	else
		domain = d;

	domain.toLowercase();
	if (domains.contains(domain) && domains[domain].contains(key))
		return domains[domain][key].c_str();

	return 0;
}

const int Config::getInt(const String &key, int def, const String &d) const {
	const char *value = get(key, d);

	if (value)
		return atoi(value);
	return def;
}

const bool Config::getBool(const String &key, bool def, const String &d) const {
	const char *value = get(key, d);

	if (value)
		return !scumm_stricmp(value, "true");
	return def;
}

void Config::set(const String &key, const String &value, const String &d) {
	String domain(d);

	if (domain.isEmpty())
		domain = defaultDomain;

	domain.toLowercase();
	domains[domain][key] = value;
}

void Config::setInt(const String &key, int value_i, const String &d) {
	char value[MAXLINELEN];
	sprintf(value, "%i", value_i);
	set(key, String(value), d);
}

void Config::setBool(const String &key, bool value_b, const String &d) {
	String value(value_b ? "true" : "false");
	set(key, value, d);
}

void Config::set_domain(const String &d) {
	defaultDomain = d;
	defaultDomain.toLowercase();
}

bool Config::has_domain(const String &d) const {
	String temp(d);
	temp.toLowercase();
	return domains.contains(temp);
}

void Config::flush() const {
	FILE *cfg_file;

	if (!willwrite)
		return;

	if (!(cfg_file = fopen(filename.c_str(), "w"))) {
		debug(1, "Unable to write configuration file: %s.\n", filename.c_str());
	} else {
		DomainMap::ConstIterator d;
		for (d = domains.begin(); d != domains.end(); ++d) {
			fprintf(cfg_file, "[%s]\n", d->_key.c_str());

			const StringMap &data = d->_value;
			StringMap::ConstIterator x;
			for (x = data.begin(); x != data.end(); ++x) {
				const String &value = x->_value;
				if (!value.isEmpty())
					fprintf(cfg_file, "%s=%s\n", x->_key.c_str(), value.c_str());
			}
			fprintf(cfg_file, "\n");
		}
		fclose(cfg_file);
	}
}

void Config::rename_domain(const String &oldD, const String &newD) {
	String oldDomain(oldD);
	String newDomain(newD);
	oldDomain.toLowercase();
	newDomain.toLowercase();

	if (oldDomain == newDomain)
		return;

	StringMap &oldHash = domains[oldDomain];
	StringMap &newHash = domains[newDomain];

	newHash.merge(oldHash);
	
	domains.remove(oldDomain);
}

void Config::delete_domain(const String &d) {
	String domain(d);
	domain.toLowercase();
	domains.remove(d);
}

void Config::set_filename(const String &f) {
	filename = f;
}

void Config::merge_config(const Config &c) {
	DomainMap::ConstIterator d, end(c.domains.end());
	for (d = c.domains.begin(); d != end; ++d) {
		domains[d->_key].merge(d->_value);
	}
}

void Config::set_writing(bool w) {
	willwrite = w;
}

const int Config::count_domains() {
	int count = 0;
	DomainMap::ConstIterator d, end(domains.end());
	for (d = domains.begin(); d != end; ++d)
		count++;

	return count;
}

Common::StringList Config::get_domains() {
	StringList domainNames;
	DomainMap::ConstIterator d, end(domains.end());
	for (d = domains.begin(); d != end; ++d) {
		domainNames.push_back(d->_key);
	}

	return domainNames;
}

