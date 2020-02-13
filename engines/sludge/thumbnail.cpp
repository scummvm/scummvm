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

#include "common/savefile.h"
#include "image/png.h"

#include "sludge/allfiles.h"
#include "sludge/backdrop.h"
#include "sludge/errors.h"
#include "sludge/graphics.h"
#include "sludge/imgloader.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/sludger.h"
#include "sludge/version.h"

namespace Sludge {

bool GraphicsManager::setThumbnailSize(int thumbWidth, int thumbHeight)
{
	if (checkSizeValide(thumbWidth, thumbHeight))
	{
		_thumbWidth = thumbWidth;
		_thumbHeight = thumbHeight;
		return true;
	}
	return false;
}

bool GraphicsManager::saveThumbnail(Common::WriteStream *stream) {

	stream->writeUint32LE(_thumbWidth);
	stream->writeUint32LE(_thumbHeight);

	if (_thumbWidth && _thumbHeight) {
		if (!freeze())
			return false;

		if(!Image::writePNG(*stream, _renderSurface))
			return false;

		unfreeze(true);
	}
	stream->writeByte('!');
	return true;
}

void GraphicsManager::showThumbnail(const Common::String &filename, int atX, int atY) {
	Common::InSaveFile *fp = g_system->getSavefileManager()->openForLoading(filename);

	if (fp == nullptr)
		return;

	bool headerBad = false;
	if (fp->readByte() != 'S')
		headerBad = true;
	if (fp->readByte() != 'L')
		headerBad = true;
	if (fp->readByte() != 'U')
		headerBad = true;
	if (fp->readByte() != 'D')
		headerBad = true;
	if (fp->readByte() != 'S')
		headerBad = true;
	if (fp->readByte() != 'A')
		headerBad = true;
	if (headerBad) {
		fatal(ERROR_GAME_LOAD_NO, filename);
		return;
	}
	char c = fp->readByte();
	while ((c = fp->readByte()))
		;

	int majVersion = fp->readByte();
	int minVersion = fp->readByte();
	int ssgVersion = VERSION(majVersion, minVersion);

	if (ssgVersion >= VERSION(1, 4)) {
		int fileWidth = fp->readUint32LE();
		int fileHeight = fp->readUint32LE();

		Graphics::TransparentSurface thumbnail;
		if (!ImgLoader::loadPNGImage(fp, &thumbnail))
			return;

		delete fp;
		fp = nullptr;


		if (atX < 0) {
			fileWidth += atX;
			atX = 0;
		}
		if (atY < 0) {
			fileHeight += atY;
			atY = 0;
		}
		if (fileWidth + atX > (int)_sceneWidth)
			fileWidth = _sceneWidth - atX;
		if (fileHeight + atY > (int)_sceneHeight)
			fileHeight = _sceneHeight - atY;

		thumbnail.blit(_backdropSurface, atX, atY, Graphics::FLIP_NONE, nullptr, TS_ARGB((uint)255, (uint)255, (uint)255, (uint)255), fileWidth, fileHeight);
		thumbnail.free();
	}
}

bool GraphicsManager::skipThumbnail(Common::SeekableReadStream *stream) {
	_thumbWidth = stream->readUint32LE();
	_thumbHeight = stream->readUint32LE();

	// Load image
	Graphics::Surface tmp;
	if (_thumbWidth & _thumbHeight) {
		if (!ImgLoader::loadPNGImage(stream, &tmp))
			return false;
		else
			tmp.free();
	}

	// Check flag
	return (stream->readByte() == '!');
	return true;
}

} // End of namespace Sludge
