/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#ifndef AUTOROUTE_H
#define AUTOROUTE_H

#include "stdafx.h"
#include "sky/sky.h"
#include "struc.h"
#include "compact.h"
#include "grid.h"
#include "skydefs.h"

class SkyGrid;

class SkyAutoRoute {
public:
	SkyAutoRoute(SkyGrid *pGrid);
	~SkyAutoRoute(void);
	uint16 autoRoute(Compact *cpt, uint16 **pSaveRoute);
private:
	uint16 checkBlock(uint16 *blockPos);
	SkyGrid *_grid;
	uint16 *_routeGrid;
};

#endif // AUTOROUTE_H

