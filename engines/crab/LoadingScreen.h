#pragma once

#include "common_header.h"
#include "vectors.h"
#include "timer.h"
#include "Image.h"

class LoadingScreen
{
	struct Screen
	{
		//The background image
		pyrodactyl::image::Image bg;

		Screen(rapidxml::xml_node<char> *node) { bg.Load(node, "bg"); }
		void Delete() { bg.Delete(); }
		void Draw(){ bg.Draw((gScreenSettings.cur.w - bg.W()) / 2, (gScreenSettings.cur.h - bg.H()) / 2); }
	};

	//The different loading screens
	std::vector<Screen> screen;

	//The current loading screen
	unsigned int cur;

	//The text image (says loading)
	pyrodactyl::image::Image text;

public:
	LoadingScreen(){ cur = 0; };
	~LoadingScreen(){};

	void Change(){ cur = gRandom.Num() % screen.size(); }

	void Load();
	void Draw();
	void Dim();
	void Quit();
};

extern LoadingScreen gLoadScreen;