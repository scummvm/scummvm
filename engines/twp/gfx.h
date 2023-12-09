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

#ifndef TWP_GFX_H
#define TWP_GFX_H

#include "common/array.h"
#include "graphics/surface.h"
#include "common/hashmap.h"
#include "common/rect.h"
#include "math/vector2d.h"
#include "math/matrix4.h"

namespace Twp {

struct Color {
	union {
		float v[4];
		struct {
			float r;
			float g;
			float b;
			float a;
		} rgba;
	};

	Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f) {
		rgba.r = red;
		rgba.g = green;
		rgba.b = blue;
		rgba.a = alpha;
	}

	static Color create(uint8 red, uint8 green, uint8 blue, uint8 alpha = 0xFF) {
		return Color(red / 255.f, green / 255.f, blue / 255.f, alpha / 255.f);
	}
};

// This is a point in 2D with a color and texture coordinates
struct Vertex {
public:
	Math::Vector2d pos;
	Color color;
	Math::Vector2d texCoords;
};

class Texture {
public:
	void load(const Graphics::Surface& surface);
	static void bind(const Texture *pTexture);

public:
	uint32 id;
    int width, height;
    uint32 fbo;
};

struct TextureSlot {
	int id;
	Texture texture;
};

class Shader {
public:
	Shader();

	void init(const char *vertex, const char *fragment);

	void setUniform(const char *name, Math::Matrix4 value);

private:
	uint32 loadShader(const char *code, uint32 shaderType);
	void statusShader(uint32 shader);
	int getUniformLocation(const char *name);

public:
	uint32 program;

private:
	uint32 _vertex;
	uint32 _fragment;
	Common::HashMap<int, TextureSlot> _textures;
};

typedef Common::HashMap<int, TextureSlot> Textures;

class Gfx {
public:
	Gfx();

	void init();

	void camera(float w, float h);

	void clear(Color color);
	void drawPrimitives(uint32 primitivesType, Vertex* vertices, int v_size, Math::Matrix4 transf = Math::Matrix4());
	void drawPrimitives(uint32 primitivesType, Vertex* vertices, int v_size, uint32* indices, int i_size, Math::Matrix4 transf = Math::Matrix4());
	void drawLines(Vertex* vertices, int count, Math::Matrix4 trsf = Math::Matrix4());
	void draw(Vertex* vertices, int v_size, uint32* indices, int i_size, Math::Matrix4 trsf = Math::Matrix4());
	void drawQuad(Math::Vector2d size, Color color = Color(), Math::Matrix4 trsf = Math::Matrix4());
	void drawSprite(Common::Rect textRect, Texture& texture, Color color = Color(), Math::Matrix4 trsf = Math::Matrix4(), bool flipX = false, bool flipY = false);

	void noTexture();
	Math::Matrix4 getFinalTransform(Math::Matrix4 trsf);

public:
	Math::Vector2d _cameraPos;

private:
	uint32 _vbo, _ebo;
	Shader _shader;
	Color _color;
	Math::Matrix4 _mvp;
	Math::Vector2d _cameraSize;
	Textures _textures;
	Texture* _texture;
};
}

#endif
