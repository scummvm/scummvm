/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_setup.h"
#include "qd_screen_text.h"
#include "gr_dispatcher.h"

#include "qd_game_object.h"
#include "qd_game_object_state.h"

#include "qd_game_dispatcher.h" // ¬ qdGameDispather хран€тс€ шрифты

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdScreenTextFormat qdScreenTextFormat::default_format_;
qdScreenTextFormat qdScreenTextFormat::global_text_format_;
qdScreenTextFormat qdScreenTextFormat::global_topic_format_;

qdScreenTextFormat::qdScreenTextFormat() : arrangement_(ARRANGE_VERTICAL),
	alignment_(ALIGN_LEFT),
	color_(0xFFFFFF),
	hover_color_(0xFFFFFF),
	font_type_(QD_FONT_TYPE_NONE),
	global_depend_(true)
{
}												   

bool qdScreenTextFormat::load_script(const xml::tag* p)
{
	bool load_global_depend = false;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_TEXT_ALIGN:
			set_arrangement((qdScreenTextFormat::arrangement_t)xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_ALIGN:
			set_alignment((qdScreenTextFormat::alignment_t)xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_TEXT_COLOR:
			set_color(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_TEXT_HOVER_COLOR:
			set_hover_color(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_FONT_TYPE:
			set_font_type(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_GLOBAL_DEPEND:
			global_depend_ = (0 != xml::tag_buffer(*it).get_int());
			load_global_depend = true;
			break;
		}
	}

	// ≈сли скрипт старый и инфа о св€зи с глобальным форматом отсутствует, то 
	// €вл€етс€ ли формат св€занным с глобальным по содержимому
	if(!load_global_depend)
		global_depend_ = (global_text_format() == *this);

	return true;
}

bool qdScreenTextFormat::save_script(class XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	
	fh < "<screen_text_format";

	// ≈сли зависим от глобального формата текста, то пишем только сам факт зависимости
	// (пишем факт или отсутствие зависимости в любом случае)
	if (is_global_depend())
		fh < " global_depend=\"1\"";
	else
	{// Ќе зависит от глобальных параметров - значит пишем данные о формате
		fh < " global_depend=\"0\"";

		if(arrangement() != qdScreenTextFormat::default_format().arrangement())
			fh < " text_align=\"" <= (int)arrangement() < "\"";

		if(alignment() != qdScreenTextFormat::default_format().alignment())
			fh < " align=\"" <= (int)alignment() < "\"";

		if(color() != qdScreenTextFormat::default_format().color())
			fh < " text_color=\"" <= color() < "\"";

		if(hover_color() != qdScreenTextFormat::default_format().hover_color())
			fh < " text_hover_color=\"" <= hover_color() < "\"";

		if(font_type() != qdScreenTextFormat::default_format().font_type())
			fh < " font_type=\"" <= font_type() < "\"";
	}
	
	fh < "/>\r\n";

	return true;
}

qdScreenText::qdScreenText(const char* p,const Vect2i& pos,qdGameObjectState* owner) : pos_(pos),
	size_(0,0),
	owner_(owner)
{
	set_data(p);
}

qdScreenText::qdScreenText(const char* p,const qdScreenTextFormat& fmt,const Vect2i& pos,qdGameObjectState* owner) : pos_(pos),
	size_(0,0), 
	text_format_(fmt), 
	owner_(owner)
{
	hover_mode_ = false;

	set_data(p);
}

qdScreenText::~qdScreenText()
{
}
					   
void qdScreenText::redraw(const Vect2i& owner_pos) const
{
	int x = owner_pos.x + pos_.x;
	int y = owner_pos.y + pos_.y;

	unsigned col = hover_mode_ ? text_format_.hover_color() : text_format_.color();

	const grFont* font = qdGameDispatcher::get_dispatcher()->
		                   find_font(text_format_.font_type());

	grDispatcher::instance() -> DrawAlignedText(x,y,size_.x,size_.y,col,data(),grTextAlign(text_format_.alignment()),0,0,font);
	if(qdGameConfig::get_config().debug_draw())
		grDispatcher::instance() -> Rectangle(x,y,size_.x,size_.y,col,0,GR_OUTLINED);
}

void qdScreenText::set_data(const char* p)
{
	data_ = p;

	const grFont* font = qdGameDispatcher::get_dispatcher()->
		                   find_font(text_format_.font_type());
	size_.x = grDispatcher::instance() -> TextWidth(data(),0,font);
	size_.y = grDispatcher::instance() -> TextHeight(data(),0,font);
}

bool qdScreenText::is_owned_by(const qdNamedObject* p) const
{ 
	return (owner_ && p == owner_ -> owner());
}

bool qdScreenText::format_text(int max_width)
{
	const grFont* font = qdGameDispatcher::get_dispatcher()->find_font(
		            text_format().font_type());
	if (NULL == font) font = grDispatcher::get_default_font();

	bool correct = true;
	int safe_space = -1;
	int cur_wid = 0;
	data_ += ' '; // ƒобавл€ем пробел дл€ упрощени€ алгоритма (из-за последнего 
	              // пробела в конце всегда включитс€ попытка форматировани€ конца).
	              // ѕробел не отразитьс€ на выводе, т.к. он в конце.

	unsigned char* dp = (unsigned char*)data_.begin();

	for (int i = 0; i < data_.length(); i++)
	{
		if(dp[i] == '\n'){
			if(cur_wid > max_width)
			{
				// безопасный пробел есть - безопасно режем (т.е. все влезает в max_width)
				if (safe_space >= 0)
				{
					dp[safe_space] = '\n';
					i = safe_space; // в for(...) перейдем к safe_space + 1
				}
				// не влезли (нет безопасного пробела). Ќо режем все равно - хоть так...
				else
				{
					dp[i] = '\n';
					correct = false;
				}
			}

			safe_space = -1;
			cur_wid = 0;
		}
		// Ќе пробел - копим длину
		else if (' ' != dp[i])
			cur_wid += font->find_char(dp[i]).size_x();
		// ѕробел - здесь можно резать (запомним эту позицию или разрежем здесь)
		else
		{
			cur_wid += font->size_x()/2;

			// ƒлина не превышена - запомним текущий _безопасный_ пробел и двинемс€ дальше
			if (cur_wid < max_width)
			{
				safe_space = i;
				continue;
			}
			// ѕревысили длину, да еще и пробел встретили - нужно резать
			else
			{
				// безопасный пробел есть - безопасно режем (т.е. все влезает в max_width)
				if (safe_space >= 0)
				{
					dp[safe_space] = '\n';
					i = safe_space; // в for(...) перейдем к safe_space + 1
				}
				// не влезли (нет безопасного пробела). Ќо режем все равно - хоть так...
				else
				{
					dp[i] = '\n';
					correct = false;
				}
				safe_space = -1; // –азрезали - безопасного пробела нет
				cur_wid = 0;     // Ќовый кусок пока нулевой длины
			}
		}
	}
	
	data_.erase(data_.length()-1, 1); // ”дал€ем последний символ (пробел добавленный нами)

	set_data(data_.c_str()); // ”станавливаем данные (дл€ пересчета размера текста)

	return correct;
}
