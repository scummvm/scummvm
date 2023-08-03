#include "stdafx.h"
#include "ScreenSettings.h"

//Our screen settings object
ScreenSettings gScreenSettings;

//Our global renderer
SDL_Renderer *gRenderer = nullptr;

//Our global window object
SDL_Window *gWindow = nullptr;

void ScreenSettings::Load(rapidxml::xml_node<char> *node)
{
	LoadNum(cur.w, "w", node);
	LoadNum(cur.h, "h", node);
	LoadNum(fps, "fps", node);
	LoadNum(gamma, "gamma", node);
	LoadNum(text_speed, "text_speed", node);

	LoadBool(vsync, "vsync", node);
	LoadBool(border, "border", node);
	LoadBool(fullscreen, "fullscreen", node);
	LoadBool(save_on_exit, "save_on_exit", node);
	LoadBool(mouse_trap, "mouse_trap", node);
	LoadBool(quality, "quality", node);

	SetVideoFlags();
	CreateBackup();
}

void ScreenSettings::SetVideoFlags()
{
	if (fullscreen)
	{
		//Window border doesn't matter in fullscreen mode
		videoflags = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN;
	}
	else
	{
		if (border)
			videoflags = SDL_WINDOW_SHOWN;
		else
			videoflags = SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS;
	}
}

void ScreenSettings::CreateBackup()
{
	backup = *this;
}

void ScreenSettings::RestoreBackup()
{
	fps = backup.fps;
	cur = backup.cur;
	fullscreen = backup.fullscreen;
	border = backup.border;
	vsync = backup.vsync;
	gamma = backup.gamma;
	videoflags = backup.videoflags;
	save_on_exit = backup.save_on_exit;
	quality = backup.quality;
	mouse_trap = backup.mouse_trap;
	text_speed = backup.text_speed;
}

void ScreenSettings::SetResolution()
{
	//Check the current window flags
	Uint32 flags = SDL_GetWindowFlags(gWindow);

	//Are we in fullscreen mode right now?
	if ((flags & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN)
	{
		//Switch to windowed mode when changing resolution, then switch back once it is changed
		SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_SHOWN);
		SDL_SetWindowSize(gWindow, cur.w, cur.h);
		SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN);
	}
	else
	{
		SDL_SetWindowSize(gWindow, cur.w, cur.h);
		SDL_SetWindowPosition(gWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	change_interface = true;
}

void ScreenSettings::SetFullscreen()
{
	SetVideoFlags();
	SDL_SetWindowFullscreen(gWindow, videoflags);
}

void ScreenSettings::SetWindowBorder()
{
	if (border)
		SDL_SetWindowBordered(gWindow, SDL_TRUE);
	else
		SDL_SetWindowBordered(gWindow, SDL_FALSE);
}

void ScreenSettings::SetVsync()
{
	if (vsync)
		SDL_SetHint("SDL_HINT_RENDER_VSYNC", "1");
	else
		SDL_SetHint("SDL_HINT_RENDER_VSYNC", "0");
}

void ScreenSettings::SetMouseTrap()
{
	if (mouse_trap)
		SDL_SetRelativeMouseMode(SDL_TRUE);
	else
		SDL_SetRelativeMouseMode(SDL_FALSE);
}

void ScreenSettings::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root)
{
	root->append_attribute(doc.allocate_attribute("version", gStrPool.Get(version)));

	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "screen");
	child->append_attribute(doc.allocate_attribute("w", gStrPool.Get(cur.w)));
	child->append_attribute(doc.allocate_attribute("h", gStrPool.Get(cur.h)));
	child->append_attribute(doc.allocate_attribute("fps", gStrPool.Get(fps)));

	child->append_attribute(doc.allocate_attribute("gamma", gStrPool.FGet(gamma)));
	child->append_attribute(doc.allocate_attribute("text_speed", gStrPool.FGet(text_speed)));

	SaveBool(vsync, "vsync", doc, child);
	SaveBool(border, "border", doc, child);
	SaveBool(fullscreen, "fullscreen", doc, child);
	SaveBool(save_on_exit, "save_on_exit", doc, child);
	SaveBool(quality, "quality", doc, child);
	SaveBool(mouse_trap, "mouse_trap", doc, child);

	root->append_node(child);
	CreateBackup();
}