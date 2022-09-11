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

#include "ultima/ultima4/game/script.h"
#include "ultima/ultima4/game/armor.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/controllers/inn_controller.h"
#include "ultima/ultima4/conversation/conversation.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/game/weapon.h"
#include "ultima/ultima4/game/spell.h"
#include "ultima/ultima4/views/stats.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/shared/conf/xml_tree.h"

namespace Ultima {
namespace Ultima4 {

/*
 * Script::Variable class
 */
Script::Variable::Variable() : _iVal(0), _sVal(""), _set(false) {}
Script::Variable::Variable(const Common::String &v) : _set(true) {
	_iVal = static_cast<int>(strtol(v.c_str(), nullptr, 10));
	_sVal = v;
}

Script::Variable::Variable(const int &v) : _set(true) {
	_iVal = v;
	_sVal = xu4_to_string(v);
}

int &Script::Variable::getInt() {
	return _iVal;
}

Common::String &Script::Variable::getString() {
	return _sVal;
}

void Script::Variable::setValue(const int &v) {
	_iVal = v;
}

void Script::Variable::setValue(const Common::String &v) {
	_sVal = v;
}

void Script::Variable::unset() {
	_set = false;
	_iVal = 0;
	_sVal = "";
}

bool Script::Variable::isInt() const {
	return _iVal > 0;
}

bool Script::Variable::isString() const {
	return _iVal == 0;
}

bool Script::Variable::isSet() const {
	return _set;
}

Script::Script() : _vendorScriptDoc(nullptr), _scriptNode(nullptr),
		_debug(false), _state(STATE_UNLOADED), _currentScript(nullptr),
		_currentItem(nullptr), _inputType(INPUT_CHOICE), _inputMaxLen(0),
		_nounName("item"), _idPropName("id"), _iterator(0) {
	_actionMap["context"]           = ACTION_SET_CONTEXT;
	_actionMap["unset_context"]     = ACTION_UNSET_CONTEXT;
	_actionMap["end"]               = ACTION_END;
	_actionMap["redirect"]          = ACTION_REDIRECT;
	_actionMap["wait_for_keypress"] = ACTION_WAIT_FOR_KEY;
	_actionMap["wait"]              = ACTION_WAIT;
	_actionMap["stop"]              = ACTION_STOP;
	_actionMap["include"]           = ACTION_INCLUDE;
	_actionMap["for"]               = ACTION_FOR_LOOP;
	_actionMap["random"]            = ACTION_RANDOM;
	_actionMap["move"]              = ACTION_MOVE;
	_actionMap["sleep"]             = ACTION_SLEEP;
	_actionMap["cursor"]            = ACTION_CURSOR;
	_actionMap["pay"]               = ACTION_PAY;
	_actionMap["if"]                = ACTION_IF;
	_actionMap["input"]             = ACTION_INPUT;
	_actionMap["add"]               = ACTION_ADD;
	_actionMap["lose"]              = ACTION_LOSE;
	_actionMap["heal"]              = ACTION_HEAL;
	_actionMap["cast_spell"]        = ACTION_CAST_SPELL;
	_actionMap["damage"]            = ACTION_DAMAGE;
	_actionMap["karma"]             = ACTION_KARMA;
	_actionMap["music"]             = ACTION_MUSIC;
	_actionMap["var"]               = ACTION_SET_VARIABLE;
	_actionMap["ztats"]             = ACTION_ZTATS;
}

Script::~Script() {
	unload();

	// We have many Variables that are allocated but need to have delete called on them.
	// We do not need to clear the containers (that will happen automatically), but we do need to delete
	// these things. Do NOT clean up the providers though, it seems the providers map doesn't own its pointers.
	// Smart pointers anyone?

	// Clean variables
	Common::HashMap<Common::String, Script::Variable *>::iterator variableItem = _variables.begin();
	Common::HashMap<Common::String, Script::Variable *>::iterator variablesEnd = _variables.end();
	while (variableItem != variablesEnd) {
		delete variableItem->_value;
		++variableItem;
	}
}

void Script::removeCurrentVariable(const Common::String &name) {
	Common::HashMap<Common::String, Script::Variable *>::iterator dup = _variables.find(name);
	if (dup != _variables.end()) {
		delete dup->_value;
		_variables.erase(dup); // not strictly necessary, but correct.
	}
}

void Script::addProvider(const Common::String &name, Provider *p) {
	_providers[name] = p;
}

bool Script::load(const Common::String &filename, const Common::String &baseId, const Common::String &subNodeName, const Common::String &subNodeId) {
	Shared::XMLNode *root, *node, *child;
	_state = STATE_NORMAL;

	/* unload previous script */
	unload();

	/**
	 * Open and parse the .xml file
	 */
	Shared::XMLTree *doc = new Shared::XMLTree(
		Common::String::format("data/conf/%s", filename.c_str()));
	_vendorScriptDoc = root = doc->getTree();

	if (!root->id().equalsIgnoreCase("scripts"))
		error("malformed %s", filename.c_str());

	// Check whether script is set to debug
	_debug = root->getPropertyBool("debug");

	/**
	 * Get a new global item name or id name
	 */
	if (root->hasProperty("noun"))
		_nounName = root->getProperty("noun");
	if (root->hasProperty("id_prop"))
		_idPropName = root->getProperty("id_prop");

	_currentScript = nullptr;
	_currentItem = nullptr;

	for (node = root->firstChild(); node; node = node->getNext()) {
		if (node->nodeIsText() || !node->id().equalsIgnoreCase("script"))
			continue;

		if (baseId == node->getProperty("id")) {
			/**
			 * We use the base node as our main script node
			 */
			if (subNodeName.empty()) {
				_scriptNode = node;
				_translationContext.push_back(node);

				break;
			}

			for (child = node->firstChild(); child; child = child->getNext()) {
				if (child->nodeIsText() || !child->id().equalsIgnoreCase(subNodeName))
					continue;

				Common::String id = child->getProperty("id");

				if (id == subNodeId) {
					_scriptNode = child;
					_translationContext.push_back(child);

					/**
					 * Get a new local item name or id name
					 */
					if (node->hasProperty("noun"))
						_nounName = node->getProperty("noun");
					if (node->hasProperty("id_prop"))
						_idPropName = node->getProperty("id_prop");

					break;
				}
			}

			if (_scriptNode)
				break;
		}
	}

	if (_scriptNode) {
		/**
		 * Get a new local item name or id name
		 */
		if (_scriptNode->hasProperty("noun"))
			_nounName = _scriptNode->getProperty("noun");
		if (_scriptNode->hasProperty("id_prop"))
			_idPropName = _scriptNode->getProperty("id_prop");

		if (_debug)
			debug("\n<Loaded subscript '%s' where id='%s' for script '%s'>\n", subNodeName.c_str(), subNodeId.c_str(), baseId.c_str());
	} else {
		if (subNodeName.empty())
			error("Couldn't find script '%s' in %s", baseId.c_str(), filename.c_str());
		else
			error("Couldn't find subscript '%s' where id='%s' in script '%s' in %s", subNodeName.c_str(), subNodeId.c_str(), baseId.c_str(), filename.c_str());
	}

	_state = STATE_UNLOADED;

	return false;
}

void Script::unload() {
	if (_vendorScriptDoc) {
		_vendorScriptDoc->freeDoc();
		_vendorScriptDoc = nullptr;
	}
}

void Script::run(const Common::String &script) {
	Shared::XMLNode *scriptNode;
	Common::String search_id;

	if (_variables.find(_idPropName) != _variables.end()) {
		if (_variables[_idPropName]->isSet())
			search_id = _variables[_idPropName]->getString();
		else
			search_id = "null";
	}

	scriptNode = find(_scriptNode, script, search_id);

	if (!scriptNode)
		error("Script '%s' not found in vendorScript.xml", script.c_str());

	execute(scriptNode);
}

Script::ReturnCode Script::execute(Shared::XMLNode *script, Shared::XMLNode *currentItem, Common::String *output) {
	Shared::XMLNode *current;
	Script::ReturnCode retval = RET_OK;

	if (!script->hasChildren()) {
		/* redirect the script to another node */
		if (script->hasProperty("redirect"))
			retval = redirect(nullptr, script);
		/* end the conversation */
		else {
			if (_debug)
				debug("A script with no children found (nowhere to go). Ending script...");
			g_screen->screenMessage("\n");
			_state = STATE_DONE;
		}
	}

	/* do we start where we left off, or start from the beginning? */
	if (currentItem) {
		current = currentItem->getNext();
		if (_debug)
			debug("Returning to execution from end of '%s' script", currentItem->id().c_str());
	} else {
		current = script->firstChild();
	}

	for (; current; current = current->getNext()) {
		Common::String name = current->id();
		retval = RET_OK;
		ActionMap::iterator action;

		/* nothing left to do */
		if (_state == STATE_DONE)
			break;

		/* begin execution of script */

		/**
		 * Handle Text
		 */
		if (current->nodeIsText()) {
			Common::String content = getContent(current);
			if (output)
				*output += content;
			else
				g_screen->screenMessage("%s", content.c_str());

			if (_debug && content.size())
				debug("Output: \n====================\n%s\n====================", content.c_str());
		} else {
			/**
			 * Search for the corresponding action and execute it!
			 */
			action = _actionMap.find(name);
			if (action != _actionMap.end()) {
				/**
				 * Found it!
				 */
				switch (action->_value) {
				case ACTION_SET_CONTEXT:
					retval = pushContext(script, current);
					break;
				case ACTION_UNSET_CONTEXT:
					retval = popContext(script, current);
					break;
				case ACTION_END:
					retval = end(script, current);
					break;
				case ACTION_REDIRECT:
					retval = redirect(script, current);
					break;
				case ACTION_WAIT_FOR_KEY:
					retval = waitForKeypress(script, current);
					break;
				case ACTION_WAIT:
					retval = wait(script, current);
					break;
				case ACTION_STOP:
					retval = RET_STOP;
					break;
				case ACTION_INCLUDE:
					retval = include(script, current);
					break;
				case ACTION_FOR_LOOP:
					retval = forLoop(script, current);
					break;
				case ACTION_RANDOM:
					retval = randomScript(script, current);
					break;
				case ACTION_MOVE:
					retval = move(script, current);
					break;
				case ACTION_SLEEP:
					retval = sleep(script, current);
					break;
				case ACTION_CURSOR:
					retval = cursor(script, current);
					break;
				case ACTION_PAY:
					retval = pay(script, current);
					break;
				case ACTION_IF:
					retval = _if(script, current);
					break;
				case ACTION_INPUT:
					retval = input(script, current);
					break;
				case ACTION_ADD:
					retval = add(script, current);
					break;
				case ACTION_LOSE:
					retval = lose(script, current);
					break;
				case ACTION_HEAL:
					retval = heal(script, current);
					break;
				case ACTION_CAST_SPELL:
					retval = castSpell(script, current);
					break;
				case ACTION_DAMAGE:
					retval = damage(script, current);
					break;
				case ACTION_KARMA:
					retval = karma(script, current);
					break;
				case ACTION_MUSIC:
					retval = music(script, current);
					break;
				case ACTION_SET_VARIABLE:
					retval = setVar(script, current);
					break;
				case ACTION_ZTATS:
					retval = ztats(script, current);
					break;
				default:

					break;
				}
			}
			/**
			 * Didn't find the corresponding action...
			 */
			else if (_debug)
				debug("ERROR: '%s' method not found", name.c_str());

			/* The script was redirected or stopped, stop now! */
			if ((retval == RET_REDIRECTED) || (retval == RET_STOP))
				break;
		}

		if (_debug)
			debug("\n");
	}

	return retval;
}

void Script::_continue() {
	/* reset our script state to normal */
	resetState();

	/* there's no target indicated, just start where we left off! */
	if (_target.empty())
		execute(_currentScript, _currentItem);
	else
		run(_target);
}

void Script::resetState() {
	_state = STATE_NORMAL;
}

void Script::setState(Script::State s) {
	_state = s;
}

void Script::setTarget(const Common::String &val) {
	_target = val;
}

void Script::setChoices(const Common::String &val) {
	_choices = val;
}

void Script::setVar(const Common::String &name, const Common::String &val) {
	removeCurrentVariable(name);
	_variables[name] = new Variable(val);
}

void Script::setVar(const Common::String &name, int val) {
	removeCurrentVariable(name);
	_variables[name] = new Variable(val);
}

void Script::unsetVar(const Common::String &name) {
	// Ensure that the variable at least exists, but has no value
	if (_variables.find(name) != _variables.end())
		_variables[name]->unset();
	else
		_variables[name] = new Variable();
}

Script::State Script::getState() {
	return _state;
}

Common::String Script::getTarget() {
	return _target;
}

Script::InputType Script::getInputType() {
	return _inputType;
}

Common::String Script::getChoices() {
	return _choices;
}

Common::String Script::getInputName() {
	return _inputName;
}

int Script::getInputMaxLen() {
	return _inputMaxLen;
}

void Script::translate(Common::String *text) {
	uint pos;
	bool nochars = true;
	Shared::XMLNode *node = _translationContext.back();

	/* determine if the script is completely whitespace */
	for (Common::String::iterator current = text->begin(); current != text->end(); current++) {
		if (Common::isAlnum(*current)) {
			nochars = false;
			break;
		}
	}

	/* erase scripts that are composed entirely of whitespace */
	if (nochars)
		text->clear();

	while ((pos = text->findFirstOf("{")) < text->size()) {
		Common::String pre = text->substr(0, pos);
		Common::String post;
		Common::String item = text->substr(pos + 1);

		/**
		 * Handle embedded items
		 */
		int num_embedded = 0;
		int total_pos = 0;
		Common::String current = item;
		while (true) {
			uint open = current.findFirstOf("{"),
			             close = current.findFirstOf("}");

			if (close == current.size())
				error("Error: no closing } found in script.");

			if (open < close) {
				num_embedded++;
				total_pos += open + 1;
				current = current.substr(open + 1);
			}
			if (close < open) {
				total_pos += close;
				if (num_embedded == 0) {
					pos = total_pos;
					break;
				}
				num_embedded--;
				total_pos += 1;
				current = current.substr(close + 1);
			}
		}

		/**
		 * Separate the item itself from the pre- and post-data
		 */
		post = item.substr(pos + 1);
		item = item.substr(0, pos);

		if (_debug)
			debugN("{%s} == ", item.c_str());

		/* translate any stuff contained in the item */
		translate(&item);

		Common::String prop;

		// Get defined variables
		if (item[0] == '$') {
			Common::String varName = item.substr(1);
			if (_variables.find(varName) != _variables.end())
				prop = _variables[varName]->getString();
		}
		// Get the current iterator for our loop
		else if (item == "iterator")
			prop = xu4_to_string(_iterator);
		else if ((pos = item.find("show_inventory:")) < item.size()) {
			pos = item.find(":");
			Common::String itemScript = item.substr(pos + 1);

			Shared::XMLNode *itemShowScript = find(node, itemScript);

			Shared::XMLNode *nodePtr;
			prop.clear();

			/**
			 * Save iterator
			 */
			int oldIterator = _iterator;

			/* start iterator at 0 */
			_iterator = 0;

			for (nodePtr = node->firstChild(); nodePtr; nodePtr = nodePtr->getNext()) {
				if (nodePtr->id().equalsIgnoreCase(_nounName)) {
					bool hidden = nodePtr->getPropertyBool("hidden");

					if (!hidden) {
						/* make sure the nodePtr's requisites are met */
						if (!nodePtr->hasProperty("req") || compare(nodePtr->getProperty("req"))) {
							/* put a newline after each */
							if (_iterator > 0)
								prop += "\n";

							/* set translation context to nodePtr */
							_translationContext.push_back(nodePtr);
							execute(itemShowScript, nullptr, &prop);
							_translationContext.pop_back();

							_iterator++;
						}
					}
				}
			}

			/**
			 * Restore iterator to previous value
			 */
			_iterator = oldIterator;
		}

		/**
		 * Make a Common::String containing the available ids using the
		 * vendor's inventory (i.e. "bcde")
		 */
		else if (item == "inventory_choices") {
			Shared::XMLNode *nodePtr;
			Common::String ids;

			for (nodePtr = node->firstChild(); nodePtr; nodePtr = nodePtr->getNext()) {
				if (nodePtr->id().equalsIgnoreCase(_nounName)) {
					Common::String id = getPropAsStr(nodePtr, _idPropName.c_str());
					/* make sure the nodePtr's requisites are met */
					if (!nodePtr->hasProperty("req") || (compare(getPropAsStr(nodePtr, "req"))))
						ids += id[0];
				}
			}

			prop = ids;
		}

		/**
		 * Ask our providers if they have a valid translation for us
		 */
		else if (item.findFirstOf(":") != Common::String::npos) {
			int index = item.findFirstOf(":");
			Common::String provider = item;
			Common::String to_find;

			provider = item.substr(0, index);
			to_find = item.substr(index + 1);
			if (_providers.find(provider) != _providers.end()) {
				Std::vector<Common::String> parts = split(to_find, ":");
				Provider *p = _providers[provider];
				prop = p->translate(parts);
			}
		}

		/**
		 * Resolve as a property name or a function
		 */
		else {
			Common::String funcName, content;

			funcParse(item, &funcName, &content);

			/*
			 * Check to see if it's a property name
			 */
			if (funcName.empty()) {
				/* we have the property name, now go get the property value! */
				prop = getPropAsStr(_translationContext, item, true);
			}

			/**
			 * We have a function, make it work!
			 */
			else {
				/* perform the <math> function on the content */
				if (funcName == "math") {
					if (content.empty())
						warning("Error: empty math() function");

					prop = xu4_to_string(mathValue(content));
				}

				/**
				 * Does a true/false comparison on the content.
				 * Replaced with "true" if evaluates to true, or "false" if otherwise
				 */
				else if (funcName == "compare") {
					if (compare(content))
						prop = "true";
					else
						prop = "false";
				}

				/* make the Common::String upper case */
				else if (funcName == "toupper") {
					Common::String::iterator it;
					for (it = content.begin(); it != content.end(); it++)
						*it = toupper(*it);

					prop = content;
				}
				/* make the Common::String lower case */
				else if (funcName == "tolower") {
					Common::String::iterator it;
					for (it = content.begin(); it != content.end(); it++)
						*it = tolower(*it);

					prop = content;
				}

				/* generate a random number */
				else if (funcName == "random")
					prop = xu4_to_string(xu4_random((int)strtol(content.c_str(), nullptr, 10)));

				/* replaced with "true" if content is empty, or "false" if not */
				else if (funcName == "isempty") {
					if (content.empty())
						prop = "true";
					else
						prop = "false";
				}
			}
		}

		if (prop.empty() && _debug)
			debug("Warning: dynamic property '{%s}' not found in vendor script (was this intentional?)", item.c_str());

		if (_debug)
			debug("\"%s\"", prop.c_str());

		/* put the script back together */
		*text = pre + prop + post;
	}

	/* remove all unnecessary spaces from xml */
	while ((pos = text->find("\t")) < text->size())
		text->replace(pos, 1, "");
	while ((pos = text->find("  ")) < text->size())
		text->replace(pos, 2, "");
	while ((pos = text->find("\n ")) < text->size())
		text->replace(pos, 2, "\n");
}

Shared::XMLNode *Script::find(Shared::XMLNode *node, const Common::String &script_to_find, const Common::String &id, bool _default) {
	Shared::XMLNode *current;
	if (node) {
		for (current = node->firstChild(); current; current = current->getNext()) {
			if (!current->nodeIsText() && (script_to_find == current->id().c_str())) {
				if (id.empty() && !current->hasProperty(_idPropName.c_str()) && !_default)
					return current;
				else if (current->hasProperty(_idPropName.c_str()) && (id == current->getProperty(_idPropName)))
					return current;
				else if (_default && current->hasProperty("default") && current->getPropertyBool("default"))
					return current;
			}
		}

		/* only search the parent nodes if we haven't hit the base <script> node */
		if (!node->id().equalsIgnoreCase("script"))
			current = find(node->getParent(), script_to_find, id);

		/* find the default script instead */
		if (!current && !id.empty() && !_default)
			current = find(node, script_to_find, "", true);
		return current;
	}
	return nullptr;
}

Common::String Script::getPropAsStr(Std::list<Shared::XMLNode *> &nodes, const Common::String &prop, bool recursive) {
	Common::String propvalue;
	Std::list<Shared::XMLNode *>::reverse_iterator i;

	for (i = nodes.rbegin(); i != nodes.rend(); ++i) {
		Shared::XMLNode *node = *i;
		if (node->hasProperty(prop)) {
			propvalue = node->getProperty(prop);
			break;
		}
	}

	if (propvalue.empty() && recursive) {
		for (i = nodes.rbegin(); i != nodes.rend(); ++i) {
			Shared::XMLNode *node = *i;
			if (node->getParent()) {
				propvalue = getPropAsStr(node->getParent(), prop, recursive);
				break;
			}
		}
	}

	translate(&propvalue);
	return propvalue;
}

Common::String Script::getPropAsStr(Shared::XMLNode *node, const Common::String &prop, bool recursive) {
	Std::list<Shared::XMLNode *> list;
	list.push_back(node);
	return getPropAsStr(list, prop, recursive);
}

int Script::getPropAsInt(Std::list<Shared::XMLNode *> &nodes, const Common::String &prop, bool recursive) {
	Common::String propvalue = getPropAsStr(nodes, prop, recursive);
	return mathValue(propvalue);
}

int Script::getPropAsInt(Shared::XMLNode *node, const Common::String &prop, bool recursive) {
	Common::String propvalue = getPropAsStr(node, prop, recursive);
	return mathValue(propvalue);
}

Common::String Script::getContent(Shared::XMLNode *node) {
	Common::String content = node->text();
	translate(&content);
	return content;
}

Script::ReturnCode Script::pushContext(Shared::XMLNode *script, Shared::XMLNode *current) {
	Common::String nodeName = getPropAsStr(current, "name");
	Common::String search_id;

	if (current->hasProperty(_idPropName.c_str()))
		search_id = getPropAsStr(current, _idPropName);
	else if (_variables.find(_idPropName) != _variables.end()) {
		if (_variables[_idPropName]->isSet())
			search_id = _variables[_idPropName]->getString();
		else
			search_id = "null";
	}

	// When looking for a new context, start from within our old one
	_translationContext.push_back(find(_translationContext.back(), nodeName, search_id));
	if (_debug) {
		if (!_translationContext.back())
			debug("Warning!!! Invalid translation context <%s %s=\"%s\" ...>", nodeName.c_str(), _idPropName.c_str(), search_id.c_str());
		else
			debug("Changing translation context to <%s %s=\"%s\" ...>", nodeName.c_str(), _idPropName.c_str(), search_id.c_str());
	}

	return RET_OK;
}

Script::ReturnCode Script::popContext(Shared::XMLNode *script, Shared::XMLNode *current) {
	if (_translationContext.size() > 1) {
		_translationContext.pop_back();
		if (_debug)
			debug("Reverted translation context to <%s ...>", _translationContext.back()->id().c_str());
	}
	return RET_OK;
}

Script::ReturnCode Script::end(Shared::XMLNode *script, Shared::XMLNode *current) {
	/**
	 * See if there's a global 'end' node declared for cleanup
	 */
	Shared::XMLNode *endScript = find(_scriptNode, "end");
	if (endScript)
		execute(endScript);

	if (_debug)
		debug("<End script>");

	_state = STATE_DONE;

	return RET_STOP;
}

Script::ReturnCode Script::waitForKeypress(Shared::XMLNode *script, Shared::XMLNode *current) {
	_currentScript = script;
	_currentItem = current;
	_choices = "abcdefghijklmnopqrstuvwxyz01234567890\015 \033";
	_target.clear();
	_state = STATE_INPUT;
	_inputType = INPUT_KEYPRESS;

	if (_debug)
		debug("<Wait>");

	return RET_STOP;
}

Script::ReturnCode Script::redirect(Shared::XMLNode *script, Shared::XMLNode *current) {
	Common::String target;

	if (current->hasProperty("redirect"))
		target = getPropAsStr(current, "redirect");
	else
		target = getPropAsStr(current, "target");

	/* set a new search id */
	Common::String search_id = getPropAsStr(current, _idPropName);

	Shared::XMLNode *newScript = find(_scriptNode, target, search_id);
	if (!newScript)
		error("Error: redirect failed -- could not find target script '%s' with %s=\"%s\"", target.c_str(), _idPropName.c_str(), search_id.c_str());

	if (_debug) {
		debugN("Redirected to <%s", target.c_str());
		if (search_id.size())
			debugN(" %s=\"%s\"", _idPropName.c_str(), search_id.c_str());
		debug(" .../>");
	}

	execute(newScript);
	return RET_REDIRECTED;
}

Script::ReturnCode Script::include(Shared::XMLNode *script, Shared::XMLNode *current) {
	Common::String scriptName = getPropAsStr(current, "script");
	Common::String id = getPropAsStr(current, _idPropName);

	Shared::XMLNode *newScript = find(_scriptNode, scriptName, id);
	if (!newScript)
		error("Error: include failed -- could not find target script '%s' with %s=\"%s\"", scriptName.c_str(), _idPropName.c_str(), id.c_str());

	if (_debug) {
		debugN("Included script <%s", scriptName.c_str());
		if (!id.empty())
			debugN(" %s=\"%s\"", _idPropName.c_str(), id.c_str());
		debug(" .../>");
	}

	execute(newScript);
	return RET_OK;
}

Script::ReturnCode Script::wait(Shared::XMLNode *script, Shared::XMLNode *current) {
	int msecs = getPropAsInt(current, "msecs");
	EventHandler::wait_msecs(msecs);
	return RET_OK;
}

Script::ReturnCode Script::forLoop(Shared::XMLNode *script, Shared::XMLNode *current) {
	Script::ReturnCode retval = RET_OK;
	int start = getPropAsInt(current, "start"),
	    end = getPropAsInt(current, "end"),
	    /* save the iterator in case this loop is nested */
	    oldIterator = _iterator,
	    i;

	if (_debug)
		debug("\n<For Start=%d End=%d>", start, end);

	for (i = start, _iterator = start;
	        i <= end;
	        i++, _iterator++) {

		if (_debug)
			debug("%d: ", i);

		retval = execute(current);
		if ((retval == RET_REDIRECTED) || (retval == RET_STOP))
			break;
	}

	/* restore the previous iterator */
	_iterator = oldIterator;

	return retval;
}

Script::ReturnCode Script::randomScript(Shared::XMLNode *script, Shared::XMLNode *current) {
	int perc = getPropAsInt(current, "chance");
	int num = xu4_random(100);
	Script::ReturnCode retval = RET_OK;

	if (num < perc)
		retval = execute(current);

	if (_debug)
		debug("Random (%d%%): rolled %d (%s)", perc, num, (num < perc) ? "Succeeded" : "Failed");

	return retval;
}

Script::ReturnCode Script::move(Shared::XMLNode *script, Shared::XMLNode *current) {
	if (current->hasProperty("x"))
		g_context->_location->_coords.x = getPropAsInt(current, "x");
	if (current->hasProperty("y"))
		g_context->_location->_coords.y = getPropAsInt(current, "y");
	if (current->hasProperty("z"))
		g_context->_location->_coords.z = getPropAsInt(current, "z");

	if (_debug)
		debug("Move: x-%d y-%d z-%d", g_context->_location->_coords.x, g_context->_location->_coords.y, g_context->_location->_coords.z);

	gameUpdateScreen();
	return RET_OK;
}

Script::ReturnCode Script::sleep(Shared::XMLNode *script, Shared::XMLNode *current) {
	if (_debug)
		debug("Sleep!");

	CombatController *cc = new InnController();
	cc->begin();

	return RET_OK;
}

Script::ReturnCode Script::cursor(Shared::XMLNode *script, Shared::XMLNode *current) {
	bool enable = current->getPropertyBool("enable");
	if (enable)
		g_screen->screenEnableCursor();
	else
		g_screen->screenDisableCursor();

	return RET_OK;
}

Script::ReturnCode Script::pay(Shared::XMLNode *script, Shared::XMLNode *current) {
	int price = getPropAsInt(current, "price");
	int quant = getPropAsInt(current, "quantity");

	Common::String cantpay = getPropAsStr(current, "cantpay");

	if (price < 0)
		error("Error: could not find price for item");

	if (_debug) {
		debug("Pay: price(%d) quantity(%d)", price, quant);
		debug("\tParty gold:  %d -", g_ultima->_saveGame->_gold);
		debug("\tTotal price: %d", price * quant);
	}

	price *= quant;
	if (price > g_ultima->_saveGame->_gold) {
		if (_debug)
			debug("\t=== Can't pay! ===");
		run(cantpay);
		return RET_STOP;
	} else {
		g_context->_party->adjustGold(-price);
	}

	if (_debug)
		debug("\tBalance:     %d\n", g_ultima->_saveGame->_gold);

	return RET_OK;
}

Script::ReturnCode Script::_if(Shared::XMLNode *script, Shared::XMLNode *current) {
	Common::String test = getPropAsStr(current, "test");
	Script::ReturnCode retval = RET_OK;

	if (_debug)
		debugN("If(%s) - ", test.c_str());

	if (compare(test)) {
		if (_debug)
			debug("True - Executing '%s'", current->id().c_str());

		retval = execute(current);
	} else if (_debug)
		debug("False");

	return retval;
}

Script::ReturnCode Script::input(Shared::XMLNode *script, Shared::XMLNode *current) {
	Common::String type = getPropAsStr(current, "type");

	_currentScript = script;
	_currentItem = current;

	if (current->hasProperty("target"))
		_target = getPropAsStr(current, "target");
	else
		_target.clear();

	_state = STATE_INPUT;
	_inputName = "input";

	// Does the variable have a maximum length?
	if (current->hasProperty("maxlen"))
		_inputMaxLen = getPropAsInt(current, "maxlen");
	else
		_inputMaxLen = Conversation::BUFFERLEN;

	// Should we name the variable something other than "input"
	if (current->hasProperty("name"))
		_inputName = getPropAsStr(current, "name");
	else {
		if (type == "choice")
			_inputName = _idPropName;
	}

	if (type == "number")
		_inputType = INPUT_NUMBER;
	else if (type == "keypress")
		_inputType = INPUT_KEYPRESS;
	else if (type == "choice") {
		_inputType = INPUT_CHOICE;
		_choices = getPropAsStr(current, "options");
		_choices += " \015\033";
	} else if (type == "text")
		_inputType = INPUT_STRING;
	else if (type == "direction")
		_inputType = INPUT_DIRECTION;
	else if (type == "player")
		_inputType = INPUT_PLAYER;

	if (_debug)
		debug("Input: %s", type.c_str());

	/* the script stops here, at least for now */
	return RET_STOP;
}

Script::ReturnCode Script::add(Shared::XMLNode *script, Shared::XMLNode *current) {
	Common::String type = getPropAsStr(current, "type");
	Common::String subtype = getPropAsStr(current, "subtype");
	int quant = getPropAsInt(_translationContext.back(), "quantity");
	if (quant == 0)
		quant = getPropAsInt(current, "quantity");
	else
		quant *= getPropAsInt(current, "quantity");

	if (_debug) {
		debugN("Add: %s ", type.c_str());
		if (!subtype.empty())
			debug("- %s ", subtype.c_str());
	}

	if (type == "gold")
		g_context->_party->adjustGold(quant);
	else if (type == "food") {
		quant *= 100;
		g_context->_party->adjustFood(quant);
	} else if (type == "horse")
		g_context->_party->setTransport(g_tileSets->findTileByName("horse")->getId());
	else if (type == "torch") {
		AdjustValueMax(g_ultima->_saveGame->_torches, quant, 99);
		g_context->_party->notifyOfChange(0, PartyEvent::INVENTORY_ADDED);
	} else if (type == "gem") {
		AdjustValueMax(g_ultima->_saveGame->_gems, quant, 99);
		g_context->_party->notifyOfChange(0, PartyEvent::INVENTORY_ADDED);
	} else if (type == "key") {
		AdjustValueMax(g_ultima->_saveGame->_keys, quant, 99);
		g_context->_party->notifyOfChange(0, PartyEvent::INVENTORY_ADDED);
	} else if (type == "sextant") {
		AdjustValueMax(g_ultima->_saveGame->_sextants, quant, 99);
		g_context->_party->notifyOfChange(0, PartyEvent::INVENTORY_ADDED);
	} else if (type == "weapon") {
		AdjustValueMax(g_ultima->_saveGame->_weapons[subtype[0] - 'a'], quant, 99);
		g_context->_party->notifyOfChange(0, PartyEvent::INVENTORY_ADDED);
	} else if (type == "armor") {
		AdjustValueMax(g_ultima->_saveGame->_armor[subtype[0] - 'a'], quant, 99);
		g_context->_party->notifyOfChange(0, PartyEvent::INVENTORY_ADDED);
	} else if (type == "reagent") {
		int reagent;
		static const Common::String reagents[] = {
			"ash", "ginseng", "garlic", "silk", "moss", "pearl", "mandrake", "nightshade", ""
		};

		for (reagent = 0; reagents[reagent].size(); reagent++) {
			if (reagents[reagent] == subtype)
				break;
		}

		if (reagents[reagent].size()) {
			AdjustValueMax(g_ultima->_saveGame->_reagents[reagent], quant, 99);
			g_context->_party->notifyOfChange(0, PartyEvent::INVENTORY_ADDED);
			g_context->_stats->resetReagentsMenu();
		} else {
			warning("Error: reagent '%s' not found", subtype.c_str());
		}
	}

	if (_debug)
		debug("(x%d)", quant);

	return RET_OK;
}

Script::ReturnCode Script::lose(Shared::XMLNode *script, Shared::XMLNode *current) {
	Common::String type = getPropAsStr(current, "type");
	Common::String subtype = getPropAsStr(current, "subtype");
	int quant = getPropAsInt(current, "quantity");

	if (type == "weapon")
		AdjustValueMin(g_ultima->_saveGame->_weapons[subtype[0] - 'a'], -quant, 0);
	else if (type == "armor")
		AdjustValueMin(g_ultima->_saveGame->_armor[subtype[0] - 'a'], -quant, 0);

	if (_debug) {
		debugN("Lose: %s ", type.c_str());
		if (subtype.size())
			debug("- %s ", subtype.c_str());
		debug("(x%d)", quant);
	}

	return RET_OK;
}

Script::ReturnCode Script::heal(Shared::XMLNode *script, Shared::XMLNode *current) {
	Common::String type = getPropAsStr(current, "type");
	PartyMember *p = g_context->_party->member(getPropAsInt(current, "player") - 1);

	if (type == "cure")
		p->heal(HT_CURE);
	else if (type == "heal")
		p->heal(HT_HEAL);
	else if (type == "fullheal")
		p->heal(HT_FULLHEAL);
	else if (type == "resurrect")
		p->heal(HT_RESURRECT);

	return RET_OK;
}

Script::ReturnCode Script::castSpell(Shared::XMLNode *script, Shared::XMLNode *current) {
	g_spells->spellEffect('r', -1, SOUND_MAGIC);
	if (_debug)
		debug("<Spell effect>");

	return RET_OK;
}

Script::ReturnCode Script::damage(Shared::XMLNode *script, Shared::XMLNode *current) {
	int player = getPropAsInt(current, "player") - 1;
	int pts = getPropAsInt(current, "pts");
	PartyMember *p;

	p = g_context->_party->member(player);
	p->applyDamage(pts);

	if (_debug)
		debug("Damage: %d damage to player %d", pts, player + 1);

	return RET_OK;
}

Script::ReturnCode Script::karma(Shared::XMLNode *script, Shared::XMLNode *current) {
	Common::String action = getPropAsStr(current, "action");

	if (_debug)
		debugN("Karma: adjusting - '%s'", action.c_str());

	typedef Common::HashMap<Common::String, KarmaAction /*, Std::less<Common::String> */> KarmaActionMap;
	static KarmaActionMap action_map;

	if (action_map.size() == 0) {
		action_map["found_item"]            = KA_FOUND_ITEM;
		action_map["stole_chest"]           = KA_STOLE_CHEST;
		action_map["gave_to_beggar"]        = KA_GAVE_TO_BEGGAR;
		action_map["bragged"]               = KA_BRAGGED;
		action_map["humble"]                = KA_HUMBLE;
		action_map["hawkwind"]              = KA_HAWKWIND;
		action_map["meditation"]            = KA_MEDITATION;
		action_map["bad_mantra"]            = KA_BAD_MANTRA;
		action_map["attacked_good"]         = KA_ATTACKED_GOOD;
		action_map["fled_evil"]             = KA_FLED_EVIL;
		action_map["fled_good"]             = KA_FLED_GOOD;
		action_map["healthy_fled_evil"]     = KA_HEALTHY_FLED_EVIL;
		action_map["killed_evil"]           = KA_KILLED_EVIL;
		action_map["spared_good"]           = KA_SPARED_GOOD;
		action_map["gave_blood"]            = KA_DONATED_BLOOD;
		action_map["didnt_give_blood"]      = KA_DIDNT_DONATE_BLOOD;
		action_map["cheated_merchant"]      = KA_CHEAT_REAGENTS;
		action_map["honest_to_merchant"]    = KA_DIDNT_CHEAT_REAGENTS;
		action_map["used_skull"]            = KA_USED_SKULL;
		action_map["destroyed_skull"]       = KA_DESTROYED_SKULL;
	}

	KarmaActionMap::iterator ka = action_map.find(action);
	if (ka != action_map.end())
		g_context->_party->adjustKarma(ka->_value);
	else if (_debug)
		debug(" <FAILED - action '%s' not found>", action.c_str());

	return RET_OK;
}

Script::ReturnCode Script::music(Shared::XMLNode *script, Shared::XMLNode *current) {
	if (current->getPropertyBool("reset"))
		g_music->playMapMusic();
	else {
		Common::String type = getPropAsStr(current, "type");

		if (current->getPropertyBool("play"))
			g_music->playMapMusic();
		if (current->getPropertyBool("stop"))
			g_music->stop();
		else if (type == "shopping")
			g_music->shopping();
		else if (type == "camp")
			g_music->camp();
	}

	return RET_OK;
}

Script::ReturnCode Script::setVar(Shared::XMLNode *script, Shared::XMLNode *current) {
	Common::String name = getPropAsStr(current, "name");
	Common::String value = getPropAsStr(current, "value");

	if (name.empty()) {
		if (_debug)
			debug("Variable name empty!");
		return RET_STOP;
	}

	removeCurrentVariable(name);
	_variables[name] = new Variable(value);

	if (_debug)
		debug("Set Variable: %s=%s", name.c_str(), _variables[name]->getString().c_str());

	return RET_OK;
}

Script::ReturnCode Script::ztats(Shared::XMLNode *script, Shared::XMLNode *current) {
	typedef Common::HashMap<Common::String, StatsView/*, Std::less<Common::String>*/ > StatsViewMap;
	static StatsViewMap view_map;

	if (view_map.size() == 0) {
		view_map["party"]       = STATS_PARTY_OVERVIEW;
		view_map["party1"]      = STATS_CHAR1;
		view_map["party2"]      = STATS_CHAR2;
		view_map["party3"]      = STATS_CHAR3;
		view_map["party4"]      = STATS_CHAR4;
		view_map["party5"]      = STATS_CHAR5;
		view_map["party6"]      = STATS_CHAR6;
		view_map["party7"]      = STATS_CHAR7;
		view_map["party8"]      = STATS_CHAR8;
		view_map["weapons"]     = STATS_WEAPONS;
		view_map["armor"]       = STATS_ARMOR;
		view_map["equipment"]   = STATS_EQUIPMENT;
		view_map["item"]        = STATS_ITEMS;
		view_map["reagents"]    = STATS_REAGENTS;
		view_map["mixtures"]    = STATS_MIXTURES;
	}

	if (current->hasProperty("screen")) {
		Common::String screen = getPropAsStr(current, "screen");
		StatsViewMap::iterator view;

		if (_debug)
			debug("Ztats: %s", screen.c_str());

		/**
		 * Find the correct stats view
		 */
		view = view_map.find(screen);
		if (view != view_map.end())
			g_context->_stats->setView(view->_value); /* change it! */
		else if (_debug)
			debug(" <FAILED - view could not be found>");
	} else {
		g_context->_stats->setView(STATS_PARTY_OVERVIEW);
	}

	return RET_OK;
}

void Script::mathParseChildren(Shared::XMLNode *math, Common::String *result) {
	Shared::XMLNode *current;
	result->clear();

	for (current = math->firstChild(); current; current = current->getNext()) {
		if (current->nodeIsText()) {
			*result = getContent(current);
		} else if (current->id().equalsIgnoreCase("math")) {
			Common::String children_results;

			mathParseChildren(current, &children_results);
			*result = xu4_to_string(mathValue(children_results));
		}
	}
}

bool Script::mathParse(const Common::String &str, int *lval, int *rval, Common::String *op) {
	Common::String left, right;
	parseOperation(str, &left, &right, op);

	if (op->empty())
		return false;

	if (left.size() == 0 || right.size() == 0)
		return false;

	/* make sure that we're dealing with numbers */
	if (!Common::isDigit(left[0]) || !Common::isDigit(right[0]))
		return false;

	*lval = (int)strtol(left.c_str(), nullptr, 10);
	*rval = (int)strtol(right.c_str(), nullptr, 10);
	return true;
}

void Script::parseOperation(const Common::String &str, Common::String *left, Common::String *right, Common::String *op) {
	/* list the longest operators first, so they're detected correctly */
	static const Common::String ops[] = {"==", ">=", "<=", "+", "-", "*", "/", "%", "=", ">", "<", ""};
	int pos = 0,
	    i = 0;

	pos = str.find(ops[i]);
	while ((pos <= 0) && !ops[i].empty()) {
		i++;
		pos = str.find(ops[i]);
	}

	if (ops[i].empty()) {
		op->clear();
		return;
	} else {
		*op = ops[i];
	}

	*left = str.substr(0, pos);
	*right = str.substr(pos + ops[i].size());
}

int Script::mathValue(const Common::String &str) {
	int lval, rval;
	Common::String op;

	/* something was invalid, just return the integer value */
	if (!mathParse(str, &lval, &rval, &op))
		return (int)strtol(str.c_str(), nullptr, 10);
	else
		return math(lval, rval, op);
}

int Script::math(int lval, int rval, Common::String &op) {
	if (op == "+")
		return lval + rval;
	else if (op == "-")
		return lval - rval;
	else if (op == "*")
		return lval * rval;
	else if (op == "/")
		return lval / rval;
	else if (op == "%")
		return lval % rval;
	else if ((op == "=") || (op == "=="))
		return lval == rval;
	else if (op == ">")
		return lval > rval;
	else if (op == "<")
		return lval < rval;
	else if (op == ">=")
		return lval >= rval;
	else if (op == "<=")
		return lval <= rval;
	else
		error("Error: invalid 'math' operation attempted in vendorScript.xml");

	return 0;
}

bool Script::compare(const Common::String &statement) {
	Common::String str = statement;
	int lval, rval;
	Common::String left, right, op;
	int and_pos, or_pos;
	bool invert = false,
	     _and = false;

	/**
	 * Handle parsing of complex comparisons
	 * For example:
	 *
	 * true&&true&&true||false
	 *
	 * Since this resolves right-to-left, this would evaluate
	 * similarly to (true && (true && (true || false))), returning
	 * true.
	 */
	and_pos = str.findFirstOf("&&");
	or_pos = str.findFirstOf("||");

	if ((and_pos > 0) || (or_pos > 0)) {
		bool retfirst, retsecond;
		int pos;

		if ((or_pos < 0) || ((and_pos > 0) && (and_pos < or_pos)))
			_and = true;

		if (_and)
			pos = and_pos;
		else
			pos = or_pos;

		retsecond = compare(str.substr(pos + 2));
		str = str.substr(0, pos);
		retfirst = compare(str);

		if (_and)
			return (retfirst && retsecond);
		else
			return (retfirst || retsecond);
	}

	if (str[0] == '!') {
		str = str.substr(1);
		invert = true;
	}

	if (str == "true")
		return !invert;
	else if (str == "false")
		return invert;
	else if (mathParse(str, &lval, &rval, &op))
		return (bool)math(lval, rval, op) ? !invert : invert;
	else {
		parseOperation(str, &left, &right, &op);
		/* can only really do equality comparison */
		if ((op[0] == '=') && (left == right))
			return !invert;
	}
	return invert;
}

void Script::funcParse(const Common::String &str, Common::String *funcName, Common::String *contents) {
	uint pos;
	*funcName = str;

	pos = funcName->findFirstOf("(");
	if (pos < funcName->size()) {
		*funcName = funcName->substr(0, pos);

		*contents = str.substr(pos + 1);
		pos = contents->findFirstOf(")");
		if (pos >= contents->size())
			warning("Error: No closing ) in function %s()", funcName->c_str());
		else
			*contents = contents->substr(0, pos);
	} else {
		funcName->clear();
	}
}

} // End of namespace Ultima4
} // End of namespace Ultima
