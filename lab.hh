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

#ifndef LAB_HH
#define LAB_HH

#include <string>
#include <cstdio>
#include <stdint.h>
#include "hash_map.hh"

class Block {
public:
  Block(const char *data, int len) :
    data_(data), len_(len) { }
  const char *data() const { return data_; }
  int len() const { return len_; }

  ~Block() { delete[] data_; }

private:
  Block();
  const char *data_;
  int len_;
  bool owner_;
};

class Lab {
public:
  Lab() : f_(NULL) { }
  explicit Lab(const char *filename) : f_(NULL) { open(filename); }
  bool open(const char *filename);
  bool isOpen() const { return f_ != NULL; }
  void close();
  bool fileExists(const char *filename) const;
  Block *getFileBlock(const char *filename) const;
  std::FILE *openNewStream(const char *filename) const;
  int fileLength(const char *filename) const;

  ~Lab() { close(); }

private:
  struct LabEntry {
    LabEntry(int the_offset, int the_len) :
      offset(the_offset), len(the_len) { }
    int offset, len;
  };

  std::FILE *f_;
  typedef std::hash_map<std::string, LabEntry> file_map_type;
  file_map_type file_map_;

  file_map_type::const_iterator find_filename(const char *filename) const;
};

#endif
