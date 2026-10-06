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

#ifndef MADS_NEBULAR_MAC_RESOURCES_H
#define MADS_NEBULAR_MAC_RESOURCES_H

#include "common/scummsys.h"
#include "common/str.h"
#include "graphics/managed_surface.h"
#include "mads/core/env.h"

namespace Common {
class MacResManager;
class SeekableReadStream;
}

namespace Graphics {
class Font;
class MacFontManager;
}

namespace MADS {
namespace RexNebular {

class MacResourceProvider : public EnvResourceProvider {
public:
	enum Container {
		kInvalidContainer,
		kApplicationContainer,
		kGlobalContainer,
		kSoundContainer,
		kSection1Container,
		kSection9Container = kSection1Container + 8
	};

	struct ResourceID {
		Container container;
		uint32 type;
		uint16 id;

		ResourceID(Container container_ = kInvalidContainer, uint32 type_ = 0, uint16 id_ = 0) :
				container(container_), type(type_), id(id_) {}

		bool isValid() const { return container != kInvalidContainer; }
	};

	MacResourceProvider();
	~MacResourceProvider() override;

	bool load();
	Common::SeekableReadStream *openResource(Container container, uint32 type,
		uint16 id) const;
	Common::String getApplicationVersion() const;
	Common::SeekableReadStream *open(const char *filename) override;
	bool exists(const char *filename) override;
	bool allowsFallback(const char *filename) const override;
	Common::SeekableReadStream *openText(int32 id, uint16 &unpackedSize) override;
	Common::SeekableReadStream *openSound(int section, int commandId);
	int getCursorCount() const override { return 6; }
	bool setCursor(int id) override;
	void updateCursor() override;
	const Graphics::Surface *getNativeInterface() const {
		return _nativeInterface.empty() ? nullptr : &_nativeInterface.rawSurface();
	}
	const Graphics::Surface *getLogicalInterface() const {
		return _logicalInterface.empty() ? nullptr : &_logicalInterface.rawSurface();
	}
	const Graphics::Font *getDialogFont();
	const Graphics::Font *getInterfaceFont();
	const Graphics::Font *getGameFont();
	const Graphics::Font *getAboutFont(int size, bool bold);
	const byte *getNativeInterfacePalette() const { return _nativeInterfacePalette; }

	static ResourceID mapResource(const Common::String &filename);

private:
	Common::MacResManager *_containers[kSection9Container + 1] = {};
	Graphics::MacFontManager *_fontManager = nullptr;
	Graphics::ManagedSurface _nativeInterface;
	Graphics::ManagedSurface _logicalInterface;
	byte _nativeInterfacePalette[10 * 3] = {};
	int _cursorID = 0;
	int _waitCursorFrame = 0;
	uint32 _nextCursorTime = 0;

	Common::MacResManager *getContainer(Container container) const;
	Common::SeekableReadStream *createFontResource();
	Common::SeekableReadStream *createInterfaceResource(int interfaceID);
	bool installCursorResource(uint16 id);
};

} // namespace RexNebular
} // namespace MADS

#endif
