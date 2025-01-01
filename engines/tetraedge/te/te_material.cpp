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

#include "common/textconsole.h"

#include "tetraedge/tetraedge.h"

#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_material.h"
#include "tetraedge/te/te_model.h"
#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

TeMaterial::TeMaterial() {
	defaultValues();
	_mode = MaterialMode1;
}

TeMaterial::TeMaterial(TeIntrusivePtr<Te3DTexture> texture, Mode mode) {
	defaultValues();
	_texture = texture;
	_mode = mode;
}

void TeMaterial::defaultValues() {
	_ambientColor = TeColor(0, 0, 0, 255);
	_diffuseColor = TeColor(255, 255, 255, 255);
	_specularColor = TeColor(0, 0, 0, 255);
	_emissionColor = TeColor(0, 0, 0, 255);
	_shininess = 0.0;
	_enableLights = false;
	_isShadowTexture = false;
}

Common::String TeMaterial::dump() const {
	return Common::String::format
			("amb:%s dif:%s spe:%s emi:%s mode:%d tex:%s shin:%.02f lights:%s",
			  _ambientColor.dump().c_str(),
			  _diffuseColor.dump().c_str(),
			  _specularColor.dump().c_str(),
			  _emissionColor.dump().c_str(),
			  (int)_mode,
			 _texture ? _texture->getAccessName().toString('/').c_str() : "None",
			  _shininess, _enableLights ? "on" : "off");
}

bool TeMaterial::operator==(const TeMaterial &other) const {
	return (_texture == other._texture) && (_ambientColor == other._ambientColor)
		&& (_diffuseColor == other._diffuseColor) && (_specularColor == other._specularColor)
		&& (_emissionColor == other._emissionColor) && (_enableLights == other._enableLights)
		&& (_shininess == other._shininess) && (_mode == other._mode);
}

TeMaterial &TeMaterial::operator=(const TeMaterial &other) {
	if (&other == this)
		return *this;

	_texture = other._texture;
	_ambientColor = other._ambientColor;
	_diffuseColor = other._diffuseColor;
	_specularColor = other._specularColor;
	_emissionColor = other._emissionColor;
	_enableLights = other._enableLights;
	_shininess = other._shininess;
	_mode = other._mode;
	_isShadowTexture = other._isShadowTexture;

	return *this;
}

/*static*/
void TeMaterial::deserialize(Common::SeekableReadStream &stream, TeMaterial &material, const Common::Path &texPath) {
	const Common::String nameStr = Te3DObject2::deserializeString(stream);

	TeModel::loadAlign(stream);
	material._mode = static_cast<TeMaterial::Mode>(stream.readUint32LE());

	if (nameStr.size()) {
		TeCore *core = g_engine->getCore();
		TetraedgeFSNode matPath = core->findFile(Common::Path(texPath).join(nameStr));
		material._texture = Te3DTexture::load2(matPath, false);
		if (!material._texture)
			warning("failed to load texture %s (texpath %s)", nameStr.c_str(), matPath.toString().c_str());
	}

	material._ambientColor.deserialize(stream);
	material._diffuseColor.deserialize(stream);
	material._specularColor.deserialize(stream);
	// TODO: Confirm this - Surely this should be emission color,
	// but the original doesn't assign the result
	// to _emissionColor. It does read though.
	TeColor c;
	c.deserialize(stream);
	material._shininess = stream.readFloatLE();
}

/*static*/ void TeMaterial::serialize(Common::SeekableWriteStream &stream, TeMaterial &material) {
	Te3DTexture *tex = material._texture.get();
	Common::String texName;
	if (tex) {
		texName = tex->getAccessName().toString('/');
		// "Remove extension" twice for some reason..
		size_t offset = texName.rfind('.');
		if (offset != Common::String::npos) {
			texName.substr(0, offset);
		}
		offset = texName.rfind('.');
		if (offset != Common::String::npos) {
			texName.substr(0, offset);
		}
	}
	stream.writeUint32LE(texName.size());
	stream.write(texName.c_str(), texName.size());
	TeModel::saveAlign(stream);
	stream.writeUint32LE(static_cast<uint32>(material._mode));
	material._ambientColor.serialize(stream);
	material._diffuseColor.serialize(stream);
	material._specularColor.serialize(stream);
	material._emissionColor.serialize(stream);
	stream.writeFloatLE(material._shininess);
}

} // end namespace Tetraedge
