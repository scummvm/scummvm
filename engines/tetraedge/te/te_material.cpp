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
#include "graphics/opengl/system_headers.h"

#include "tetraedge/tetraedge.h"

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
	_enableSomethingDefault0 = false;
}

Common::String TeMaterial::dump() const {
	return Common::String::format
			("amb:%s dif:%s spe:%s emi:%s mode:%d tex:%s shin:%.02f lights:%s",
			  _ambientColor.dump().c_str(),
			  _diffuseColor.dump().c_str(),
			  _specularColor.dump().c_str(),
			  _emissionColor.dump().c_str(),
			  (int)_mode,
			 _texture ? _texture->getAccessName().toString().c_str() : "None",
			  _shininess, _enableLights ? "on" : "off");
}

void TeMaterial::apply() const {
	//debug("TeMaterial::apply (%s)", dump().c_str());
	static const float constColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	TeRenderer *renderer = g_engine->getRenderer();
	if (renderer->shadowMode() == TeRenderer::ShadowMode0) {
		if (_enableLights)
			TeLight::enableAll();
		else
			TeLight::disableAll();

		if (_texture) {
			renderer->enableTexture();
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			_texture->bind();
		}

		glDisable(GL_ALPHA_TEST);
		if (_mode == MaterialMode0) {
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_CONSTANT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		} else {
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			if (_mode != MaterialMode1) {
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, 0.5);
			}
		}
		const float ambient[4] = { _ambientColor.r() / 255.0f, _ambientColor.g() / 255.0f,
			_ambientColor.b() / 255.0f, _ambientColor.a() / 255.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);

		const float specular[4] = { _specularColor.r() / 255.0f, _specularColor.g() / 255.0f,
			_specularColor.b() / 255.0f, _specularColor.a() / 255.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);

		const float emission[4] = { _emissionColor.r() / 255.0f, _emissionColor.g() / 255.0f,
			_emissionColor.b() / 255.0f, _emissionColor.a() / 255.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);

		glMaterialf(GL_FRONT, GL_SHININESS, _shininess);

		const float diffuse[4] = { _diffuseColor.r() / 255.0f, _diffuseColor.g() / 255.0f,
			_diffuseColor.b() / 255.0f, _diffuseColor.a() / 255.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);

		renderer->setCurrentColor(_diffuseColor);
	} else if (renderer->shadowMode() == TeRenderer::ShadowMode1) {
		// NOTE: Diverge from original here, it sets 255.0 but the
		// colors should be scaled -1.0 .. 1.0.
		static const float fullColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		TeLight::disableAll();
		glDisable(GL_ALPHA_TEST);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fullColor);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, fullColor);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fullColor);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, fullColor);
	}

	//warning("TODO: Work out what TeMaterial::_enableSomethingDefault0 actually is.");
	if (!_enableSomethingDefault0) {
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_GEN_Q);
	} else {
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_TEXTURE_GEN_Q);
		glEnable(GL_TEXTURE_2D);
		TeLight::disableAll();
		glDisable(GL_ALPHA_TEST);
		renderer->enableTexture();
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		const float diffuse[4] = { _diffuseColor.r() / 255.0f, _diffuseColor.g() / 255.0f,
			_diffuseColor.b() / 255.0f, _diffuseColor.a() / 255.0f };

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, diffuse);
	}
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
	_enableLights = other._enableLights;
	_enableSomethingDefault0 = other._enableSomethingDefault0;

	return *this;
}

/*static*/ void TeMaterial::deserialize(Common::SeekableReadStream &stream, TeMaterial &material, const Common::Path &texPath) {
	Common::String nameStr = Te3DObject2::deserializeString(stream);

	TeModel::loadAlign(stream);
	material._mode = static_cast<TeMaterial::Mode>(stream.readUint32LE());

	if (nameStr.size()) {
		Common::Path fullTexPath = texPath.join(nameStr);
		material._texture = Te3DTexture::load2(fullTexPath, 0x500);
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
		texName = tex->getAccessName().toString();
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
