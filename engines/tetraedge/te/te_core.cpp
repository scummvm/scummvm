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

#include "common/file.h"
#include "common/fs.h"
#include "common/debug.h"
#include "common/config-manager.h"
#include "common/language.h"
#include "common/tokenizer.h"

#include "tetraedge/te/te_core.h"

#include "tetraedge/tetraedge.h"

#include "tetraedge/te/te_png.h"
#include "tetraedge/te/te_images_sequence.h"
#include "tetraedge/te/te_jpeg.h"
#include "tetraedge/te/te_zlib_jpeg.h"
#include "tetraedge/te/te_theora.h"
#include "tetraedge/te/te_tga.h"

namespace Tetraedge {

TeCore::TeCore() : _loc(nullptr), _coreNotReady(true), _resourcesRoot("") {
	create();
}

void TeCore::addLoc(TeILoc *loc) {
	if (_loc) {
		warning("TeCore::addLoc: There is already a loc");
	}
	_loc = loc;
}

void TeCore::create() {
	const char *langCode = getLanguageCode(g_engine->getGameLanguage());
	const Common::String confLang = ConfMan.get("language");
	Common::String useLang = "en";
	if (langCode)
		useLang = langCode;
	if (!confLang.empty())
		useLang = confLang;
	language(useLang);
	_coreNotReady = false;
	_activityTrackingTimer.alarmSignal().add(this, &TeCore::onActivityTrackingAlarm);
	warning("TODO: TeCore::create: Finish implementing me.");

	const Common::FSNode gameRoot(ConfMan.getPath("path"));
	if (!gameRoot.isDirectory())
		error("Game directory should be a directory");
	const Common::FSNode resNode = (g_engine->getGamePlatform() == Common::kPlatformMacintosh
										? gameRoot.getChild("Resources")
										: gameRoot);
	if (!resNode.isDirectory())
		error("Resources directory should exist in game");

	_resourcesRoot = Common::FSDirectory(resNode, 5, false, false, true);
}

TeICodec *TeCore::createVideoCodec(const Common::String &extn) {
	// The original engine has more formats and even checks for alpha maps,
	// but it never uses them.
	if (TePng::matchExtension(extn)) {
		// png codec needs to know extension
		return new TePng(extn);
	} else if (TeJpeg::matchExtension(extn)) {
		return new TeJpeg();
	} else if (TeZlibJpeg::matchExtension(extn)) {
		return new TeZlibJpeg();
	} else if (TeTheora::matchExtension(extn)) {
		return new TeTheora();
	} else if (TeTga::matchExtension(extn)) {
		return new TeTga();
	} else if (TeImagesSequence::matchExtension(extn)) {
		return new TeImagesSequence();
	}
	return nullptr;
}

TeICodec *TeCore::createVideoCodec(const TetraedgeFSNode &node) {
	const Common::String filename = node.getPath().baseName();
	if (!filename.contains('.'))
		return nullptr;
	Common::String extn = filename.substr(filename.findLastOf('.') + 1);
	extn.toLowercase();
	TeICodec *codec = createVideoCodec(extn);
	if (!codec)
		error("TTeCore::createVideoCodec: Unrecognised format %s", filename.c_str());
	return codec;
}

const Common::String &TeCore::fileFlagSystemFlag(const Common::String &name) const {
	return _fileSystemFlags.find(name)->_value;
}

void TeCore::fileFlagSystemSetFlag(const Common::String &name, const Common::String &val) {
	// TODO: Impmenent this fully to check possible values
	_fileSystemFlags.setVal(name, val);
}

bool TeCore::fileFlagSystemFlagsContains(const Common::String &name) const {
	error("TODO: Implement TeCore::fileFlagSystemFlagsContains");
	return false;
}

Common::Array<Common::String> TeCore::fileFlagSystemPossibleFlags() {
	error("TODO: Implement TeCore::fileFlagSystemPossibleFlags");
	return Common::Array<Common::String>();
}

bool TeCore::fileFlagSystemPossibleFlagsContains(const Common::String &name) const {
	error("TODO: Implement TeCore::fileFlagSystemPossibleFlagsContains");
	return false;
}

const Common::String &TeCore::language() const {
	return fileFlagSystemFlag("language");
}

void TeCore::language(const Common::String &val) {
	return fileFlagSystemSetFlag("language", val);
}

bool TeCore::onActivityTrackingAlarm() {
	error("TODO: Implement TeCore::onActivityTrackingAlarm");
}

static bool _checkFileFlag(const Common::String &fname, const Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> &activeTags) {
	Common::StringTokenizer tokenizer(fname, "-");
	while(!tokenizer.empty())
		if (activeTags.getValOrDefault(tokenizer.nextToken(), false))
			return true;
	return false;
}

static void _findFileRecursively(const TetraedgeFSNode &parent,
				 const Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> &activeTags,
				 const Common::String &fname,
				 Common::Array<TetraedgeFSNode> &foundFiles,
				 int maxDepth) {
	if (parent.getChild(Common::Path(fname, '/')).exists()) {
		foundFiles.push_back(parent.getChild(Common::Path(fname, '/')));
	}

	if (maxDepth <= 0)
		return;

	TetraedgeFSList list;
	if (!parent.getChildren(list))
		return;

	for (TetraedgeFSList::const_iterator it = list.begin(); it != list.end(); it++)
		if (_checkFileFlag(it->getName(), activeTags))
			_findFileRecursively(*it, activeTags, fname, foundFiles, maxDepth - 1);
}

TetraedgeFSNode TeCore::findFile(const Common::Path &path, bool quiet) const {
	Common::Array<TetraedgeFSNode> dirNodes;
	const Common::Path dir = path.getParent();

	TetraedgeFSNode node;

	const Common::Array<Common::Archive *> &roots = g_engine->getRootArchives();
	for (Common::Archive *const archive : roots) {
		TetraedgeFSNode archiveNode(archive);
		node = archiveNode.getChild(path);
		if (node.exists())
			return node;
		dirNodes.push_back(archiveNode.getChild(dir));
	}

	Common::String fname = path.getLastComponent().toString();

	// Slight HACK: Remove 'comments' used to specify animated pngs
	if (fname.contains('#'))
		fname = fname.substr(0, fname.find('#'));

	Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> activeFlags;

	for (Common::HashMap<Common::String, Common::String, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo>::const_iterator it = _fileSystemFlags.begin();
	     it != _fileSystemFlags.end(); it++)
		activeFlags[it->_value] = true;

	// This is to keep behivour changes small when we migrated from old system.
	// I'm not sure if it's needed
	// TODO: Figure out what to do with this. Right now we set flag
	// to "SD" but use assets from "HD". This seems to give the best
	// results but is fundamentally wrong.
	activeFlags.erase("SD");
	activeFlags["HD"] = true;

	for (int attempt = 0; attempt < 2; attempt++) {
		if (attempt == 1)
			activeFlags["en"] = true;
		for (uint dirNode = 0; dirNode < dirNodes.size(); dirNode++) {
			Common::Array<TetraedgeFSNode> foundFiles;
			_findFileRecursively(dirNodes[dirNode], activeFlags, fname, foundFiles, 5);
			if (foundFiles.empty())
				continue;
			TetraedgeFSNode best = foundFiles[0];
			int bestDepth = best.getDepth();
			for (uint i = 1; i < foundFiles.size(); i++) {
				int depth = foundFiles[i].getDepth();
				if (depth > bestDepth) {
					bestDepth = depth;
					best = foundFiles[i];
				}
			}

			if (attempt == 1 && !quiet)
				debug("TeCore::findFile Falled back to English for %s", path.toString().c_str());

			return best;
		}
	}

	// Didn't find it at all..
	if (!quiet)
		debug("TeCore::findFile Searched but didn't find %s", path.toString().c_str());
	return TetraedgeFSNode(nullptr, path);
}

} // end namespace Tetraedge
