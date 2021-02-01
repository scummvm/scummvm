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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/3d/3dinc.h"

/*------------------------------------------------
 Useful Global Variables:

 _panel        s : Structure containing the panels
 _panelNum     i : Number of panels
 _curPanel     i : current Panel
 _oldPanel     i : starting Panel
 _curX         f : X position of the affected Panel
 _curZ         f : Z position of the affected Panel
 _cam          s : Camera
 _proj         f : camera projection matrix
 _invP         f : camera antiprojection matrix
 _x3d, _y3d, _z3d  f : Float variables for the ouput functions
 _x2d, _y2d    i : Int variables for the ouput functions
 _cx, _cy      i : Center of the 2D screen
 _actor        s : Actor structure
 _step         s : Character frame struture
 _pathNode     s : Path nodes
 _numPathNodes i : Number of nodes
 _curStep      i : Character current frame
 _lastStep     i : character last frame
 _defActionLen u : Default character action length
--------------------------------------------------*/

namespace Trecision {

struct SPan      _panel[MAXPANELSINROOM];
struct SSortPan  _sortPan[32];
struct SPathNode _pathNode[MAXPATHNODES];

struct SStep     _step[MAXSTEP];
struct SActor    _actor;

float  _proj[3][3];
float  _invP[3][3];

int    _panelNum;
int    _actorPos;
int    _forcedActorPos;

int    _cx, _cy;
int    _x2d, _y2d;

float  _x3d, _y3d, _z3d;

int    _curPanel = - 1, _oldPanel = - 1;
float  _curX, _curZ;

int    _numPathNodes;
int    _numSortPan;
int    _curStep;
int    _lastStep;

uint8  _defActionLen[hLAST + 1] = {
	/* STAND */		1,
	/* PARTE */		1,
	/* WALK  */		10,
	/* END   */		1,
	/* STOP0 */		3,
	/* STOP1 */		4,
	/* STOP2 */		3,
	/* STOP3 */		2,
	/* STOP4 */		3,
	/* STOP5 */		4,
	/* STOP6 */		3,
	/* STOP7 */		3,
	/* STOP8 */		2,
	/* STOP9 */		3,
	/* WALKI */		12,
	/* BOH   */		9,
	/* UGG   */		41,
	/* UTT   */		35,
	/* WALKO */		12,
	/* LAST  */		15
};

LLBOOL    _shadowSplit;

} // End of namespace Trecision
