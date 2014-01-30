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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(WIN32)
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE
#endif

#include "engines/myst3/gfx.h"
#include "engines/myst3/gfx_opengl_texture.h"

#include "common/rect.h"
#include "common/textconsole.h"

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

#include "graphics/colormasks.h"
#include "graphics/surface.h"

#include "math/vector2d.h"
#include "math/rect2d.h"
#include "math/quat.h"

#include "graphics/opengles2/shader.h"

namespace Myst3 {

static struct {
	Graphics::Shader *g_box_shader;
	Graphics::Shader *g_cube_shader;
	Graphics::Shader *g_rect3d_shader;
	Graphics::Shader *g_text_shader;

	GLuint g_boxVBO;
	GLuint g_cubeVBO;
	GLuint g_rect3dVBO;
	GLuint g_quadEBO;

	Math::Matrix4 g_mvpMatrix;
	Math::Rect2d g_viewport;
} g_shaders;

static const GLfloat box_vertices[] = {
// XS   YT
  0.0, 0.0,
  1.0, 0.0,
  0.0, 1.0,
  1.0, 1.0,
};


static const GLfloat cube_vertices[] = {
//   S     T      X      Y      Z
    0.0f, 1.0f, -1.0f, -1.0f,  1.0f,
    1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
    0.0f, 0.0f, -1.0f,  1.0f,  1.0f,
    1.0f, 0.0f, -1.0f,  1.0f, -1.0f,
    0.0f, 1.0f,  1.0f, -1.0f, -1.0f,
    1.0f, 1.0f,  1.0f, -1.0f,  1.0f,
    0.0f, 0.0f,  1.0f,  1.0f, -1.0f,
    1.0f, 0.0f,  1.0f,  1.0f,  1.0f,
    0.0f, 1.0f,  1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
    0.0f, 0.0f,  1.0f, -1.0f,  1.0f,
    1.0f, 0.0f, -1.0f, -1.0f,  1.0f,
    0.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f, 1.0f, -1.0f,  1.0f,  1.0f,
    0.0f, 0.0f,  1.0f,  1.0f, -1.0f,
    1.0f, 0.0f, -1.0f,  1.0f, -1.0f,
    0.0f, 1.0f, -1.0f, -1.0f, -1.0f,
    1.0f, 1.0f,  1.0f, -1.0f, -1.0f,
    0.0f, 0.0f, -1.0f,  1.0f, -1.0f,
    1.0f, 0.0f,  1.0f,  1.0f, -1.0f,
    0.0f, 1.0f,  1.0f, -1.0f,  1.0f,
    1.0f, 1.0f, -1.0f, -1.0f,  1.0f,
    0.0f, 0.0f,  1.0f,  1.0f,  1.0f,
    1.0f, 0.0f, -1.0f,  1.0f,  1.0f,
};


void setupQuadEBO() {
  // FIXME: Probably way too big...
  unsigned short quad_indices[6 * 1000];

  unsigned short start = 0;
  for (unsigned short *p = quad_indices; p < &quad_indices[6 * 1000]; p += 6) {
    p[0] = p[3] = start++;
    p[1] = start++;
    p[2] = p[4] = start++;
    p[5] = start++;
  }

  g_shaders.g_quadEBO = Graphics::Shader::createBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);
}

static Math::Vector2d scaled(float x, float y) {
  return Math::Vector2d(x / Renderer::kOriginalWidth, y / Renderer::kOriginalHeight);
}

Renderer::Renderer(OSystem *system) :
  _system(system),
  _font(0),
  _nonPowerOfTwoTexSupport(false) {
}

Renderer::~Renderer() {
  if (_font)
    freeTexture(_font);
}

Texture *Renderer::createTexture(const Graphics::Surface *surface) {
  return new OpenGLTexture(surface);
}

void Renderer::freeTexture(Texture *texture) {
  OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);
  delete glTexture;
}

void Renderer::init() {
#ifndef USE_GLES2
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    error("Error: %s\n", glewGetErrorString(err));
  }
  assert(GLEW_OK == err);
#endif

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);

  const char* attributes[] = { "position", "texcoord", NULL };
  g_shaders.g_box_shader = Graphics::Shader::fromFiles("myst3_box", attributes);
  g_shaders.g_boxVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(box_vertices), box_vertices);
  g_shaders.g_box_shader->enableVertexAttribute("position", g_shaders.g_boxVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
  g_shaders.g_box_shader->enableVertexAttribute("texcoord", g_shaders.g_boxVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);


  g_shaders.g_cube_shader = Graphics::Shader::fromFiles("myst3_cube", attributes);
  g_shaders.g_cubeVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices);
  g_shaders.g_cube_shader->enableVertexAttribute("texcoord", g_shaders.g_cubeVBO, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(float), 0);
  g_shaders.g_cube_shader->enableVertexAttribute("position", g_shaders.g_cubeVBO, 3, GL_FLOAT, GL_TRUE, 5 * sizeof(float), 2 * sizeof(float));

  g_shaders.g_rect3d_shader = Graphics::Shader::fromFiles("myst3_cube", attributes);
  g_shaders.g_rect3dVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, 20 * sizeof(float), NULL);
  g_shaders.g_rect3d_shader->enableVertexAttribute("texcoord", g_shaders.g_rect3dVBO, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(float), 0);
  g_shaders.g_rect3d_shader->enableVertexAttribute("position", g_shaders.g_rect3dVBO, 3, GL_FLOAT, GL_TRUE, 5 * sizeof(float), 2 * sizeof(float));

  setupQuadEBO();
}

void Renderer::initFont(const Graphics::Surface *surface) {
  _font = createTexture(surface);
}

void Renderer::clear() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setupCameraOrtho2D() {
  glViewport(0, 0, kOriginalWidth, kOriginalHeight);
}

void Renderer::setupCameraPerspective(float pitch, float heading, float fov) {
  // TODO: Find a correct and exact formula for the FOV
  GLfloat glFOV = 0.63 * fov; // Approximative and experimental formula
  if (fov > 79.0 && fov < 81.0)
    glFOV = 50.5; // Somewhat good value for fov == 80
  else if (fov > 59.0 && fov < 61.0)
    glFOV = 36.0; // Somewhat good value for fov == 60

  glViewport(0, kBottomBorderHeight, kOriginalWidth, kFrameHeight);

  const Math::Vector2d topLeft = Math::Vector2d(0, kBottomBorderHeight + kFrameHeight);
  const Math::Vector2d bottomRight = Math::Vector2d(kOriginalWidth, kBottomBorderHeight);
  g_shaders.g_viewport = Math::Rect2d(topLeft, bottomRight);

  float nclip = 1.0, fclip = 10000.0;
  float aspect = g_shaders.g_viewport.getWidth() / g_shaders.g_viewport.getHeight();

  // taken from glm
  float range = nclip * tan(glFOV / 2 * (LOCAL_PI / 180));
  float left = -range * aspect;
  float right = range * aspect;
  float bottom = -range;
  float top = range;

  Math::Matrix4 proj;
  proj(0,0) = (2.0f * nclip) / (right - left);
  proj(1,1) = (2.0f * nclip) / (top - bottom);
  proj(2,0) = (right + left) / (right - left);
  proj(2,1) = 0.0f; // (top + bottom) / (top - bottom);
  proj(2,2) = -(fclip + nclip) / (fclip - nclip);
  proj(2,3) = -1.0f;
  proj(3,2) = -(2.0f * fclip * nclip) / (fclip - nclip);
  proj(3,3) = 0.0f;
  proj.transpose();

  Math::Matrix4 model = Math::Quaternion::fromEuler(180.0f - heading, pitch, 0.0f).toMatrix();
  model.transpose();

  g_shaders.g_mvpMatrix = proj * model;
  g_shaders.g_mvpMatrix.transpose();
}

void Renderer::drawRect2D(const Common::Rect &rect, uint32 color) {
  uint8 a, r, g, b;
  Graphics::colorToARGB< Graphics::ColorMasks<8888> >(color, a, r, g, b);

  g_shaders.g_box_shader->use();
  g_shaders.g_box_shader->setUniform("textured", false);
  g_shaders.g_box_shader->setUniform("color", Math::Vector4d(r / 255.0, g / 255.0, b / 255.0, a / 255.0));
  g_shaders.g_box_shader->setUniform("verOffsetXY", scaled(rect.left, rect.top));
  g_shaders.g_box_shader->setUniform("verSizeWH", scaled(rect.width(), rect.height()));

  glDisable(GL_TEXTURE_2D);
  glDepthMask(GL_FALSE);

  if (a != 255) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

void Renderer::drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect,
    Texture *texture, float transparency) {

  OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);

  const float tLeft = textureRect.left / (float) glTexture->internalWidth;
  const float tWidth = textureRect.width() / (float) glTexture->internalWidth;
  const float tTop = textureRect.top / (float) glTexture->internalHeight;
  const float tHeight = textureRect.height() / (float) glTexture->internalHeight;

  const float sLeft = screenRect.left;
  const float sTop = screenRect.top;
  const float sWidth = screenRect.width();
  const float sHeight = screenRect.height();

  if (transparency >= 0.0) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
  } else {
    transparency = 1.0;
  }

  g_shaders.g_box_shader->use();
  g_shaders.g_box_shader->setUniform("textured", true);
  g_shaders.g_box_shader->setUniform("color", Math::Vector4d(1.0f, 1.0f, 1.0f, transparency));
  g_shaders.g_box_shader->setUniform("verOffsetXY", scaled(sLeft, sTop));
  g_shaders.g_box_shader->setUniform("verSizeWH", scaled(sWidth, sHeight));
  g_shaders.g_box_shader->setUniform("texOffsetXY", Math::Vector2d(tLeft, tTop));
  g_shaders.g_box_shader->setUniform("texSizeWH", Math::Vector2d(tWidth, tHeight));

  glEnable(GL_TEXTURE_2D);
  glDepthMask(GL_FALSE);

  glBindTexture(GL_TEXTURE_2D, glTexture->id);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

Common::Rect Renderer::getFontCharacterRect(uint8 character) {
  uint index = 0;

  if (character == ' ')
    index = 0;
  else if (character >= '0' && character <= '9')
    index = 1 + character - '0';
  else if (character >= 'A' && character <= 'Z')
    index = 1 + 10 + character - 'A';
  else if (character == '|')
    index = 1 + 10 + 26;

  return Common::Rect(16 * index, 0, 16 * (index + 1), 32);
}

void Renderer::draw2DText(const Common::String &text, const Common::Point &position) {
  return;
  static Common::String _prevText = "";
  static Common::Point _prevPosition = Common::Point(0.0, 0.0);
  static GLuint _prevVBO = 0;

  OpenGLTexture *glFont = static_cast<OpenGLTexture *>(_font);

  // The font only has uppercase letters
  Common::String textToDraw = text;
  textToDraw.toUppercase();

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_TEXTURE_2D);
  glDepthMask(GL_FALSE);

  if (_prevText != textToDraw || _prevPosition != position) {
    _prevText = textToDraw;
    _prevPosition = position;
    if (_prevVBO)
      glDeleteBuffers(1, &_prevVBO);

    float x = position.x / g_shaders.g_viewport.getWidth();
    float y = position.y / g_shaders.g_viewport.getHeight();

    float *bufData = new float[textToDraw.size()];
    float *cur = bufData;

    for (uint i = 0; i < textToDraw.size(); i++) {
      Common::Rect textureRect = getFontCharacterRect(textToDraw[i]);
      float w = textureRect.width() / g_shaders.g_viewport.getWidth();
      float h = textureRect.height() / g_shaders.g_viewport.getHeight();

      float cw = textureRect.width() / (float) glFont->internalWidth;
      float ch = textureRect.height() / (float) glFont->internalHeight;
      float cx = textureRect.left / (float) glFont->internalWidth;
      float cy = textureRect.top / (float) glFont->internalHeight;

      const float charData[] = {
        cx,      cy + ch, x,     y,     1.0f,
        cx + cw, cy + ch, x + w, y,     1.0f,
        cx + cw, cy,      x + w, y + h, 1.0f,
        cx,      cy,      x,     y + h, 1.0f,
      };

      memcpy(cur, charData, 20 * sizeof(float));
      cur += 20;

      x += textureRect.width() - 3;
    }

    GLuint vbo = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, textToDraw.size() * 20 * sizeof(float), bufData, GL_STATIC_DRAW);
    _prevVBO = vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    g_shaders.g_text_shader->enableVertexAttribute("texcoord", vbo, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(float), 0);
    g_shaders.g_text_shader->enableVertexAttribute("position", vbo, 3, GL_FLOAT, GL_TRUE, 5 * sizeof(float), 2 * sizeof(float));
  }

  g_shaders.g_text_shader->use();
  glBindTexture(GL_TEXTURE_2D, glFont->id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_shaders.g_quadEBO);
  glDrawElements(GL_TRIANGLES, textToDraw.size(), GL_UNSIGNED_SHORT, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

void Renderer::drawCube(Texture **textures) {
  OpenGLTexture *texture0 = static_cast<OpenGLTexture *>(textures[0]);

  glEnable(GL_TEXTURE_2D);
  glDepthMask(GL_FALSE);

  g_shaders.g_cube_shader->use();
  g_shaders.g_cube_shader->setUniform1f("verScale", 256.0f);
  g_shaders.g_cube_shader->setUniform1f("texScale", texture0->width / (float) texture0->internalWidth);
  g_shaders.g_cube_shader->setUniform("mvpMatrix", g_shaders.g_mvpMatrix);

  glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[4])->id);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[3])->id);
  glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

  glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[1])->id);
  glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);

  glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[5])->id);
  glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);

  glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[0])->id);
  glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);

  glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[2])->id);
  glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);

  glDepthMask(GL_TRUE);
}

void Renderer::drawTexturedRect3D(const Math::Vector3d &topLeft, const Math::Vector3d &bottomLeft,
    const Math::Vector3d &topRight, const Math::Vector3d &bottomRight, Texture *texture) {
  OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);

  const float w = glTexture->width / (float) glTexture->internalWidth;
  const float h = glTexture->height / (float)glTexture->internalHeight;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glBindTexture(GL_TEXTURE_2D, glTexture->id);

  const GLfloat vertices[] = {
//  S   T         X                 Y                 Z
    0,  0,  -topLeft.x(),      topLeft.y(),      topLeft.z(),
    0,  h,  -bottomLeft.x(),   bottomLeft.y(),   bottomLeft.z(),
    w,  0,  -topRight.x(),     topRight.y(),     topRight.z(),
    w,  h,  -bottomRight.x(),  bottomRight.y(),  bottomRight.z(),
  };

  g_shaders.g_rect3d_shader->use();
  g_shaders.g_rect3d_shader->setUniform1f("verScale", 1.0f);
  g_shaders.g_rect3d_shader->setUniform1f("texScale", 1.0f);
  g_shaders.g_rect3d_shader->setUniform("mvpMatrix", g_shaders.g_mvpMatrix);
  glBindBuffer(GL_ARRAY_BUFFER, g_shaders.g_rect3dVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, 20 * sizeof(float), vertices);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisable(GL_BLEND);
}

Graphics::Surface *Renderer::getScreenshot() {
  Graphics::Surface *s = new Graphics::Surface();
  s->create(kOriginalWidth, kOriginalHeight, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

  glReadPixels(0, 0, kOriginalWidth, kOriginalHeight, GL_RGBA, GL_UNSIGNED_BYTE, s->getPixels());

  return s;
}

void Renderer::screenPosToDirection(const Common::Point screen, float &pitch, float &heading) {
  double x, y, z;

  x = screen.x;
  y = kOriginalHeight - screen.y;
  z = 0.9f;

  const Math::Vector2d tl = g_shaders.g_viewport.getTopLeft();
  x = 2 * double(x - tl.getX()) / g_shaders.g_viewport.getWidth() - 1.0f;
  y = 2 * double(y - tl.getY()) / g_shaders.g_viewport.getHeight() - 1.0f;
  z = 2 * z - 1.0f;

  // Screen coords to 3D coords
  Math::Vector4d point = Math::Vector4d(x, y, z, 1.0f);
  point = g_shaders.g_mvpMatrix * point;

  // 3D coords to polar coords
  Math::Vector3d v = Math::Vector3d(point.x(), point.y(), point.z());
  v.normalize();

  Math::Vector2d horizontalProjection = Math::Vector2d(v.x(), v.z());
  horizontalProjection.normalize();

  pitch = 90 - Math::Angle::arcCosine(v.y()).getDegrees();
  heading = Math::Angle::arcCosine(horizontalProjection.getY()).getDegrees();

  if (horizontalProjection.getX() > 0.0)
    heading = 360 - heading;
}

} // end of namespace Myst3

#endif
