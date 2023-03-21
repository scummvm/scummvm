#include "stdafx.h"
#include "QuestText.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;
using namespace pyrodactyl::input;

void QuestText::Load(rapidxml::xml_node<char> *node)
{
	ParagraphData::Load(node);
	LoadNum(col_s, "color_s", node);

	if (NodeValid("line", node))
		LoadNum(lines_per_page, "page", node->first_node("line"));

	if (NodeValid("inc", node))
		inc.Load(node->first_node("inc"));

	if (NodeValid("img", node))
		img.Load(node->first_node("img"));

	if (NodeValid("prev", node))
	{
		prev.Load(node->first_node("prev"));
		prev.hotkey.Set(IU_PAGE_PREV);
	}

	if (NodeValid("next", node))
	{
		next.Load(node->first_node("next"));
		next.hotkey.Set(IU_PAGE_NEXT);
	}

	if (NodeValid("status", node))
		status.Load(node->first_node("status"));
}

void QuestText::Draw(pyrodactyl::event::Quest &q)
{
	//First, we must scan and find the part of the quest text we should draw

	//Assign default values to start and stop
	start = 0;
	stop = q.text.size();

	//Keep count of lines and pages - remember a single entry can take more than one line
	int page_count = 0, page_start = 0;

	//Start from line 0, page 0 and scan the list of entries
	for (int i = 0, line_count = 0; i < q.text.size(); ++i)
	{
		//Increment the number of lines by one text entry
		line_count += (q.text[i].size() / line.x) + 1;

		//If we go over the quota for lines per page, go to next page and reset line counter to 0
		if (line_count > lines_per_page)
		{
			//We are about to go to next page, stop at this entry
			if (page_count == current_page)
			{
				start = page_start;
				stop = i;
			}

			page_count++;
			line_count = 0;

			//This is the start of the next page
			page_start = i;
		}
	}

	//Used for the final page, because the page count won't be incremented for the last one
	if (page_count == current_page)
	{
		start = page_start;
		stop = q.text.size();
	}

	//Find out how many pages the lines need
	total_page = page_count + 1;

	//Update the text
	status.text = NumberToString(current_page + 1) + " of " + NumberToString(total_page);

	//Now, start drawing the quest
	status.Draw();

	if (current_page > 0)
		prev.Draw();

	if (current_page < total_page - 1)
		next.Draw();

	//Draw the current page of quest text
	if (!q.text.empty())
	{
		//Count the number of lines, because a single entry can take more than one line
		int count = 0;

		for (unsigned int i = start; i < stop; ++i)
		{
			img.Draw(inc.x * count, inc.y * count);

			//Draw first entry in selected color, and older quest entries in standard color
			if (i == 0)
				gTextManager.Draw(x, y, q.text[i], col_s, font, align, line.x, line.y);
			else
				ParagraphData::Draw(q.text[i], inc.x * count, inc.y * count);

			//Count is reduced extra by the amount of lines it takes for the message to be drawn
			count += (q.text[i].size() / line.x) + 1;
		}
	}
}

void QuestText::HandleEvents(pyrodactyl::event::Quest &q, const SDL_Event &Event)
{
	if (current_page > 0 && prev.HandleEvents(Event) == BUAC_LCLICK)
	{
		current_page--;

		if (current_page < 0)
			current_page = 0;
	}

	if (current_page < total_page - 1 && next.HandleEvents(Event) == BUAC_LCLICK)
	{
		current_page++;

		if (current_page >= total_page)
			current_page = total_page - 1;
	}
}

void QuestText::SetUI()
{
	ParagraphData::SetUI();
	img.SetUI();
	prev.SetUI();
	next.SetUI();
	status.SetUI();
}