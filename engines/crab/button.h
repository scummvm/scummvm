//=============================================================================
// Author:   Arvind
// Purpose:  Button class
//=============================================================================
#pragma once

#include "Caption.h"
#include "HoverInfo.h"
#include "ImageManager.h"
#include "MusicManager.h"
#include "TextManager.h"
#include "common_header.h"
#include "element.h"
#include "hotkey.h"

namespace pyrodactyl {
namespace ui {
enum ButtonAction { BUAC_IGNORE,
					BUAC_LCLICK,
					BUAC_RCLICK,
					BUAC_GRABBED };

struct ButtonImage {
	ImageKey normal, select, hover;

	bool operator==(const ButtonImage &img) { return normal == img.normal && select == img.select && hover == img.hover; }

	ButtonImage() {
		normal = 0;
		select = 0;
		hover = 0;
	}

	void Load(rapidxml::xml_node<char> *node, const bool &echo = true) {
		if (NodeValid(node)) {
			LoadImgKey(normal, "img_b", node, echo);
			LoadImgKey(select, "img_s", node, echo);
			LoadImgKey(hover, "img_h", node, echo);
		}
	}

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
		root->append_attribute(doc.allocate_attribute("img_b", gStrPool.Get(normal)));
		root->append_attribute(doc.allocate_attribute("img_s", gStrPool.Get(select)));
		root->append_attribute(doc.allocate_attribute("img_h", gStrPool.Get(hover)));
	}
};

class Button : public Element {
public:
	bool visible, mousepressed;

	// We need to keep track of keyboard and mouse hovering separately
	bool hover_mouse, hover_key, hover_prev;

	// Can the player move this button?
	bool canmove;

	// The button images
	ButtonImage img;

	// The sound effect played when button is clicked
	pyrodactyl::music::ChunkKey se_click, se_hover;

	// Text shown when mouse is hovered over the button
	HoverInfo tooltip;

	// Text shown all times on the button
	Caption caption;

	// A hotkey is a keyboard key(s) that are equivalent to pressing a button
	pyrodactyl::input::HotKey hotkey;

	Button();
	~Button() {}
	void Reset();
	void SetUI(Rect *parent = NULL);

	void Load(rapidxml::xml_node<char> *node, const bool &echo = true);
	void Init(const Button &ref, const int &XOffset = 0, const int &YOffset = 0);

	void Img(Button &b) { img = b.img; }
	void Img(ButtonImage &img) { this->img = img; }
	ButtonImage Img() { return img; }

	void Draw(const int &XOffset = 0, const int &YOffset = 0, Rect *clip = NULL);
	ButtonAction HandleEvents(const SDL_Event &Event, const int &XOffset = 0, const int &YOffset = 0);

	// Special functions to only draw parts of a button (used in special situations like world map)
	void ImageCaptionOnlyDraw(const int &XOffset = 0, const int &YOffset = 0, Rect *clip = NULL);
	void HoverInfoOnlyDraw(const int &XOffset = 0, const int &YOffset = 0, Rect *clip = NULL);
};
} // End of namespace ui
} // End of namespace pyrodactyl