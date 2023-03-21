#include "stdafx.h"
#include "slider.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;

void Slider::Load(rapidxml::xml_node<char> * node, const int &Min, const int &Max, const int &Val)
{
	if (NodeValid(node))
	{
		knob.Load(node->first_node("knob"), false);
		bar.Load(node->first_node("bar"));

		knob.y = bar.y;
		knob.w = gImageManager.GetTexture(knob.img.normal).W();
		knob.h = gImageManager.GetTexture(knob.img.normal).H();
		knob.canmove = true;

		min = Min;
		max = Max;
		Value(Val);

		caption.Load(node->first_node("caption"), &bar);
	}

	CreateBackup();
}

bool Slider::HandleEvents(const SDL_Event &Event)
{
	//A person is moving the knob
	if (knob.HandleEvents(Event) == BUAC_GRABBED)
	{
		int dx = gMouse.motion.x - bar.x;

		if (dx < 0) dx = 0;
		else if (dx >(bar.w - knob.w)) dx = (bar.w - knob.w);

		knob.x = bar.x + dx;
		knob.y = bar.y;

		value = min + (((max - min) * (knob.x - bar.x)) / (bar.w - knob.w));
		return true;
	}

	//If a person clicks on the slider bar, the knob needs to travel there
	if (Event.type == SDL_MOUSEBUTTONDOWN && bar.Contains(gMouse.button.x, gMouse.button.y))
	{
		knob.x = gMouse.button.x;
		knob.y = bar.y;

		value = min + (((max - min) * (knob.x - bar.x)) / (bar.w - knob.w));
		return true;
	}

	return false;
}

void Slider::Draw()
{
	bar.Draw();
	caption.Draw(false);
	knob.Draw();
}

void Slider::Value(const int val)
{
	value = val;

	if (value < min) value = min;
	else if (value > max) value = max;

	knob.x = bar.x + ((bar.w - knob.w) * (value - min)) / (max - min);
}

void Slider::SetUI()
{
	bar.SetUI();
	knob.SetUI();
	caption.SetUI(&bar);

	knob.x = bar.x + ((bar.w - knob.w) * value / (max - min));
	knob.y = bar.y;
	knob.w = gImageManager.GetTexture(knob.img.normal).W();
	knob.h = gImageManager.GetTexture(knob.img.normal).H();
}