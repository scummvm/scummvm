/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SAGA_H
#define SAGA_H

#include "common/scummsys.h"
#include "base/engine.h"
#include "base/gameDetector.h"
#include "common/util.h"

#include "gamedesc.h"

namespace Saga {

#define R_PBOUNDS(n,max) (((n)>=(0))&&((n)<(max)))

enum SAGAGameId {
	GID_ITE,
	GID_ITECD,
	GID_IHNM
};

class SagaEngine : public Engine {

	void errorString( const char *buf_input, char *buf_output);

protected:
	void go();
	void shutdown();

public:

	SagaEngine(GameDetector *detector, OSystem *syst);
	virtual ~SagaEngine();

};

} // End of namespace Saga

#endif
