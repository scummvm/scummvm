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

#include "textsplit.h"
#include "debug.h"
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdarg>


// FIXME: Replace this with a proper parser (this is just too dodgy :)
int residual_vsscanf(const char *str, int field_count, const char *format, va_list ap) {
    unsigned int f01 = va_arg(ap, long);
    unsigned int f02 = va_arg(ap, long);
    unsigned int f03 = va_arg(ap, long);
    unsigned int f04 = va_arg(ap, long);
    unsigned int f05 = va_arg(ap, long);
    unsigned int f06 = va_arg(ap, long);
    unsigned int f07 = va_arg(ap, long);
    unsigned int f08 = va_arg(ap, long);
    unsigned int f09 = va_arg(ap, long);
    unsigned int f10 = va_arg(ap, long);

    if (field_count > 10)
	error("Too many fields requested of residual_vsscanf (%d)", field_count);

    return sscanf(str, format, f01, f02, f03, f04, f05, f06, f07, f08, f09, f10);
}

TextSplitter::TextSplitter(const char *data, int len) {
  data_ = new char[len + 1];
  std::memcpy(data_, data, len);
  data_[len] = '\0';
  curr_line_ = data_;
  processLine();
}

bool TextSplitter::checkString(const char *needle) {
 if (std::strstr(currentLine(), needle))
   return true;
 else
   return false;
}

void TextSplitter::expectString(const char *expected) {
  if (eof())
    error("Expected `%s', got EOF\n", expected);
  if (std::strcmp(currentLine(), expected) != 0)
    error("Expected `%s', got `%s'\n", expected, currentLine());
  nextLine();
}

void TextSplitter::scanString(const char *fmt, int field_count, ...) {
  if (eof())
    error("Expected line of format `%s', got EOF\n", fmt);

  std::va_list va;

  va_start(va, field_count);
  
  #ifdef WIN32
  if (residual_vsscanf(currentLine(), field_count, fmt, va) < field_count)
  #else
  if (std::vsscanf(currentLine(), fmt, va) < field_count)
  #endif
    error("Expected line of format `%s', got `%s'\n", fmt, currentLine());
  va_end(va);

  nextLine();
}

void TextSplitter::processLine() {
  if (eof())
    return;

  next_line_ = std::strchr(curr_line_, '\n');
  if (next_line_ != NULL) {
    *next_line_ = '\0';
    next_line_++;
  }

  // Cut off comments
  char *comment_start = std::strchr(curr_line_, '#');
  if (comment_start != NULL)
    *comment_start = '\0';

  // Cut off trailing whitespace (including '\r')
  char *strend = std::strchr(curr_line_, '\0');
  while (strend > curr_line_ && std::isspace(strend[-1]))
    strend--;
  *strend = '\0';

  // Skip blank lines
  if (*curr_line_ == '\0')
    nextLine();

  // Convert to lower case
  if (! eof())
    for (char *s = curr_line_; *s != '\0'; s++)
      *s = std::tolower(*s);
}
