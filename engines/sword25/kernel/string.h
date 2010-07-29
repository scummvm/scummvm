// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef BS_STRING
#define BS_STRING

#include "memlog_off.h"
#include <string>
#include "memlog_on.h"

namespace BS_String
{
  inline unsigned int GetHash(const std::string & Str)
  {
    unsigned int Result = 0;

    for (unsigned int i = 0; i < Str.size(); i++)
      Result = ((Result << 5) - Result) + Str[i];

    return Result;
  }

  inline bool ToInt(const std::string & Str, int & Result)
  {
    std::string::const_iterator Iter = Str.begin();

    // Whitespace überspringen
    while (*Iter && (*Iter == ' ' || *Iter == '\t')) { ++Iter; }
    if (Iter == Str.end()) return false;

    // Vorzeichen auslesen, wenn vorhanden
    bool IsNegative = false;
    if (*Iter == '-')
    {
      IsNegative = true;
      ++Iter;
    }
    else if (*Iter == '+')
      ++Iter;

    // Whitespace überspringen
    while (*Iter && (*Iter == ' ' || *Iter == '\t')) { ++Iter; }
    if (Iter ==Str.end()) return false;

    // String in Ganzzahl umwandeln
    Result = 0;
    while (Iter != Str.end())
    {
      if (*Iter < '0' || *Iter > '9')
      {
        while (*Iter && (*Iter == ' ' || *Iter == '\t')) { ++Iter; }
        if (Iter != Str.end()) return false;
        break;
      }
      Result = (Result * 10) + (*Iter - '0');
      ++Iter;
    }

    if (IsNegative) Result = -Result;

    return true;
  }

  inline bool ToBool(const std::string & Str, bool & Result)
  {
    if (Str == "true" ||
      Str == "TRUE")
    {
      Result = true;
      return true;
    }
    else if (Str == "false" ||
      Str == "FALSE")
    {
      Result = false;
      return true;
    }

    return false;
  }

  inline void ToLower(std::string & Str)
  {
    static const unsigned char LowerCaseMap[256] =
    {
      0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
      32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
      64,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,91,92,93,94,95,
      96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
      128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
      160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
      192,193,194,195,228,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,246,215,216,217,218,219,252,221,222,223,
      224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
    };

    for (unsigned int i = 0; i < Str.size(); ++i)
      Str[i] = LowerCaseMap[Str[i]];
  }
}

#endif
