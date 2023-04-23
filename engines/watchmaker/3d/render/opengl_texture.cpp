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

#include "watchmaker/3d/render/opengl_texture.h"
#include "common/textconsole.h"
#include "graphics/opengl/system_headers.h"
#include "watchmaker/3d/dds_header.h"
#include "watchmaker/render.h"

#ifdef USE_OPENGL_GAME

namespace Watchmaker {

GLuint dxtCompressionToTextureFormat(DxtCompression compression) {
	switch (compression) {
	case DxtCompression::UNCOMPRESSED:
		return GL_RGBA;
	case DxtCompression::DXT1:
		return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	case DxtCompression::DXT2:
		error("DXT2 Support is not implemented");
	case DxtCompression::DXT3:
		return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	case DxtCompression::DXT4:
		error("DXT4 Support is not implemented");
	case DxtCompression::DXT5:
		return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	}
}

class OpenGLTexture : public Texture {
public:
	unsigned int _texId;
	OpenGLTexture() {
		glGenTextures(1, &_texId);
	}
	OpenGLTexture(unsigned int texId) : _texId(texId) {}
	void assignData(const TextureData &data) override {
		glBindTexture(GL_TEXTURE_2D, _texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		// TODO: Check both compiletime and runtime for the existence of EXT_texture_compression_s3tc
		GLuint texFormat = dxtCompressionToTextureFormat(data._compression);
		bool compressed = data._compression != DxtCompression::UNCOMPRESSED;

		if (compressed) {
			glCompressedTexImage2D(GL_TEXTURE_2D, // target
								   0, // level
								   texFormat, // internalFormat
								   data.getWidth(), // width
								   data.getHeight(), // height
								   0, // border
								   data.getDataSize(),
								   data.getData()
			);
			checkGlError("glCompressedTexImage");
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, texFormat, data.getWidth(), data.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, data.getData());
			checkGlError("glTexImage2D");
		}
	}
	void bind() override {
		glBindTexture(GL_TEXTURE_2D, _texId);
		checkGlError("OpenGLTexture::bind");
	};
};

Texture *createGLTexture() {
	return new OpenGLTexture();
}

} // End of namespace Watchmaker

#endif // USE_OPENGL_GAME
