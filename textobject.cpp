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

#include "stdafx.h"
#include "textobject.h"
#include "engine.h"
#include "localize.h"

TextObject::TextObject(const char *text, const int x, const int y, const Color& fgColor) :
                       fgColor_(fgColor), x_(x), y_(y) {
  strcpy(textID_, text);
  Engine::instance()->registerTextObject(this);
}

void TextObject::setX(int x) {x_ = x;}
void TextObject::setY(int y) {y_ = y;}
void TextObject::setColor(Color *newcolor) {fgColor_ = newcolor;}

void TextObject::draw() {
  const char *localString = Localizer::instance()->localize(textID_).c_str();
  //warning("Drawing text object %s at (%d,%d): %s", textID_, x_, y_, localString);

  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, 640, 480, 0, 0, 1);
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glColor3f(fgColor_.red(), fgColor_.green(), fgColor_.blue());
  glRasterPos2i(x_, y_);
  glListBase(Engine::instance()->font);
  glCallLists(
    strlen(strrchr(localString, '/')) - 1, 
    GL_UNSIGNED_BYTE,
    strrchr(localString, '/') + 1
  );

  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
}

