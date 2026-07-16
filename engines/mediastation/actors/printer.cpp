/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mediastation/actors/printer.h"
#include "mediastation/graphics.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

ScriptValue PrinterActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (methodId) {
	case kIsPrinterReadyMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToBool(g_engine->getPrintManager()->printerIsReady());
		break;

	case kPrintActorsMethod:
		ARGCOUNTCHECK(1);
		if (args[0].getType() == kScriptValueTypeActorId) {
			const uint actorId = args[0].asActorId();
			if (printActor(actorId, true)) {
				g_engine->getPrintManager()->flushToPrinter();
			}
		} else if (args[0].getType() == kScriptValueTypeCollection) {
			const Collection *actorCollection = args[0].asCollection();
			if (actorCollection != nullptr) {
				printActorCollection(*actorCollection);
			}
		}
		break;

	case kPrintScreenMethod:
		ARGCOUNTCHECK(0);
		g_engine->getPrintManager()->printScreen();
		break;

	case kSetPortraitPrintMethod:
		ARGCOUNTCHECK(1);
		if (args[0].asString().equalsIgnoreCase("PORTRAIT")) {
			g_engine->getPrintManager()->_isPortrait = true;
		} else if (args[0].asString().equalsIgnoreCase("LANDSCAPE")) {
			g_engine->getPrintManager()->_isPortrait = false;
		}
		break;

	case kGetPortraitPrintMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToString(g_engine->getPrintManager()->_isPortrait ? "PORTRAIT" : "LANDSCAPE");
		break;

	case kSetLeftPrintMarginMethod:
		ARGCOUNTCHECK(1);
		g_engine->getPrintManager()->_leftMargin = args[0].asFloat();
		break;

	case kSetTopPrintMarginMethod:
		ARGCOUNTCHECK(1);
		g_engine->getPrintManager()->_topMargin = args[0].asFloat();
		break;

	case kSetRightPrintMarginMethod:
		ARGCOUNTCHECK(1);
		g_engine->getPrintManager()->_rightMargin = args[0].asFloat();
		break;

	case kSetBottomPrintMarginMethod:
		ARGCOUNTCHECK(1);
		g_engine->getPrintManager()->_bottomMargin = args[0].asFloat();
		break;

	case kGetLeftPrintMarginMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(g_engine->getPrintManager()->_leftMargin);
		break;

	case kGetTopPrintMarginMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(g_engine->getPrintManager()->_topMargin);
		break;

	case kGetRightPrintMarginMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(g_engine->getPrintManager()->_rightMargin);
		break;

	case kGetBottomPrintMarginMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(g_engine->getPrintManager()->_bottomMargin);
		break;

	default:
		returnValue = SpatialEntity::callMethod(methodId, args);
		break;
	}

	return returnValue;
}

bool PrinterActor::printActor(uint actorId, bool unk1) {
	if (!unk1) {
		return false;
	}

	SpatialEntity *entityToPrint = g_engine->getImtGod()->getSpatialEntityById(actorId);
	if (entityToPrint == nullptr) {
		warning("[%s] %s: Actor %u is not spatial and cannot be printed", debugName(), __func__, actorId);
		return false;
	}

	PrintManager *printManager = g_engine->getPrintManager();
	if (printManager == nullptr) {
		warning("[%s] %s: Print manager has not been initialized", debugName(), __func__);
		return false;
	}

	printManager->printSpatialObject(entityToPrint);
	return true;
}

void PrinterActor::printActorCollection(const Collection &actors) {
	for (const ScriptValue &item : actors) {
		if (item.getType() != kScriptValueTypeActorId) {
			continue;
		}

		const uint actorId = item.asActorId();
		printActor(actorId, true);
	}
}

} // End of namespace MediaStation
