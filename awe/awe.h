/* AWE - Another World Engine
 * Copyright (C) 2004 Gregory Montoir
 * Copyright (C) 2004 The ScummVM project
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
 */

#ifndef AWE_H
#define AWE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "base/engine.h"
#include "base/gameDetector.h"
#include "common/util.h"

namespace Awe {

#define BYPASS_PROTECTION

enum AWEGameId {
	GID_WORLDE
};

class AweEngine : public ::Engine {

	void errorString( const char *buf_input, char *buf_output);

protected:
	void go();
	void shutdown();

	const char *_dataPath;
	const char *_savePath;

public:

	AweEngine(GameDetector *detector, OSystem *syst);
	virtual ~AweEngine();

};

}

#endif
