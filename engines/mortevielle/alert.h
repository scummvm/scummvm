/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#ifndef MORTEVIELLE_ALERT_H
#define MORTEVIELLE_ALERT_H

namespace Mortevielle {

int do_alert(Common::String str_, int n);
void decod(Common::String s, int &nbc, int &nbl, int &col, Common::String &c, Common::String &cs);
static void posit(int ji, int &coldep, int &esp);
static void fait_boite(int lidep, int nli, int tx);
static void fait_choix(Common::String c, int &coldep, int &nbcase, array<1, 2, varying_string<3> > &s, int &esp);
int do_alert(Common::String str_, int n);

} // End of namespace Mortevielle
#endif
