// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "registry.h"
#include "debug.h"
#include <cstdlib>

Registry::Registry() : dirty_(false) {
  #ifdef WIN32
    std::string filename = "C:\\residual.ini";
  #else
    std::string filename = std::string(std::getenv("HOME")) + "/.residualrc";
  #endif

  std::FILE *f = fopen(filename.c_str(), "r");
  if (f != NULL) {
    char line[1024];
    while (!feof(f) && fgets(line, sizeof(line), f) != NULL) {
      char *equals = std::strchr(line, '=');
      char *newline = std::strchr(line, '\n');
      if (newline != NULL)
	*newline = '\0';
      if (equals != NULL) {
	std::string key = std::string(line, equals - line);
	std::string val = std::string(equals + 1);
	settings_[key] = val;
      }
    }
    std::fclose(f);
  }
}

Registry *Registry::instance_ = NULL;

Registry *Registry::instance() {
  if (instance_ == NULL)
    instance_ = new Registry;
  return instance_;
}

const char *Registry::get(const char *key) const {
  group::const_iterator i = settings_.find(key);
  if (i == settings_.end())
    return NULL;
  else
    return i->second.c_str();
}

void Registry::set(const char *key, const char *val) {
  // Hack: Don't save these, so we can run in good_times mode
  // without new games being bogus.
  if (strstr(key, "GrimLastSet") || strstr(key, "GrimMannyState"))
   return;

  settings_[key] = val;
  dirty_ = true;
}

void Registry::save() {
  if (! dirty_)
    return;

  #ifdef WIN32
    std::string filename = "C:\residual.ini";
  #else
    std::string filename = std::string(std::getenv("HOME")) + "/.residualrc";
  #endif

  std::FILE *f = std::fopen(filename.c_str(), "w");
  if (f == NULL) {
    warning("Could not open registry file %s for writing\n",
	    filename.c_str());
    return;
  }

  for (group::iterator i = settings_.begin(); i != settings_.end(); i++)
    std::fprintf(f, "%s=%s\n", i->first.c_str(), i->second.c_str());
  std::fclose(f);
  dirty_ = false;
}
