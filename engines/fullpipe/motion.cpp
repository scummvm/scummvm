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

#include "fullpipe/fullpipe.h"

#include "common/file.h"
#include "common/array.h"
#include "common/list.h"

#include "fullpipe/utils.h"
#include "fullpipe/objects.h"
#include "fullpipe/motion.h"

namespace Fullpipe {

bool CMotionController::load(MfcArchive &file) {
	// Is originally empty	file.readClass();


	return true;
}

bool CMctlCompound::load(MfcArchive &file) {
	int count = file.readUint32LE();

	debug(0, "CMctlCompund::count = %d", count);

	for (int i = 0; i < count; i++) {
	  CMctlCompoundArrayItem *obj = (CMctlCompoundArrayItem *)file.readClass();

	  _motionControllers.push_back(*obj);
	}

	return true;
}

bool CMctlCompoundArray::load(MfcArchive &file) {
	int count = file.readUint32LE();

	debug(0, "CMctlCompundArray::count = %d", count);

	return true;
}

CMovGraph::CMovGraph() {
  _itemsCount = 0;
  _items = 0;
  //_callback1 = CMovGraphCallback1;  // TODO
  _field_44 = 0;
  // insertMessageHandler(CMovGraph_messageHandler, getMessageHandlersCount() - 1, 129);
}

bool CMovGraph::load(MfcArchive &file) {
  _links.load(file);
  _nodes.load(file);

  return true;
}

} // End of namespace Fullpipe
