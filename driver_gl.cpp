// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "debug.h"
#include "colormap.h"
#include "material.h"
#include "driver_gl.h"
#include "font.h"

Driver *g_driver;

// Constructor. Should create the driver and open screens, etc.
Driver::Driver(int screenW, int screenH, int screenBPP) {
	char GLDriver[1024];

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	if (SDL_SetVideoMode(screenW, screenH, screenBPP, SDL_OPENGL) == 0)
		error("Could not initialize video");

	sprintf(GLDriver, "Residual: %s/%s", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
	SDL_WM_SetCaption(GLDriver, "Residual");

	// Load emergency built-in font
	loadEmergFont();

	_smushNumTex = 0;
}

void Driver::setupCamera(float fov, float nclip, float fclip, float roll) {
	// Set perspective transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(std::atan(std::tan(fov_ / 2 * (M_PI / 180)) * 0.75) * 2 * (180 / M_PI), 4.0f / 3, nclip_, fclip_);

	float right = nclip * std::tan(fov / 2 * (M_PI / 180));
	glFrustum(-right, right, -right * 0.75, right * 0.75, nclip, fclip);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Vector3d up_vec(0, 0, 1);
	glRotatef(roll, 0, 0, -1);
}

void Driver::positionCamera(Vector3d pos, Vector3d interest) {
	Vector3d up_vec(0, 0, 1);

	if (pos.x() == interest.x() && pos.y() == interest.y())
		up_vec = Vector3d(0, 1, 0);

	gluLookAt(pos.x(), pos.y(), pos.z(), interest.x(), interest.y(), interest.z(), up_vec.x(), up_vec.y(), up_vec.z());
}

void Driver::clearScreen() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Driver::flipBuffer() {
	SDL_GL_SwapBuffers();
}

void Driver::startActorDraw(Vector3d pos, float yaw, float pitch, float roll) {
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(pos.x(), pos.y(), pos.z());
	glRotatef(yaw, 0, 0, 1);
	glRotatef(pitch, 1, 0, 0);
	glRotatef(roll, 0, 1, 0);
}

void Driver::finishActorDraw() {
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}


void Driver::set3DMode() {
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
}

void Driver::drawModelNodeDebug(const Model::Mesh *model) {
	// debug
	// this draw the model node in red

	GLdouble modelView[500];
	GLdouble projection[500];
	GLint viewPort[500];

	glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewPort);

	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glPointSize(3.f);
	glColor4f(1.f, 0.f, 0.f, 1.f);
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_POINTS);
	glVertex3f(model->_matrix._pos.x(), model->_matrix._pos.y(), model->_matrix._pos.z());
	glEnd();

	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

void Driver::drawModelPolygonPointsDebug(const Model::Mesh *model) {
	// debug
	// this draw the poly points

	GLdouble modelView[500];
	GLdouble projection[500];
	GLint viewPort[500];
	int i, j;

	glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewPort);

	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glPointSize(3.f);
	glColor4f(0.f, 1.f, 0.f, 1.f);
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_POINTS);

	for (i = 0; i < model->_numFaces; i++) {
		Vector3d v;
		Matrix4 tempMatrix = model->_matrix;
		float* pVertices;

		for(j = 0; j < model->_faces[i]._numVertices; j++ ) {
			pVertices = model->_vertices + 3 * model->_faces[i]._vertices[j];

			v.set(*(pVertices), *(pVertices + 1), *(pVertices + 2));

//			tempMatrix._rot.transform(&v);
			v += tempMatrix._pos;

			glVertex3f(v.x(), v.y(), v.z());
		}
	}

	glEnd();
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

void Driver::drawModelFace(const Model::Face *face, float *vertices, float *vertNormals, float *textureVerts) {
	glNormal3fv(face->_normal._coords);
	glBegin(GL_POLYGON);
	for (int i = 0; i < face->_numVertices; i++) {
		glNormal3fv(vertNormals + 3 * face->_vertices[i]);
		if (face->_texVertices != NULL)
			glTexCoord2fv(textureVerts + 2 * face->_texVertices[i]);
		glVertex3fv(vertices + 3 * face->_vertices[i]);
	}
	glEnd();
}

void Driver::drawHierachyNode(const Model::HierNode *node) {
	if (node->_hierVisible) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		glTranslatef(node->_animPos.x() / node->_totalWeight, node->_animPos.y() / node->_totalWeight, node->_animPos.z() / node->_totalWeight);
		glRotatef(node->_animYaw / node->_totalWeight, 0, 0, 1);
		glRotatef(node->_animPitch / node->_totalWeight, 1, 0, 0);
		glRotatef(node->_animRoll / node->_totalWeight, 0, 1, 0);

		if (node->_mesh != NULL && node->_meshVisible) {
			glPushMatrix();
			glTranslatef(node->_pivot.x(), node->_pivot.y(), node->_pivot.z());
			node->_mesh->draw();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		}
		if (node->_child != NULL) {
			node->_child->draw();
			glMatrixMode(GL_MODELVIEW);
		}
		glPopMatrix();
	}

	if (node->_sibling != NULL)
		node->_sibling->draw();
}

void Driver::updateHierachyNode1(const Model::HierNode *node) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef(node->_animPos.x() / node->_totalWeight, node->_animPos.y() / node->_totalWeight, node->_animPos.z() / node->_totalWeight);
	glRotatef(node->_animYaw / node->_totalWeight, 0, 0, 1);
	glRotatef(node->_animPitch / node->_totalWeight, 1, 0, 0);
	glRotatef(node->_animRoll / node->_totalWeight, 0, 1, 0);
}

void Driver::updateHierachyNode2(const Model::HierNode *node) {
	if (node->_mesh != NULL) {
		glPushMatrix();
		glTranslatef(node->_pivot.x(), node->_pivot.y(), node->_pivot.z());
		node->_mesh->_matrix = node->_pivotMatrix;
		node->_mesh->update();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	if (node->_child != NULL ) {
		node->_child->setMatrix(node->_matrix);
		node->_child->update();
		glMatrixMode(GL_MODELVIEW);
	}

	glPopMatrix();
}

void Driver::createBitmap(Bitmap *bitmap) {
	if (bitmap->_format == 1) {
		bitmap->_hasTransparency = false;
		bitmap->_numTex = ((bitmap->_width + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE) *
			((bitmap->_height + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE);
		bitmap->_texIds = new GLuint[bitmap->_numTex * bitmap->_numImages];
		glGenTextures(bitmap->_numTex * bitmap->_numImages, bitmap->_texIds);

		byte *texData = new byte[4 * bitmap->_width * bitmap->_height];

		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			// Convert data to 32-bit RGBA format
			byte *texDataPtr = texData;
			uint16 *bitmapData = reinterpret_cast<uint16 *>(bitmap->_data[pic]);
			for (int i = 0; i < bitmap->_width * bitmap->_height; i++, texDataPtr += 4, bitmapData++) {
				uint16 pixel = *bitmapData;
				int r = pixel >> 11;
				texDataPtr[0] = (r << 3) | (r >> 2);
				int g = (pixel >> 5) & 0x3f;
				texDataPtr[1] = (g << 2) | (g >> 4);
				int b = pixel & 0x1f;
				texDataPtr[2] = (b << 3) | (b >> 2);
				if (pixel == 0xf81f) { // transparent
					texDataPtr[3] = 0;
					bitmap->_hasTransparency = true;
				} else {
					texDataPtr[3] = 255;
				}
			}

			for (int i = 0; i < bitmap->_numTex; i++) {
				glBindTexture(GL_TEXTURE_2D, bitmap->_texIds[bitmap->_numTex * pic + i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			}

			glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, bitmap->_width);

			int cur_tex_idx = bitmap->_numTex * pic;

			for (int y = 0; y < bitmap->_height; y += BITMAP_TEXTURE_SIZE) {
				for (int x = 0; x < bitmap->_width; x += BITMAP_TEXTURE_SIZE) {
					int width  = (x + BITMAP_TEXTURE_SIZE >= bitmap->_width)  ? (bitmap->_width  - x) : BITMAP_TEXTURE_SIZE;
					int height = (y + BITMAP_TEXTURE_SIZE >= bitmap->_height) ? (bitmap->_height - y) : BITMAP_TEXTURE_SIZE;
					glBindTexture(GL_TEXTURE_2D, bitmap->_texIds[cur_tex_idx]);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
						texData + (y * 4 * bitmap->_width) + (4 * x));
					cur_tex_idx++;
				}
			}
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		delete [] texData;
	} else {
		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			uint16 *zbufPtr = reinterpret_cast<uint16 *>(bitmap->_data[pic]);
			for (int i = 0; i < (bitmap->_width * bitmap->_height); i++) {
				uint16 val = READ_LE_UINT16(bitmap->_data[pic] + 2 * i);
				zbufPtr[i] = 0xffff - ((uint32) val) * 0x10000 / 100 / (0x10000 - val);
			}

			// Flip the zbuffer image to match what GL expects
			for (int y = 0; y < bitmap->_height / 2; y++) {
				uint16 *ptr1 = zbufPtr + y * bitmap->_width;
				uint16 *ptr2 = zbufPtr + (bitmap->_height - 1 - y) * bitmap->_width;
				for (int x = 0; x < bitmap->_width; x++, ptr1++, ptr2++) {
					uint16 tmp = *ptr1;
					*ptr1 = *ptr2;
					*ptr2 = tmp;
				}
			}
		}
		bitmap->_texIds = NULL;
	}
}

void Driver::drawBitmap(const Bitmap *bitmap) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	// A lot more may need to be put there : disabling Alpha test, blending, ...
	// For now, just keep this here :-)
	if (bitmap->_format == 1 && bitmap->_hasTransparency) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else
		glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	if (bitmap->_format == 1) { // Normal image
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glEnable(GL_SCISSOR_TEST);
		glScissor(bitmap->_x, 480 - (bitmap->_y + bitmap->_height), bitmap->_width, bitmap->_height);
		int cur_tex_idx = bitmap->_numTex * (bitmap->_currImage - 1);
		for (int y = bitmap->_y; y < (bitmap->_y + bitmap->_height); y += BITMAP_TEXTURE_SIZE) {
			for (int x = bitmap->_x; x < (bitmap->_x + bitmap->_width); x += BITMAP_TEXTURE_SIZE) {
				glBindTexture(GL_TEXTURE_2D, bitmap->_texIds[cur_tex_idx]);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);
				glVertex2i(x, y);
				glTexCoord2f(1.0, 0.0);
				glVertex2i(x + BITMAP_TEXTURE_SIZE, y);
				glTexCoord2f(1.0, 1.0);
				glVertex2i(x + BITMAP_TEXTURE_SIZE, y + BITMAP_TEXTURE_SIZE);
				glTexCoord2f(0.0, 1.0);
				glVertex2i(x, y + BITMAP_TEXTURE_SIZE);
				glEnd();
				cur_tex_idx++;
			}
		}

		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	} else if (bitmap->_format == 5) {	// ZBuffer image
		// Only draw the manual zbuffer when enabled
		if (ZBUFFER_GLOBAL)
			g_driver->drawDepthBitmap(bitmap->_x, bitmap->_y, bitmap->_width, bitmap->_height, bitmap->_data[bitmap->_currImage - 1]);
	}
}

void Driver::destroyBitmap(Bitmap *bitmap) {
	if (bitmap->_texIds) {
		glDeleteTextures(bitmap->_numTex * bitmap->_numImages, bitmap->_texIds);
		delete[] bitmap->_texIds;
	}
}

void Driver::createMaterial(Material *material, const char *data, const CMap *cmap) {
	material->_textures = new GLuint[material->_numImages];
	glGenTextures(material->_numImages, material->_textures);
	char *texdata = new char[material->_width * material->_height * 4];
	for (int i = 0; i < material->_numImages; i++) {
		char *texdatapos = texdata;
		for (int y = 0; y < material->_height; y++) {
			for (int x = 0; x < material->_width; x++) {
				int col = *(uint8 *)(data);
				if (col == 0)
					memset(texdatapos, 0, 4); // transparent
				else {
					memcpy(texdatapos, cmap->_colors + 3 * (*(uint8 *)(data)), 3);
					texdatapos[3] = '\xff'; // fully opaque
				}
				texdatapos += 4;
				data++;
			}
		}
		glBindTexture(GL_TEXTURE_2D, material->_textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, material->_width, material->_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdata);
		data += 24;
	}
	delete[] texdata;
}

void Driver::selectMaterial(const Material *material) {
	glBindTexture(GL_TEXTURE_2D, material->_textures[material->_currImage]);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef(1.0f / material->_width, 1.0f / material->_height, 1);
}

void Driver::destroyMaterial(Material *material) {
	glDeleteTextures(material->_numImages, material->_textures);
	delete[] material->_textures;
}

void Driver::drawDepthBitmap(int x, int y, int w, int h, char *data) {
	//	if (num != 0) {
	//		warning("Animation not handled yet in GL texture path !\n");
	//	}

	if (y + h == 480) {
		
		//For some mysterious reason residual crashes
		//on this glRasterPos2i(...) call on Mac.
		//However, if we issue some dummy gl* calls beforehand
		//(either this or some nonsensical state change) then
		//everything works fine... how odd.
		//Use this workaround for now.

		#ifdef MACOSX
			glBegin(GL_POINTS);
			glEnd();
		#endif
		
		glRasterPos2i(x, 479);
		glBitmap(0, 0, 0, 0, 0, -1, NULL);
	} else
		glRasterPos2i(x, y + h);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	glDrawPixels(w, h, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthFunc(GL_LESS);
}

void Driver::prepareSmushFrame(int width, int height, byte *bitmap) {
	// remove if already exist
	if (_smushNumTex > 0) {
		glDeleteTextures(_smushNumTex, _smushTexIds);
		delete[] _smushTexIds;
		_smushNumTex = 0;
	}

	// create texture
	_smushNumTex = ((width + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE) *
		((height + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE);
	_smushTexIds = new GLuint[_smushNumTex];
	glGenTextures(_smushNumTex, _smushTexIds);
	for (int i = 0; i < _smushNumTex; i++) {
		glBindTexture(GL_TEXTURE_2D, _smushTexIds[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

	int curTexIdx = 0;
	for (int y = 0; y < height; y += BITMAP_TEXTURE_SIZE) {
		for (int x = 0; x < width; x += BITMAP_TEXTURE_SIZE) {
			int t_width = (x + BITMAP_TEXTURE_SIZE >= width) ? (width - x) : BITMAP_TEXTURE_SIZE;
			int t_height = (y + BITMAP_TEXTURE_SIZE >= height) ? (height - y) : BITMAP_TEXTURE_SIZE;
			glBindTexture(GL_TEXTURE_2D, _smushTexIds[curTexIdx]);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, t_width, t_height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, bitmap + (y * 2 * width) + (2 * x));
			curTexIdx++;
		}
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	_smushWidth = width;
	_smushHeight = height;
}

void Driver::drawSmushFrame(int offsetX, int offsetY) {
	// prepare view
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	// A lot more may need to be put there : disabling Alpha test, blending, ...
	// For now, just keep this here :-)

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	// draw
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_SCISSOR_TEST);

	glScissor(offsetX, 480 - (offsetY + _smushHeight), _smushWidth, _smushHeight);

	int curTexIdx = 0;
	for (int y = 0; y < _smushHeight; y += BITMAP_TEXTURE_SIZE) {
		for (int x = 0; x < _smushWidth; x += BITMAP_TEXTURE_SIZE) {
			int t_width = (x + BITMAP_TEXTURE_SIZE >= _smushWidth) ? (_smushWidth - x) : BITMAP_TEXTURE_SIZE;
			int t_height = (y + BITMAP_TEXTURE_SIZE >= _smushHeight) ? (_smushHeight - y) : BITMAP_TEXTURE_SIZE;
			glBindTexture(GL_TEXTURE_2D, _smushTexIds[curTexIdx]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(x + offsetX, y + offsetY);
			glTexCoord2f(1.0, 0.0);
			glVertex2i(x + offsetX + BITMAP_TEXTURE_SIZE, y + offsetY);
			glTexCoord2f(1.0, 1.0);
			glVertex2i(x + offsetX + BITMAP_TEXTURE_SIZE, y + offsetY + BITMAP_TEXTURE_SIZE);
			glTexCoord2f(0.0, 1.0);
			glVertex2i(x + offsetX, y + offsetY + BITMAP_TEXTURE_SIZE);
			glEnd();
			curTexIdx++;
		}
	}

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

// Load emergency font
void Driver::loadEmergFont() {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	_emergFont = glGenLists(128);
	for (int i = 32; i < 127; i++) {
		glNewList(_emergFont + i, GL_COMPILE);
		glBitmap(8, 13, 0, 2, 10, 0, font[i - 32]);
		glEndList();
	}
}

// Draw text string using emergency font
void Driver::drawEmergString(int x, int y, const char *text, const Color &fgColor) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);

	glColor3f(fgColor.red(), fgColor.green(), fgColor.blue());
	glRasterPos2i(x, y);

	glListBase(_emergFont);
	//glCallLists(strlen(strrchr(text, '/')) - 1, GL_UNSIGNED_BYTE, strrchr(text, '/') + 1);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, (GLubyte *) text);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}
