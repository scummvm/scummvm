#include "SpriteConstant.h"
#include "stdafx.h"

using namespace pyrodactyl::ai;

FlyerConstant::FlyerConstant() : start(10, 40), vel(8.0f, 0.0f) {
	delay_min = 5000;
	delay_max = 20000;
}

void FlyerConstant::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid("start", node))
		start.Load(node->first_node("start"));

	if (NodeValid("vel", node))
		vel.Load(node->first_node("vel"));

	if (NodeValid("delay", node)) {
		auto n = node->first_node("delay");
		LoadNum(delay_min, "min", n);
		LoadNum(delay_max, "max", n);
	}
}

SpriteConstant::SpriteConstant() : walk_vel_mod(0.9f, 0.63f) {
	plane_w = 20;
	tweening = 0.2f;
}

void SpriteConstant::Load(rapidxml::xml_node<char> *node) {
	LoadNum(plane_w, "plane_width", node);
	LoadNum(tweening, "tweening", node);

	if (NodeValid("walk_vel_mod", node))
		walk_vel_mod.Load(node->first_node("walk_vel_mod"));

	if (NodeValid("fly", node))
		fly.Load(node->first_node("fly"));
}