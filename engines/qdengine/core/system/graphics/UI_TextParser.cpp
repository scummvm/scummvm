#include "qd_precomp.h"

#include "gr_dispatcher.h"
#include "UI_TextParser.h"

UI_TextParser::UI_TextParser(const grFont* font) : font_(font)
{
	outNodes_.reserve(8);
	init();
}

UI_TextParser::UI_TextParser(const UI_TextParser& src)
{
	(*this) = src;
}

void UI_TextParser::operator= (const UI_TextParser& src)
{
	font_ = src.font_;

	outNodes_.reserve(8);
	init();
}

void UI_TextParser::init()
{
	tagWidth_ = 0;
	lineWidth_ = 0;

	lineBegin_ = 0;
	pstr_ = 0;

	fitIn_ = -1;
	lastSpace_ = 0;
	lastTagWidth_ = 0;

	outNodes_.clear();
	outNodes_.push_back(OutNode());
	prevLineIndex_ = outNodes_.size() - 1;

	size_.set(0, fontHeight());

	lineCount_ = 1;
}

void UI_TextParser::setFont(const grFont* font)
{
	font_ = font;
	init();
}

OutNodes::const_iterator UI_TextParser::getLineBegin(int lineNum) const
{
	dassert(lineNum >= 0);

	if(!lineNum)
		return outNodes_.begin();

	if(lineNum >= lineCount_)
		return outNodes_.end();

	OutNodes::const_iterator it;
	FOR_EACH(outNodes_, it)
		if(it->type == OutNode::NEW_LINE)
			if(lineNum-- == 0)
				return it;

	xassert(lineNum == 0); 
	return outNodes_.end();
}

bool UI_TextParser::testWidth(int width)
{
	if(fitIn_ < 0)
		return true;

	if(lineWidth_ + tagWidth_ + width > fitIn_){
		if(lastSpace_ != lineBegin_){
			outNodes_.push_back(OutNode(lineBegin_, lastSpace_, lastTagWidth_));

			lineWidth_ += lastTagWidth_;
			endLine();

			lineBegin_ = lastSpace_ + 1;
			lastSpace_ = lineBegin_;
			tagWidth_ -= lastTagWidth_;
			lastTagWidth_ = 0;
		}
		else if(lineWidth_ > 0){
			dassert(lastTagWidth_ == 0);
			endLine();
			testWidth(width);
		}
		else if(tagWidth_ > 0){
			putText();
			endLine();
			skipNode();
		}
		return false;
	}
	return true;
}

void UI_TextParser::parseString(const char* text, int color, int fitIn)
{
	if(!font_)
		setFont(grDispatcher::get_default_font());

	xassert(font_);
	init();

	fitIn_ = fitIn > 2 * fontHeight() ? fitIn : -1;

	pstr_ = text;

	lineBegin_ = text;
	lastSpace_ = lineBegin_;

	while(unsigned char cc = *pstr_){
		if(cc == '\n'){
			putText();
			++pstr_;

			endLine();
			skipNode();

			continue;
		}

		if(cc < 32){
			++pstr_;
			continue;
		}

		if(cc == ' '){
			lastTagWidth_ = tagWidth_;
			lastSpace_ = pstr_;
		}

		//if(useWildChars)
		if(cc == '&'){
			if(pstr_[1] != '&'){
				putText();
				++pstr_;
				getColor(color);
				continue;
			}
			else{
				addChar('&');
				putText();
				++pstr_;
				skipNode();
				continue;
			}
		}
		else if(cc == '<'){
			if(pstr_[1] != '<'){
				putText();
				++pstr_;
				lineWidth_ += getToken();
				continue;
			}
			else{
				addChar('<');
				putText();
				++pstr_;
				skipNode();
				continue;
			}
		}
		
		addChar((BYTE)cc);
	}
	
	putText();
	size_.x = max(size_.x, lineWidth_);
	outNodes_[prevLineIndex_].width = lineWidth_;

	size_.y = fontHeight() * lineCount_;
}

int UI_TextParser::getToken()
{
	const char* ptr = pstr_;
	const char* begin_tag = ptr;
	const char* begin_style = 0;

	char cc;
	while((cc = *pstr_) && cc != '=' && cc != '>')
		++pstr_;
	unsigned int tag_len = pstr_ - begin_tag;

	if(cc != '>'){
		while((cc = *pstr_) && cc != ';' && cc != '>')
			++pstr_;
		if(cc == ';'){
			begin_style = pstr_;
			while((cc = *pstr_) && cc != '>')
				++pstr_;
		}
	}

	if(!cc){
		skipNode();
		return 0;
	}

/*	switch(tag_len){
	case 3:
		if(!strncmp(begin_tag, "img=", 4)){
			string img_name(begin_tag + 4, begin_style ? begin_style : pstr_);
			if(const UI_Sprite* sprite = UI_SpriteReference(img_name.c_str()))
				if(!sprite->isEmpty()){
					OutNode node;
					node.type = OutNode::SPRITE;
					node.sprite = sprite;
					node.style = getStyle(begin_style, pstr_);
					if((node.style & 0x03) != 2)
						node.width = sprite->size().xi();
					else{
						Vect2f size = sprite->size();
						node.width = round(size.x / size.y * fontHeight());
					}
					++pstr_;
					testWidth(node.width);
					putNode(node);
					return node.width;
				}
		}
		break;
	}*/

	++pstr_;
	skipNode();
	return 0;
}

int UI_TextParser::getStyle(const char* styleptr, const char* end)
{
	if(!styleptr || *end != '>')
		return 0;

	char cc;
	while((cc = *(++styleptr)) && cc != '=' && cc != '>');

	if(cc != '=')
		return 0;

	int style = 0;
	while((cc = *(++styleptr)) >= '0' && cc <= '9')
		style = style * 10 + (int)(cc - '0');

	return style;
}

void UI_TextParser::getColor(int defColor)
{
	int color = defColor;

	if(*pstr_ != '>'){
		DWORD s = 0;
		int i = 0;
		for(; i < 6; ++i, ++pstr_)
			if(char k = *pstr_){
				int a = fromHex(k);
				if(a < 0)
					break;
				s |= a << (i * 4);
			}
			else
				break;

		if(i > 5){
			color &= 0xFF000000;
			color |= s;
		}
		else {
			skipNode();
			return;
		}
	}
	else
		++pstr_;

	putNode(OutNode(color));
}