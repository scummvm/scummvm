#pragma once

#include "common_header.h"
#include "menu.h"

namespace pyrodactyl
{
	namespace ui
	{
		//Sometimes we need multiple pages in a menu, this object does that
		//Used in save, load, mod and quest menu
		template <typename T>
		class PageMenu
		{
			//The buttons for cycling between pages of the menu
			Button prev, next;

			//Each page is stored separately in a menu object
			std::vector<Menu<T>> menu;

			//Keep track of which page we are at, and how many elements we keep in a page
			unsigned int current_page, elements_per_page, rows, cols;

			//The image used for the elements
			Button ref;

			//This vector stores the increments in x,y for each new button
			Vector2i inc;

			//Display "Page 1 of 3" style information for the menu
			TextData status;
			std::string info;

		public:
			PageMenu(){ current_page = 0; elements_per_page = 1; rows = 1; cols = 1; Clear(); }
			~PageMenu(){}

			void Reset()
			{
				for (auto m = menu.begin(); m != menu.end(); ++m)
					m->Reset();
			}

			void Clear()
			{
				menu.resize(1);
				menu.at(0).Clear();
				menu.at(0).UseKeyboard(true);
			}

			//Get the elements per page
			unsigned int ElementsPerPage() { return elements_per_page; }

			//This is added to the result from HandleEvents to calculate the exact position
			unsigned int Index() { return current_page * elements_per_page; }

			//The end position of the elements
			unsigned int IndexPlusOne() { return (current_page + 1) * elements_per_page; }

			//Get the current page of the menu
			unsigned int CurrentPage() { return current_page; }
			void CurrentPage(int &val) { current_page = val; }

			//Get the index of the hovered element in the menu
			const int HoverIndex()
			{
				if (menu.at(current_page).HoverIndex() >= 0)
					return (current_page * elements_per_page) + menu.at(current_page).HoverIndex();

				return -1;
			}

			//Get the base position of the elements
			const int BaseX(const int &count) { return ref.x + inc.x * (count % cols); }
			const int BaseY(const int &count) { return ref.y + inc.y * (count / cols); }

			//This is used to get the coordinates of a button
			const int& CurX(const int& count) { return menu[current_page].element[count].x; }
			const int& CurY(const int& count) { return menu[current_page].element[count].y; }

			void Image(const int &slot, const int &page, ButtonImage &bi) { menu.at(page).element.at(slot).Img(bi); }

			void AssignPaths()
			{
				for (auto m = menu.begin(); m != menu.end(); ++m)
					m->AssignPaths();
			}

			void UseKeyboard(const bool &val)
			{
				for (auto m = menu.begin(); m != menu.end(); ++m)
					m->UseKeyboard(val);
			}

			void SetUI()
			{
				prev.SetUI();
				next.SetUI();
				ref.SetUI();
				status.SetUI();

				for (auto m = menu.begin(); m != menu.end(); ++m)
					m->SetUI();
			}

			void UpdateInfo() { info = NumberToString(current_page + 1) + " of " + NumberToString(menu.size()); }

			void Load(rapidxml::xml_node<char> *node)
			{
				using namespace pyrodactyl::input;
				if (NodeValid(node))
				{
					if (NodeValid("prev", node))
					{
						prev.Load(node->first_node("prev"));
						prev.hotkey.Set(IU_PREV);
					}

					if (NodeValid("next", node))
					{
						next.Load(node->first_node("next"));
						next.hotkey.Set(IU_NEXT);
					}

					if (NodeValid("reference", node))
						ref.Load(node->first_node("reference"));

					if (NodeValid("inc", node))
						inc.Load(node->first_node("inc"));

					if (NodeValid("status", node))
						status.Load(node->first_node("status"));

					if (NodeValid("dim", node))
					{
						rapidxml::xml_node<char> *dimnode = node->first_node("dim");
						LoadNum(rows, "rows", dimnode);
						LoadNum(cols, "cols", dimnode);
						elements_per_page = rows * cols;
					}
				}
			}

			void Add(unsigned int &slot, unsigned int &page)
			{
				if (slot >= elements_per_page)
				{
					++page;
					slot = 0;
					menu.resize(page + 1);
					menu.at(page).UseKeyboard(true);
				}

				T b;
				b.Init(ref, inc.x*(slot%cols), inc.y*(slot / cols));
				menu[page].element.push_back(b);
				++slot;

				AssignPaths();
				UpdateInfo();
			}

			void Add()
			{
				unsigned int page = menu.size() - 1;
				unsigned int slot = menu.at(page).element.size();
				Add(slot, page);
			}

			void Erase()
			{
				unsigned int page = menu.size() - 1;
				menu.at(page).element.pop_back();
				AssignPaths();
				UpdateInfo();
			}

			int HandleEvents(const SDL_Event &Event)
			{
				using namespace pyrodactyl::input;

				if (current_page > 0 && prev.HandleEvents(Event) == BUAC_LCLICK)
				{
					current_page--;
					UpdateInfo();

					if (current_page < 0)
						current_page = 0;
				}

				if (current_page < menu.size() - 1 && next.HandleEvents(Event) == BUAC_LCLICK)
				{
					current_page++;
					UpdateInfo();

					if (current_page >= menu.size())
						current_page = menu.size() - 1;
				}

				return menu.at(current_page).HandleEvents(Event);
			}

			void Draw()
			{
				status.Draw(info);
				menu.at(current_page).Draw();

				if (current_page > 0)
					prev.Draw();

				if (current_page < menu.size() - 1)
					next.Draw();
			}
		};

		typedef PageMenu<Button> PageButtonMenu;
	}
}