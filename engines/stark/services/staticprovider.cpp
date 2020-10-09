/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/services/staticprovider.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/animscript.h"
#include "engines/stark/resources/container.h"
#include "engines/stark/resources/image.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/sound.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/global.h"
#include "engines/stark/visual/image.h"

namespace Stark {

StaticProvider::StaticProvider(ArchiveLoader *archiveLoader) :
		_archiveLoader(archiveLoader),
		_level(nullptr),
		_location(nullptr) {
}

void StaticProvider::init() {
	// Load the static archive
	_archiveLoader->load("static/static.xarc");

	// Set the root tree
	_level = _archiveLoader->useRoot<Resources::Level>("static/static.xarc");

	// Resources lifecycle update
	_level->onAllLoaded();

	Resources::Item *staticItem = _level->findChild<Resources::Item>();
	_stockAnims = staticItem->listChildren<Resources::Anim>();

	for (uint i = 0; i< _stockAnims.size(); i++) {
		_stockAnims[i]->applyToItem(nullptr);
	}

	Resources::Anim *imagesAnim = _stockAnims[kImages];
	_stockImages = imagesAnim->listChildrenRecursive<Resources::Image>();
}

void StaticProvider::onGameLoop() {
	_level->onGameLoop();
}

void StaticProvider::shutdown() {
	if (_location) {
		unloadLocation(_location);
	}

	_level = nullptr;

	_archiveLoader->returnRoot("static/static.xarc");
	_archiveLoader->unloadUnused();
}

VisualImageXMG *StaticProvider::getCursorImage(uint32 cursor) const {
	Resources::Anim *anim = _stockAnims[cursor];
	return anim->getVisual()->get<VisualImageXMG>();
}

VisualImageXMG *StaticProvider::getUIElement(UIElement element) const {
	return getCursorImage(element);
}

VisualImageXMG *StaticProvider::getUIElement(UIElement element, uint32 index) const {
	Resources::Anim *anim = _stockAnims[element];

	uint32 prevIndex = anim->getCurrentFrame();
	anim->selectFrame(index);
	VisualImageXMG *visualImage = anim->getVisual()->get<VisualImageXMG>();
	anim->selectFrame(prevIndex);
	
	return visualImage;
}

VisualImageXMG *StaticProvider::getUIImage(UIImage image) const {
	Resources::Image *anim = _stockImages[image];
	return anim->getVisual()->get<VisualImageXMG>();
}

void StaticProvider::goToAnimScriptStatement(StaticProvider::UIElement stockUIElement, int animScriptItemIndex) {
	Resources::Anim *anim = _stockAnims[stockUIElement];
	Resources::AnimScript *animScript = anim->findChild<Resources::AnimScript>();
	Resources::AnimScriptItem *animScriptItem = animScript->findChildWithIndex<Resources::AnimScriptItem>(animScriptItemIndex);
	animScript->goToScriptItem(animScriptItem);
}

Resources::Sound *StaticProvider::getUISound(UISound sound) const {
	Resources::Item *staticLevelItem = _level->findChild<Resources::Item>();
	Resources::Anim *anim = staticLevelItem->findChildWithOrder<Resources::Anim>(4);
	Resources::Container *sounds = anim->findChildWithSubtype<Resources::Container>(Resources::Container::kSounds);
	return sounds->findChildWithOrder<Resources::Sound>(sound);
}

Common::String StaticProvider::buildLocationArchiveName(const char *locationName) const {
	return Common::String::format("static/%s/%s.xarc", locationName, locationName);
}

Resources::Location *StaticProvider::loadLocation(const char *locationName) {
	assert(!_location);

	Common::String archiveName = buildLocationArchiveName(locationName);

	_archiveLoader->load(archiveName);
	_location = _archiveLoader->useRoot<Resources::Location>(archiveName);

	_location->onAllLoaded();
	_location->onEnterLocation();

	// Start background music
	Common::Array<Resources::Sound *> sounds = _location->listChildren<Resources::Sound>(Resources::Sound::kSoundBackground);
	for (uint i = 0; i < sounds.size(); i++) {
		sounds[i]->play();
	}

	return _location;
}

void StaticProvider::unloadLocation(Resources::Location *location) {
	assert(_location == location);

	location->onExitLocation();

	Common::String archiveName = buildLocationArchiveName(location->getName().c_str());
	_archiveLoader->returnRoot(archiveName);
	_archiveLoader->unloadUnused();

	_location = nullptr;
}

bool StaticProvider::isStaticLocation() const {
	return _location != nullptr;
}

Resources::Location *StaticProvider::getLocation() const {
	return _location;
}

Resources::Sound *StaticProvider::getLocationSound(uint16 index) const {
	assert(_location);

	Resources::Container *sounds = _location->findChildWithSubtype<Resources::Container>(Resources::Container::kSounds);
	return sounds->findChildWithIndex<Resources::Sound>(index);
}

} // End of namespace Stark
