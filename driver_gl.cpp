#include "driver_gl.h"		// Driver interface
#include "debug.h"		// error(), warning(), etc

Driver *g_driver;

// Hacky includes for temporary font rendering
#ifndef WIN32
 #include <GL/glx.h>
 #include <X11/Xlib.h>
#else
 #include <SDL_syswm.h>
 #include <windows.h>
#endif

// Constructor. Should create the driver and open screens, etc.
Driver::Driver(int screenW, int screenH, int screenBPP) {
  char  GLDriver[1024];

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  if (SDL_SetVideoMode(screenW, screenH, screenBPP, SDL_OPENGL) == 0)
    error("Could not initialize video");

  sprintf(GLDriver, "Residual: %s/%s", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
  SDL_WM_SetCaption(GLDriver, "Residual");

  // FIXME: Hacky temporary font renderer code
  hackFont = glGenLists(256);
  #ifdef WIN32
  {
        HDC   hDC;
        HFONT font;
        SDL_SysWMinfo wmi;
        SDL_VERSION(&wmi.version);
        SDL_GetWMInfo(&wmi);

        hDC = GetDC(wmi.window);
        font = CreateFont(0, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                          OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 0 ,
                          FF_DONTCARE|DEFAULT_PITCH, "Courier New");
        SelectObject(hDC, font);
        wglUseFontBitmaps(hDC, 0, 256, hackFont);
  }
  #else
  {
//        Display *dpy = XOpenDisplay(NULL);
//        XFontStruct *XFont = XLoadQueryFont(dpy, "-misc-fixed-medium-r-*-*-20-*-*-*-*-*-*-*" );
//        glXUseXFont(XFont->fid, 0, 256, hackFont);
//        XFreeFont(dpy, XFont);
//        XCloseDisplay(dpy);
  }
  #endif

}

void Driver::setupCamera(float fov, float nclip, float fclip, float roll) {
  // Set perspective transformation
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //  gluPerspective(std::atan(std::tan(fov_ / 2 * (M_PI/180)) * 0.75) * 2 * (180/M_PI), 4.0f / 3, nclip_, fclip_);

  float right = nclip * std::tan(fov / 2 * (M_PI/180));
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

  gluLookAt(pos.x(), pos.y(), pos.z(),
            interest.x(), interest.y(), interest.z(),
            up_vec.x(), up_vec.y(), up_vec.z());

}

void Driver::clearScreen() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Driver::flipBuffer() {
  SDL_GL_SwapBuffers();
}

void Driver::startActorDraw(Vector3d pos, float yaw, float pitch, float roll) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(pos.x(), pos.y(), pos.z());
    glRotatef(yaw, 0, 0, 1);
    glRotatef(pitch, 1, 0, 0);
    glRotatef(roll, 0, 1, 0);
}

void Driver::finishActorDraw() {
    glPopMatrix();
}

void Driver::drawDepthBitmap(int num, int x, int y, int w, int h, char **data) {
    if (num != 0) {
      warning("Animation not handled yet in GL texture path !\n");
    }

    glRasterPos2i(x, y);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_TRUE);

    /* This loop here is to prevent using PixelZoom that may be unoptimized for the 1.0 / -1.0 case
       in some drivers...
    */
    for (int row = 0; row < h; row++) {
      glRasterPos2i(x, y + row + 1);
//      glDrawPixels(w, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, data[num] + (2 * row * w));
    }
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthFunc(GL_LESS);
  }

void Driver::drawHackFont(int x, int y, const char *text, Color &fgColor) {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, 640, 480, 0, 0, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor3f(fgColor.red(), fgColor.green(), fgColor.blue());
  glRasterPos2i(x, y);

  glListBase(hackFont);
  glCallLists(strlen(strrchr(text, '/')) - 1, GL_UNSIGNED_BYTE, strrchr(text, '/') + 1);

  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
}

// drawSMUSHframe, used for quickly pushing full-screen images from cutscenes
void Driver::drawSMUSHframe(int offsetX, int offsetY, int _width, int _height, uint8 *_dst) {
        int num_tex_;
        GLuint *tex_ids_;

        // create texture
        num_tex_ = ((_width + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE) *
                                ((_height + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE);
        tex_ids_ = new GLuint[num_tex_];
        glGenTextures(num_tex_, tex_ids_);
        for (int i = 0; i < num_tex_; i++) {
                glBindTexture(GL_TEXTURE_2D, tex_ids_[i]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                                   BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0,
                                   GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
    }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, _width);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, _width);

        int cur_tex_idx = 0;
        for (int y = 0; y < _height; y += BITMAP_TEXTURE_SIZE) {
                for (int x = 0; x < _width; x += BITMAP_TEXTURE_SIZE) {
                        int width = (x + BITMAP_TEXTURE_SIZE >= _width) ? (_width - x) : BITMAP_TEXTURE_SIZE;
                        int height = (y + BITMAP_TEXTURE_SIZE >= _height) ? (_height - y) : BITMAP_TEXTURE_SIZE;
                        glBindTexture(GL_TEXTURE_2D, tex_ids_[cur_tex_idx]);
                        glTexSubImage2D(GL_TEXTURE_2D,
                                                        0,
                                                        0, 0,
                                                        width, height,
                                                        GL_RGB,
                                                        GL_UNSIGNED_SHORT_5_6_5,
                                                        _dst + (y * 2 * _width) + (2 * x));
                        cur_tex_idx++;
              }
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // prepare view
//        glMatrixMode(GL_PROJECTION);
  //      glLoadIdentity();
    //    glOrtho(0, 640, 480, 0, 0, 1);
 //       glMatrixMode(GL_MODELVIEW);
   //     glLoadIdentity();
     //   glMatrixMode(GL_TEXTURE);
       // glLoadIdentity();
        // A lot more may need to be put there : disabling Alpha test, blending, ...
        // For now, just keep this here :-)

//        glDisable(GL_LIGHTING);
  //      glEnable(GL_TEXTURE_2D);

        // draw
//        glDisable(GL_DEPTH_TEST);
//        glDepthMask(GL_FALSE);
//        glEnable(GL_SCISSOR_TEST);

        offsetY = 480 - offsetY - _height;
        cur_tex_idx = 0;
        for (int y = 0; y < _height; y += BITMAP_TEXTURE_SIZE) {
                for (int x = 0; x < _width; x += BITMAP_TEXTURE_SIZE) {
                        int width = (x + BITMAP_TEXTURE_SIZE >= _width) ? (_width - x) : BITMAP_TEXTURE_SIZE;
                        int height = (y + BITMAP_TEXTURE_SIZE >= _height) ? (_height - y) : BITMAP_TEXTURE_SIZE;
                        glBindTexture(GL_TEXTURE_2D, tex_ids_[cur_tex_idx]);
                        glScissor(x, 480 - (y + height), x + width, 480 - y);
                        glBegin(GL_QUADS);
                        glTexCoord2f(0, 0);

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
//        glDisable(GL_SCISSOR_TEST);
  //      glDisable(GL_TEXTURE_2D);
    //    glDepthMask(GL_TRUE);
  //      glEnable(GL_DEPTH_TEST);
    //    SDL_GL_SwapBuffers();

        // remove
        glDeleteTextures(num_tex_, tex_ids_);
        delete[] tex_ids_;
}
