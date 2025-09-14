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

#include "alcachofa/graphics-opengl.h"
#include "alcachofa/detection.h"

#include "common/system.h"
#include "engines/util.h"

using namespace Common;
using namespace Math;
using namespace Graphics;

namespace Alcachofa {

class OpenGLRendererClassic : public OpenGLRenderer, public virtual IDebugRenderer {
public:
	using OpenGLRenderer::OpenGLRenderer;

	void begin() override {
		GL_CALL(glEnableClientState(GL_VERTEX_ARRAY));
		GL_CALL(glDisableClientState(GL_INDEX_ARRAY));
		GL_CALL(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
		resetState();
		_currentTexture = nullptr;
	}

	void setTexture(ITexture *texture) override {
		if (texture == _currentTexture)
			return;
		else if (texture == nullptr) {
			GL_CALL(glDisable(GL_TEXTURE_2D));
			GL_CALL(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
			_currentTexture = nullptr;
		} else {
			if (_currentTexture == nullptr) {
				GL_CALL(glEnable(GL_TEXTURE_2D));
				GL_CALL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
			}
			auto glTexture = dynamic_cast<OpenGLTexture *>(texture);
			assert(glTexture != nullptr);
			GL_CALL(glBindTexture(GL_TEXTURE_2D, glTexture->handle()));
			_currentTexture = glTexture;
		}
	}

	void setBlendMode(BlendMode blendMode) override {
		if (blendMode == _currentBlendMode)
			return;
		setBlendFunc(blendMode);

		GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE));
		switch (blendMode) {
		case BlendMode::AdditiveAlpha:
		case BlendMode::Additive:
		case BlendMode::Multiply:
			// TintAlpha * TexColor, TexAlpha
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE));

			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA));

			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_ALPHA)); // alpha replaces color
			break;
		case BlendMode::Alpha:
			// TexColor, TintAlpha
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE));

			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA));
			break;
		case BlendMode::Tinted:
			// (TintColor * TintAlpha) * TexColor, TexAlpha
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE));

			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA));

			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR)); // we have to pre-multiply
			break;
		default:
			assert(false && "Invalid blend mode");
			break;
		}
		_currentBlendMode = blendMode;
	}

	void setLodBias(float lodBias) override {
		if (abs(_currentLodBias - lodBias) < epsilon)
			return;
		GL_CALL(glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, lodBias));
		_currentLodBias = lodBias;
	}

	void quad(
		Vector2d topLeft,
		Vector2d size,
		Color color,
		Angle rotation,
		Vector2d texMin,
		Vector2d texMax) override {
		Vector2d positions[4], texCoords[4];
		getQuadPositions(topLeft, size, rotation, positions);
		getQuadTexCoords(texMin, texMax, texCoords);

		if (_currentTexture != nullptr) {
			// float equality is fine here, if it was calculated it was not a normal graphic
			_currentTexture->setMirrorWrap(texMin != Vector2d() || texMax != Vector2d(1, 1));
		}

		float colors[] = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
		if (_currentBlendMode == BlendMode::Tinted) {
			colors[0] *= colors[3];
			colors[1] *= colors[3];
			colors[2] *= colors[3];
		}

		checkFirstDrawCommand();
		GL_CALL(glColor4fv(colors));
		GL_CALL(glVertexPointer(2, GL_FLOAT, 0, positions));
		if (_currentTexture != nullptr)
			GL_CALL(glTexCoordPointer(2, GL_FLOAT, 0, texCoords));
		GL_CALL(glDrawArrays(GL_QUADS, 0, 4));

#ifdef ALCACHOFA_DEBUG
		// make sure we crash instead of someone using our stack arrays
		GL_CALL(glVertexPointer(2, GL_FLOAT, sizeof(Vector2d), nullptr));
		GL_CALL(glTexCoordPointer(2, GL_FLOAT, sizeof(Vector2d), nullptr));
#endif
	}

	void debugPolygon(
		Span<Vector2d> points,
		Color color
	) override {
		checkFirstDrawCommand();
		setTexture(nullptr);
		setBlendMode(BlendMode::Alpha);
		GL_CALL(glVertexPointer(2, GL_FLOAT, 0, points.data()));
		GL_CALL(glLineWidth(4.0f));
		GL_CALL(glPointSize(8.0f));

		GL_CALL(glColor4ub(color.r, color.g, color.b, color.a));
		if (points.size() > 2)
			GL_CALL(glDrawArrays(GL_POLYGON, 0, points.size()));

		color.a = (byte)(MIN(255.0f, color.a * 1.3f));
		GL_CALL(glColor4ub(color.r, color.g, color.b, color.a));
		if (points.size() > 1)
			GL_CALL(glDrawArrays(GL_LINE_LOOP, 0, points.size()));

		color.a = (byte)(MIN(255.0f, color.a * 1.3f));
		GL_CALL(glColor4ub(color.r, color.g, color.b, color.a));
		if (points.size() > 0)
			GL_CALL(glDrawArrays(GL_POINTS, 0, points.size()));
	}

	void debugPolyline(
		Span<Vector2d> points,
		Color color
	) override {
		checkFirstDrawCommand();
		setTexture(nullptr);
		setBlendMode(BlendMode::Alpha);
		GL_CALL(glVertexPointer(2, GL_FLOAT, 0, points.data()));
		GL_CALL(glLineWidth(4.0f));
		GL_CALL(glPointSize(8.0f));

		GL_CALL(glColor4ub(color.r, color.g, color.b, color.a));
		if (points.size() > 1)
			GL_CALL(glDrawArrays(GL_LINE_STRIP, 0, points.size()));

		color.a = (byte)(MIN(255.0f, color.a * 1.3f));
		GL_CALL(glColor4ub(color.r, color.g, color.b, color.a));
		if (points.size() > 0)
			GL_CALL(glDrawArrays(GL_POINTS, 0, points.size()));
	}

	void setMatrices(bool flipped) override {
		float bottom = flipped ? _resolution.y : 0.0f;
		float top = flipped ? 0.0f : _resolution.y;

		GL_CALL(glMatrixMode(GL_PROJECTION));
		GL_CALL(glLoadIdentity());
		GL_CALL(glOrtho(0.0f, _resolution.x, bottom, top, -1.0f, 1.0f));
		GL_CALL(glMatrixMode(GL_MODELVIEW));
		GL_CALL(glLoadIdentity());
	}
};

IRenderer *IRenderer::createOpenGLRendererClassic(Point resolution) {
	debug("Use OpenGL classic renderer");
	return new OpenGLRendererClassic(resolution);
}

}
