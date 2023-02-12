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

#ifndef TETRAEDGE_TE_TE_MATERIAL_H
#define TETRAEDGE_TE_TE_MATERIAL_H

#include "common/ptr.h"
#include "common/path.h"
#include "common/stream.h"

#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_3d_texture.h"
#include "tetraedge/te/te_intrusive_ptr.h"

namespace Tetraedge {

class TeMaterial {
public:
	enum Mode {
		MaterialMode0,
		MaterialMode1,
		MaterialMode2
	};

	TeMaterial();
	TeMaterial(const TeMaterial &other) = default;
	TeMaterial(TeIntrusivePtr<Te3DTexture> texture, Mode mode);

	// Note: apply() function from original moved to TeRenderer to remove OGL specific code from here
	void defaultValues();
	static void deserialize(Common::SeekableReadStream &stream, TeMaterial &material, const Common::String &path);
	static void serialize(Common::SeekableWriteStream &stream, TeMaterial &material);

	bool operator==(const TeMaterial &other) const;
	bool operator!=(const TeMaterial &other) const {
		return !operator==(other);
	}

	TeMaterial &operator=(const TeMaterial &other);

	Common::String dump() const;

	TeIntrusivePtr<Te3DTexture> _texture;
	Mode _mode;
	TeColor _ambientColor;
	TeColor _diffuseColor;
	TeColor _specularColor;
	TeColor _emissionColor;
	float _shininess;
	bool _isShadowTexture;
	bool _enableLights;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_MATERIAL_H
