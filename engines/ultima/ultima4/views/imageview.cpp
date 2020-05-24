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

#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/views/imageview.h"

namespace Ultima {
namespace Ultima4 {

ImageView::ImageView(int x, int y, int width, int height) : View(x, y, width, height) {
}

ImageView::~ImageView() {
}

void ImageView::draw(const Common::String &imageName, int x, int y) {
	ImageInfo *info = imageMgr->get(imageName);
	if (info) {
		info->_image->draw(SCALED(_bounds.left + x), SCALED(_bounds.top + y));
		return;
	}

	SubImage *subimage = imageMgr->getSubImage(imageName);
	if (subimage) {
		info = imageMgr->get(subimage->_srcImageName);

		if (info) {
			info->_image->drawSubRect(SCALED(_bounds.left + x), SCALED(_bounds.top + y),
			                          SCALED(subimage->left) / info->_prescale,
			                          SCALED(subimage->top) / info->_prescale,
			                          SCALED(subimage->width()) / info->_prescale,
			                          SCALED(subimage->height()) / info->_prescale);
			return;
		}
	}

	error("ERROR 1005: Unable to load the image \"%s\"", imageName.c_str());
}

} // End of namespace Ultima4
} // End of namespace Ultima
