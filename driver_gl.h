// Driver:
#include <SDL.h>
#include <SDL_opengl.h>

// Residual:
#include "bits.h"
#include "vector3d.h"
#include "color.h"

#define BITMAP_TEXTURE_SIZE 256

class Driver {
	public:
		Driver(int screenW, int screenH, int screenBPP);

		void setupCamera(float fov, float nclip, float fclip, float roll);
		void positionCamera(Vector3d pos, Vector3d interest);

		void clearScreen(); 
		void flipBuffer();

		void startActorDraw(Vector3d pos, float yaw, float pitch, float roll);
		void finishActorDraw();

		void drawDepthBitmap(int num, int x, int y, int w, int h, char **data);
		void drawBitmap();

		void drawHackFont(int x, int y, const char *text, Color &fgColor);
		void drawSMUSHframe(int offsetX, int offsetY, int _width, int _height, uint8 *_dst);

	private:
		GLuint hackFont;  // FIXME: Temporary font drawing hack

};

extern Driver *g_driver;

