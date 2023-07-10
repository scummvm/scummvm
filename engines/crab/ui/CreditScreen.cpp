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

#include "crab/ui/CreditScreen.h"
//#include "crab/url.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;

void CreditScreen::reset() {
	start.x = g_engine->_screenSettings->cur.w / 2 - 150;
	start.y = g_engine->_screenSettings->cur.h + 20;
	cur.x = start.x;
	speed.cur = speed.slow;
}

void CreditScreen::load(const Common::String &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("credits");

		if (nodeValid("bg", node))
			bg.load(node->first_node("bg"));

		if (nodeValid("h", node))
			heading.load(node->first_node("h"));

		if (nodeValid("p", node))
			paragraph.load(node->first_node("p"));

		if (nodeValid("logo", node))
			logo.load(node->first_node("logo"));

		if (nodeValid("website", node))
			website.load(node->first_node("website"), false);

		if (nodeValid("twitter", node))
			twitter.load(node->first_node("twitter"), false);

		if (nodeValid("twitter", node))
			back.load(node->first_node("back"));

		if (nodeValid("fast", node)) {
			rapidxml::xml_node<char> *fnode = node->first_node("fast");
			fast.load(fnode);
			loadNum(speed.fast, "val", fnode);
		}

		if (nodeValid("slow", node)) {
			rapidxml::xml_node<char> *snode = node->first_node("slow");
			slow.load(snode);
			loadNum(speed.slow, "val", snode);
		}

		if (nodeValid("reverse", node)) {
			rapidxml::xml_node<char> *rnode = node->first_node("reverse");
			reverse.load(rnode);
			loadNum(speed.reverse, "val", rnode);
		}

		speed.cur = speed.slow;

		if (nodeValid("pause", node))
			pause.load(node->first_node("pause"));

		if (nodeValid("text", node)) {
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

bool CreditScreen::HandleEvents(Common::Event &Event) {
	if (slow.HandleEvents(Event) == BUAC_LCLICK)
		speed.cur = speed.slow;
	else if (fast.HandleEvents(Event) == BUAC_LCLICK)
		speed.cur = speed.fast;
	else if (pause.HandleEvents(Event) == BUAC_LCLICK)
		speed.cur = 0.0f;
	else if (reverse.HandleEvents(Event) == BUAC_LCLICK)
		speed.cur = speed.reverse;

	return (back.HandleEvents(Event) == BUAC_LCLICK);
}

#if 0
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
#endif

void CreditScreen::draw() {
	bg.draw();

	slow.draw();
	fast.draw();
	pause.draw();
	reverse.draw();

	logo.draw();
	twitter.draw();
	website.draw();

	back.draw();

	cur.y = start.y;

	for (auto i = list.begin(); i != list.end(); ++i) {
		cur.y += paragraph.inc;

		if (i->heading) {
			cur.y += heading.inc;
			if (cur.y > -30 && cur.y < g_engine->_screenSettings->cur.h + 40) // Only draw text if it is actually visible on screen
				g_engine->_textManager->draw(cur.x, cur.y, i->text, heading.color, heading.font, heading.align);
		} else if (cur.y > -30 && cur.y < g_engine->_screenSettings->cur.h + 40)
			g_engine->_textManager->draw(cur.x, cur.y, i->text, paragraph.color, paragraph.font, paragraph.align);

		// If our cur value has reached below the screen, simply exit the loop as we won't draw anything else
		if (cur.y > g_engine->_screenSettings->cur.h + 40)
			break;
	}

	start.y -= speed.cur;

	// Sanity check so that we don't scroll too high or low
	if (start.y > g_engine->_screenSettings->cur.h + 40)
		start.y = g_engine->_screenSettings->cur.h + 40;
	else if (start.y < INT_MIN + 10)
		start.y = INT_MIN + 10;
}

void CreditScreen::setUI() {
	bg.setUI();
	back.setUI();

	slow.setUI();
	fast.setUI();
	pause.setUI();
	reverse.setUI();

	logo.setUI();
	twitter.setUI();
	website.setUI();
}

} // End of namespace Crab
