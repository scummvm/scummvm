#include "stdafx.h"
#include "Image.h"

using namespace pyrodactyl::image;

//------------------------------------------------------------------------
// Purpose: Load a texture
//------------------------------------------------------------------------
bool Image::Load(SDL_Surface *surface)
{
	//Create texture from surface pixels
	texture = SDL_CreateTextureFromSurface(gRenderer, surface);
	if (texture == NULL)
	{
		fprintf(stderr, "Unable to create texture from SDL_Surface! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	//Get image dimensions
	w = surface->w;
	h = surface->h;
	return true;
}

bool Image::Load(const std::string &path)
{
	//Get rid of preexisting texture
	Delete();

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		fprintf(stderr, "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Create texture from surface pixels
		texture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);

		if (texture == NULL)
			fprintf(stderr, "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		else
		{
			//Get image dimensions
			w = loadedSurface->w;
			h = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success

	return texture != NULL;
}

bool Image::Load(rapidxml::xml_node<char> *node, const char* name)
{
	if (node->first_attribute(name) != NULL)
		return Load(node->first_attribute(name)->value());

	return false;
}

//------------------------------------------------------------------------
// Purpose: Draw a texture to the screen without cropping
//------------------------------------------------------------------------
void Image::Draw(const int &x, const int &y, SDL_Rect *clip, const TextureFlipType &flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, w, h };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	switch (flip)
	{
	case FLIP_NONE:
		//Render to screen
		SDL_RenderCopy(gRenderer, texture, clip, &renderQuad);
		break;
	case FLIP_X:
		//Bottom right <-> Bottom left
		//Top right <-> Top left
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 0, NULL, SDL_FLIP_HORIZONTAL);
		break;
	case FLIP_Y:
		//Top right <-> Bottom right
		//Top left <-> Bottom left
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 0, NULL, SDL_FLIP_VERTICAL);
		break;
	case FLIP_XY:
		//Top right <-> Bottom left
		//Top left <-> Bottom right
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 180, NULL, SDL_FLIP_NONE);
		break;
	case FLIP_D:
		//Top left <-> Bottom right
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 270, NULL, SDL_FLIP_HORIZONTAL);
		break;
	case FLIP_DX:
		//Rotate 90 degrees clockwise
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 90, NULL, SDL_FLIP_NONE);
		break;
	case FLIP_DY:
		//Rotate 270 degrees clockwise
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 270, NULL, SDL_FLIP_NONE);
		break;
	case FLIP_XYD:
		//This is just flipping across the diagonal, instead of the anti-diagonal
		//Top right <-> Bottom left
		SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, 270, NULL, SDL_FLIP_VERTICAL);
		break;
	default: break;
	}
}

//------------------------------------------------------------------------
// Purpose: Delete texture data
//------------------------------------------------------------------------
void Image::Delete()
{
	if (texture != nullptr)
	{
		SDL_DestroyTexture(texture);
		texture = nullptr;
		w = 0;
		h = 0;
	}
}