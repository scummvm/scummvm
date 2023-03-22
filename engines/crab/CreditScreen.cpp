/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "CreditScreen.h"
#include "url.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;

void CreditScreen::Reset() {
	start.x = gScreenSettings.cur.w / 2 - 150;
	start.y = gScreenSettings.cur.h + 20;
	cur.x = start.x;
	speed.cur = speed.slow;
}

void CreditScreen::Load(const std::string &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("credits");

		if (NodeValid("bg", node))
			bg.Load(node->first_node("bg"));

		if (NodeValid("h", node))
			heading.Load(node->first_node("h"));

		if (NodeValid("p", node))
			paragraph.Load(node->first_node("p"));

		if (NodeValid("logo", node))
			logo.Load(node->first_node("logo"));

		if (NodeValid("website", node))
			website.Load(node->first_node("website"), false);

		if (NodeValid("twitter", node))
			twitter.Load(node->first_node("twitter"), false);

		if (NodeValid("twitter", node))
			back.Load(node->first_node("back"));

		if (NodeValid("fast", node)) {
			rapidxml::xml_node<char> *fnode = node->first_node("fast");
			fast.Load(fnode);
			LoadNum(speed.fast, "val", fnode);
		}

		if (NodeValid("slow", node)) {
			rapidxml::xml_node<char> *snode = node->first_node("slow");
			slow.Load(snode);
			LoadNum(speed.slow, "val", snode);
		}

		if (NodeValid("reverse", node)) {
			rapidxml::xml_node<char> *rnode = node->first_node("reverse");
			reverse.Load(rnode);
			LoadNum(speed.reverse, "val", rnode);
		}

		speed.cur = speed.slow;

		if (NodeValid("pause", node))
			pause.Load(node->first_node("pause"));

		if (NodeValid("text", node)) {
			rapidxml::xml_node<char> *tnode = node->first_node("text");
			for (rapidxml::xml_node<char> *n = tnode->first_node(); n != NULL; n = n->next_sibling()) {
				CreditText t;
				t.text = n->value();
				t.heading = (n->name()[0] == 'h');
				list.push_back(t);
			}
		}
	}
}

bool CreditScreen::HandleEvents(SDL_Event &Event) {
	if (slow.HandleEvents(Event) == BUAC_LCLICK)
		speed.cur = speed.slow;
	else if (fast.HandleEvents(Event) == BUAC_LCLICK)
		speed.cur = speed.fast;
	else if (pause.HandleEvents(Event) == BUAC_LCLICK)
		speed.cur = 0.0f;
	else if (reverse.HandleEvents(Event) == BUAC_LCLICK)
		speed.cur = speed.reverse;

	if (website.HandleEvents(Event))
		OpenURL("http://pyrodactyl.com");
	else if (twitter.HandleEvents(Event))
		OpenURL("https://www.twitter.com/pyrodactylgames");

	return (back.HandleEvents(Event) == BUAC_LCLICK);
}

void CreditScreen::Draw() {
	bg.Draw();

	slow.Draw();
	fast.Draw();
	pause.Draw();
	reverse.Draw();

	logo.Draw();
	twitter.Draw();
	website.Draw();

	back.Draw();

	cur.y = start.y;

	for (auto i = list.begin(); i != list.end(); ++i) {
		cur.y += paragraph.inc;

		if (i->heading) {
			cur.y += heading.inc;
			if (cur.y > -30 && cur.y < gScreenSettings.cur.h + 40) // Only draw text if it is actually visible on screen
				gTextManager.Draw(cur.x, cur.y, i->text, heading.color, heading.font, heading.align);
		} else if (cur.y > -30 && cur.y < gScreenSettings.cur.h + 40)
			gTextManager.Draw(cur.x, cur.y, i->text, paragraph.color, paragraph.font, paragraph.align);

		// If our cur value has reached below the screen, simply exit the loop as we won't draw anything else
		if (cur.y > gScreenSettings.cur.h + 40)
			break;
	}

	start.y -= speed.cur;

	// Sanity check so that we don't scroll too high or low
	if (start.y > gScreenSettings.cur.h + 40)
		start.y = gScreenSettings.cur.h + 40;
	else if (start.y < INT_MIN + 10)
		start.y = INT_MIN + 10;
}

void CreditScreen::SetUI() {
	bg.SetUI();
	back.SetUI();

	slow.SetUI();
	fast.SetUI();
	pause.SetUI();
	reverse.SetUI();

	logo.SetUI();
	twitter.SetUI();
	website.SetUI();
}
