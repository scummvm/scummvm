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

#ifndef __CONFIG_FILE_H__
#define __CONFIG_FILE_H__

class hashconfig;

class Config {
  public:
      Config(const char * = "config.cfg", const char * = "default");
      ~Config();
    const char * get(const char *, const char * = 0) const;
    const char * set(const char *, const char *, const char * = 0);
    const char * set(const char *, int, const char * = 0);
    void set_domain(const char *);
    void flush() const;
    void rename_domain(const char *);
    void delete_domain(const char *);
    void change_filename(const char *);
    void merge_config(const Config *);
  private:
    char * filename, * domain;
    hashconfig ** hash;
    int ndomains;
};

// The global config object
extern Config * scummcfg;

#endif
