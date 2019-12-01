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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima8/misc/pent_include.h"
#include "ultima8/gumps/gump_notify_process.h"
#include "ultima8/gumps/gump.h"
#include "ultima8/world/get_object.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(GumpNotifyProcess, Process);

GumpNotifyProcess::GumpNotifyProcess()
	: Process() {

}

GumpNotifyProcess::GumpNotifyProcess(uint16 it) : Process(it), gump(0) {
	result = 0;
	type = 0x200; // CONSTANT!
}

GumpNotifyProcess::~GumpNotifyProcess(void) {
}

void GumpNotifyProcess::setGump(Gump *g) {
	gump = g->getObjId();
}


void GumpNotifyProcess::notifyClosing(int res) {
	gump = 0;
	result = res;
	if (!(flags & PROC_TERMINATED)) terminate();
}

void GumpNotifyProcess::terminate() {
	Process::terminate();

	if (gump) {
		Gump *g = ::getGump(gump);
		assert(g);
		g->Close();
	}
}

void GumpNotifyProcess::run() {
}

void GumpNotifyProcess::dumpInfo() {
	Process::dumpInfo();
	pout << " gump: " << gump << std::endl;
}

void GumpNotifyProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write2(gump);
}

bool GumpNotifyProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	gump = ids->read2();

	return true;
}

// Colourless Protection
