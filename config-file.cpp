/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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

#include <stdio.h>
#include <stdlib.h>
#include "stdafx.h"
#include "config-file.h"
#include "scumm.h"

#define MAXLINELEN 256

static char *ltrim(char *t)
{
	for (; *t && (*t == ' '); t++);
	return t;
}

static char *rtrim(char *t)
{
	int l;

	for (l = strlen(t) - 1; l; l--) {
		if (t[l] == ' ') {
			*t = 0;
		} else {
			return t;
		}
	}
	return t;
}

class hashconfig {
public:
	hashconfig(const char *);
	~hashconfig();
	bool is_domain(const char *) const;
	const char *get(const char *) const;
	const char *set(const char *, const char *);
	const char *getdomain() const;
	void flush(FILE *) const;
	void rename(const char *);
	void merge(const hashconfig *);
private:
	char *domain;
	char **keys, **values;
	int nkeys;
};

hashconfig::hashconfig(const char *dom):domain(dom ? Scumm::Strdup(dom) : 0),
keys(0), values(0), nkeys(0)
{
}

hashconfig::~hashconfig()
{
	free(domain);
}

bool hashconfig::is_domain(const char *d) const
{
	return d && domain && !strcmp(d, domain);
}

const char *hashconfig::get(const char *key) const
{
	int i;

	for (i = 0; i < nkeys; i++) {
		if (!strcmp(key, keys[i])) {
			return values[i];
		}
	}

	return 0;
}

const char *hashconfig::set(const char *key, const char *value)
{
	int i;

	for (i = 0; i < nkeys; i++) {
		if (!strcmp(key, keys[i])) {
			free(values[i]);
			return values[i] = value ? Scumm::Strdup(value) : 0;
		}
	}

	nkeys++;
	keys = (char **)realloc(keys, nkeys * sizeof(char *));
	values = (char **)realloc(values, nkeys * sizeof(char *));
	keys[nkeys - 1] = Scumm::Strdup(key);
	return values[nkeys - 1] = value ? Scumm::Strdup(value) : 0;
}

const char *hashconfig::getdomain() const
{
	return domain;
}

void hashconfig::flush(FILE * cfg_file) const
{
	int i;

	if (!domain)
		return;

	fprintf(cfg_file, "[%s]\n", domain);

	for (i = 0; i < nkeys; i++) {
		if (values[i]) {
			fprintf(cfg_file, "%s=%s\n", keys[i], values[i]);
		}
	}
	fprintf(cfg_file, "\n");
}

void hashconfig::rename(const char *d)
{
	free(domain);
	domain = d ? Scumm::Strdup(d) : 0;
}

void hashconfig::merge(const hashconfig * h)
{
	int i;

	for (i = 0; i < h->nkeys; i++) {
		set(h->keys[i], h->values[i]);
	}
}

// The config-class itself.

Config::Config(const char *cfg, const char *d)
 : filename(Scumm::Strdup(cfg)), domain(d ? Scumm::Strdup(d) : 0), hash(0), ndomains(0)
{
	FILE *cfg_file;
	char t[MAXLINELEN];

	if (!(cfg_file = fopen(cfg, "r"))) {
		debug(1, "Unable to open configuration file: %s.\n", filename);
	} else {
		while (!feof(cfg_file)) {
			if (!fgets(t, MAXLINELEN, cfg_file))
				continue;
			if (t[0] != '#') {
				if (t[0] == '[') {
					// It's a new domain which begins here.
					char *p = strchr(t, ']');
					if (!p) {
						debug(1,
									"Config file buggy: no ] at the end of the domain name.\n");
					} else {
						*p = 0;
						set_domain(t + 1);
					}
				} else {
					// It's a new key in the domain.
					if (!domain) {
						debug(1,
									"Config file buggy: we have a key without a domain first.\n");
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

Config::~Config()
{
	int i;

	free(filename);
	free(domain);

	for (i = 0; i < ndomains; i++) {
		delete hash[i];
	}
	free(hash);
}

const char *Config::get(const char *key, const char *d) const
{
	int i;

	if (!d)
		d = domain;

	for (i = 0; i < ndomains; i++) {
		if (hash[i]->is_domain(d)) {
			return hash[i]->get(key);
		}
	}

	return 0;
}

const char *Config::set(const char *key, const char *value, const char *d)
{
	int i;

	if (!d)
		d = domain;

	for (i = 0; i < ndomains; i++) {
		if (hash[i]->is_domain(d)) {
			return hash[i]->set(key, value);
		}
	}

	ndomains++;
	hash =
		(hashconfig **) realloc(hash, ndomains * sizeof(hashconfig *));
	hash[ndomains - 1] = new hashconfig(domain);

	return hash[ndomains - 1]->set(key, value);
}

const char *Config::set(const char *key, int value_i, const char *d)
{
	int i;
	char value[MAXLINELEN];

	sprintf(value, "%i", value_i);

	if (!d)
		d = domain;

	for (i = 0; i < ndomains; i++) {
		if (hash[i]->is_domain(d)) {
			return hash[i]->set(key, value);
		}
	}

	ndomains++;
	hash =
		(hashconfig **) realloc(hash, ndomains * sizeof(hashconfig *));
	hash[ndomains - 1] = new hashconfig(domain);

	return hash[ndomains - 1]->set(key, value);
}

void Config::set_domain(const char *d)
{
	int i;
	free(domain);
	domain = d ? Scumm::Strdup(d) : 0;

	for (i = 0; i < ndomains; i++) {
		if (hash[i]->is_domain(domain))
			return;
	}
	ndomains++;
	hash =
		(hashconfig **) realloc(hash, ndomains * sizeof(hashconfig *));
	hash[ndomains - 1] = new hashconfig(domain);
}

void Config::flush() const
{
	FILE *cfg_file;
	int i;

	if (!(cfg_file = fopen(filename, "w"))) {
		debug(1, "Unable to write configuration file: %s.\n", filename);
	} else {
		for (i = 0; i < ndomains; i++) {
			hash[i]->flush(cfg_file);
		}
		fclose(cfg_file);
	}
}

void Config::rename_domain(const char *d)
{
	int i, index = -1;

	for (i = 0; i < ndomains; i++) {
		if (hash[i]->is_domain(d)) {
			index = i;
		}
	}

	for (i = 0; i < ndomains; i++) {
		if (hash[i]->is_domain(domain)) {
			if (index >= 0) {
				hash[i]->merge(hash[index]);
				hash[index]->rename(0);
				rename_domain(d);
			} else {
				hash[i]->rename(d);
				set_domain(d);
			}
		}
	}
}

void Config::delete_domain(const char *d)
{
	int i;

	for (i = 0; i < ndomains; i++) {
		if (hash[i]->is_domain(d)) {
			hash[i]->rename(0);
		}
	}
}

void Config::change_filename(const char *f)
{
	free(filename);
	filename = f ? Scumm::Strdup(f) : 0;
}

void Config::merge_config(const Config * c)
{
	int i, j;
	bool found;

	for (i = 0; i < c->ndomains; i++) {
		found = false;
		for (j = 0; j < ndomains; j++) {
			if (hash[j]->is_domain(c->hash[i]->getdomain())) {
				hash[j]->merge(c->hash[i]);
				found = true;
				break;
			}
		}
		if (!found) {
			ndomains++;
			hash =
				(hashconfig **) realloc(hash, ndomains * sizeof(hashconfig *));
			hash[ndomains - 1] = new hashconfig(domain);
			hash[ndomains - 1]->merge(c->hash[i]);
		}
	}
}
