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

#ifndef COLOR_H
#define COLOR_H

class Color {
public:
  unsigned char vals_[3];

  Color() { }
  Color(unsigned char r, unsigned char g, unsigned char b) {
    vals_[0] = r; vals_[1] = g; vals_[2] = b;
  }
  Color(const Color& c) {
    vals_[0] = c.vals_[0]; vals_[1] = c.vals_[1]; vals_[2] = c.vals_[2];
  }
  unsigned char &red() { return vals_[0]; }
  unsigned char red() const { return vals_[0]; }
  unsigned char &green() { return vals_[1]; }
  unsigned char green() const { return vals_[1]; }
  unsigned char &blue() { return vals_[2]; }
  unsigned char blue() const { return vals_[2]; }

  Color& operator =(const Color &c) {
    vals_[0] = c.vals_[0]; vals_[1] = c.vals_[1]; vals_[2] = c.vals_[2];
    return *this;
  }

  Color& operator =(Color *c) {
    vals_[0] = c->vals_[0]; vals_[1] = c->vals_[1]; vals_[2] = c->vals_[2];
    return *this;
  }
};

#endif
