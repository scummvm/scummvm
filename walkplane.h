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

#ifndef WALKPLANE_H
#define WALKPLANE_H

#include "vector3d.h"
#include "debug.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <string>

class TextSplitter;

class Sector { 
public:
    void load(TextSplitter &ts);
    void load0(TextSplitter &ts, char *name, int id);

    void setVisible(bool visible);

    const char *name() const { return name_.c_str(); }
    const int id() const { return id_; }
    const int type() const { return type_; } // FIXME: Implement type de-masking
    bool visible() const { return visible_; }
    bool isPointInSector(Vector3d point) const;

private:
    int numVertices_, id_;

    std::string name_;
    int type_;
    bool visible_;
    Vector3d *vertices_;
    float height_;
};
#endif
