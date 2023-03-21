#pragma once

#include "common_header.h"
#include "Line.h"
#include "vectors.h"

//------------------------------------------------------------------------
// Purpose: A simple rectangle class
//------------------------------------------------------------------------
class Rect : public SDL_Rect
{
public:

	Rect(int X = 0, int Y = 0, int W = 0, int H = 0){ x = X; y = Y; w = W; h = H; }

	bool Load(rapidxml::xml_node<char> *node, const bool &echo = true, const std::string &x_name = "x",
		const std::string &y_name = "y", const std::string &w_name = "w", const std::string &h_name = "h");

	//Is this rectangle colliding with another rectangle?
	bool Collide(Rect box);

	//Resolving a collision means we need to correct the position of the target rectangle to just outside of the reference rectangle
	//To do this, first we need to determine which edges of the rectangles are colliding

	//Resolve a collision in the X plane
	Direction ResolveX(Rect collider);

	//Resolve a collision in the Y plane
	Direction ResolveY(Rect collider);

	//Does this rectangle contain a point?
	bool Contains(int X, int Y){ return (X > x && X < x + w && Y > y && Y < y + h); }
	bool Contains(Vector2i v){ return Contains(v.x, v.y); }

	//Does this rectangle contain another rectangle?
	bool Contains(Rect box){ return (x < box.x && x + w > box.x + box.w && y < box.y && y + h > box.y + box.h); }

	//Flip a rectangle with respect to an axis
	void Flip(const TextureFlipType &flip, const Vector2i &axis);

	//Draw the borders of the rectangle
	void Draw(const int &XOffset = 0, const int &YOffset = 0, const Uint8 &r = 0, const Uint8 &g = 0, const Uint8 &b = 0, const Uint8 &a = 255);

	//Check if a rectangle is the same as another
	bool operator==(const Rect &r) { return r.x == x && r.y == y && r.w == w && r.h == h; }

	//Save to file
	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char* name);
};
