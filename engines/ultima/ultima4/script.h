/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA4_SCRIPT_H
#define ULTIMA4_SCRIPT_H

#include "ultima/ultima4/types.h"
#include "ultima/ultima4/xml.h"
#include "ultima/shared/std/containers.h"
#include "common/file.h"

namespace Ultima {
namespace Ultima4 {

#ifdef random
#undef random
#endif

/**
 * An xml-scripting class. It loads and runs xml scripts that
 * take information and interact with the game environment itself.
 * Currently, it is mainly useful for writing vendor code; however,
 * it should be possible to write scripts for other parts of the
 * game.
 *
 * @todo
 * <ul>
 *      <li>Strip vendor-specific code from the language</li>
 *      <li>Fill in some of the missing integration with the game</li>
 * </ul>
 */ 
class Script {
public:    
    /**
     * A class that provides information to a script.  It is designed to
     * translate qualifiers and identifiers in a script to another value.
     * Each provider is assigned a qualifier that the script uses to
     * select a provider.  The provider then uses the rest of the information
     * to translate the information to something useful.
     */
    class Provider {
    public:
        virtual ~Provider() {}
        virtual Common::String translate(Std::vector<Common::String> &parts) = 0;
    };

private:
    /**
     * A class that represents a script variable
     */ 
    class Variable {
    public:
        Variable();
        Variable(const Common::String &v);
        Variable(const int &v);

        int&    getInt();
        Common::String& getString();
        
        void    setValue(const int &v);
        void    setValue(const Common::String &v);
        void    unset();

        bool    isInt() const;
        bool    isString() const;
        bool    isSet() const;
    
    private:
        int _iVal;
        Common::String _sVal;
        bool _set;
    };

public:
    /**
     * A script return code
     */ 
    enum ReturnCode {
        RET_OK,
        RET_REDIRECTED,
        RET_STOP
    };

    /**
     * The current state of the script
     */ 
    enum State {
        STATE_UNLOADED,
        STATE_NORMAL,        
        STATE_DONE,        
        STATE_INPUT
    };

    /**
     * The type of input the script is requesting
     */ 
    enum InputType {
        INPUT_CHOICE, 
        INPUT_NUMBER,
        INPUT_STRING,
        INPUT_DIRECTION,
        INPUT_PLAYER,
        INPUT_KEYPRESS
    };

    /**
     * The action that the script is taking
     */ 
    enum Action {
        ACTION_SET_CONTEXT,
        ACTION_UNSET_CONTEXT,
        ACTION_END,
        ACTION_REDIRECT,
        ACTION_WAIT_FOR_KEY,
        ACTION_WAIT,
        ACTION_STOP,
        ACTION_INCLUDE,
        ACTION_FOR_LOOP,
        ACTION_RANDOM,
        ACTION_MOVE,
        ACTION_SLEEP,
        ACTION_CURSOR,
        ACTION_PAY,
        ACTION_IF,
        ACTION_INPUT,
        ACTION_ADD,
        ACTION_LOSE,
        ACTION_HEAL,
        ACTION_CAST_SPELL,
        ACTION_DAMAGE,
        ACTION_KARMA,
        ACTION_MUSIC,
        ACTION_SET_VARIABLE,        
        ACTION_ZTATS
    };

    Script();
    ~Script();

    void addProvider(const Common::String &name, Provider *p);
    bool load(const Common::String &filename, const Common::String &baseId, const Common::String &subNodeName = "", const Common::String &subNodeId = "");
    void unload();
    void run(const Common::String &script);
    ReturnCode execute(xmlNodePtr script, xmlNodePtr currentItem = NULL, Common::String *output = NULL);    
    void _continue();
    
    void resetState();
    void setState(State state);
    State getState();    
    
    void setTarget(const Common::String &val);
    void setChoices(const Common::String &val);
    void setVar(const Common::String &name, const Common::String &val);
    void setVar(const Common::String &name, int val);
    void unsetVar(const Common::String &name);
    
    Common::String getTarget();
    InputType getInputType();
    Common::String getInputName();
    Common::String getChoices();
    int getInputMaxLen();
    
private:
    void        translate(Common::String *script);
    xmlNodePtr  find(xmlNodePtr node, const Common::String &script, const Common::String &choice = "", bool _default = false);    
    Common::String      getPropAsStr(Std::list<xmlNodePtr> &nodes, const Common::String &prop, bool recursive);
    Common::String      getPropAsStr(xmlNodePtr node, const Common::String &prop, bool recursive = false);
    int         getPropAsInt(Std::list<xmlNodePtr> &nodes, const Common::String &prop, bool recursive);
    int         getPropAsInt(xmlNodePtr node, const Common::String &prop, bool recursive = false);
    Common::String      getContent(xmlNodePtr node);   

    /*
     * Action Functions
     */     
    ReturnCode pushContext(xmlNodePtr script, xmlNodePtr current);
    ReturnCode popContext(xmlNodePtr script, xmlNodePtr current);
    ReturnCode end(xmlNodePtr script, xmlNodePtr current);
    ReturnCode waitForKeypress(xmlNodePtr script, xmlNodePtr current);
    ReturnCode redirect(xmlNodePtr script, xmlNodePtr current);
    ReturnCode include(xmlNodePtr script, xmlNodePtr current);
    ReturnCode wait(xmlNodePtr script, xmlNodePtr current);
    ReturnCode forLoop(xmlNodePtr script, xmlNodePtr current);
    ReturnCode random(xmlNodePtr script, xmlNodePtr current);
    ReturnCode move(xmlNodePtr script, xmlNodePtr current);
    ReturnCode sleep(xmlNodePtr script, xmlNodePtr current);
    ReturnCode cursor(xmlNodePtr script, xmlNodePtr current);
    ReturnCode pay(xmlNodePtr script, xmlNodePtr current);
    ReturnCode _if(xmlNodePtr script, xmlNodePtr current);
    ReturnCode input(xmlNodePtr script, xmlNodePtr current);
    ReturnCode add(xmlNodePtr script, xmlNodePtr current);
    ReturnCode lose(xmlNodePtr script, xmlNodePtr current);
    ReturnCode heal(xmlNodePtr script, xmlNodePtr current);
    ReturnCode castSpell(xmlNodePtr script, xmlNodePtr current);
    ReturnCode damage(xmlNodePtr script, xmlNodePtr current);
    ReturnCode karma(xmlNodePtr script, xmlNodePtr current);
    ReturnCode music(xmlNodePtr script, xmlNodePtr current);    
    ReturnCode setVar(xmlNodePtr script, xmlNodePtr current);
    ReturnCode setId(xmlNodePtr script, xmlNodePtr current);
    ReturnCode ztats(xmlNodePtr script, xmlNodePtr current);

    /*
     * Math and comparison functions
     */
    void mathParseChildren(xmlNodePtr math, Common::String *result);
    int mathValue(const Common::String &str);
    int math(int lval, int rval, Common::String &op);
    bool mathParse(const Common::String &str, int *lval, int *rval, Common::String *op);
    void parseOperation(const Common::String &str, Common::String *lval, Common::String *rval, Common::String *op);
    bool compare(const Common::String &str);
    void funcParse(const Common::String &str, Common::String *funcName, Common::String *contents);

    /*
     * Static variables
     */
private:
    typedef Std::map<Common::String, Action> ActionMap;
    static ActionMap _actionMap;

private:
    void removeCurrentVariable(const Common::String &name);
    xmlDocPtr _vendorScriptDoc;
    xmlNodePtr _scriptNode;
    Common::DumpFile *_debug;
    
    State _state;                    /**< The state the script is in */
    xmlNodePtr _currentScript;       /**< The currently running script */
    xmlNodePtr _currentItem;         /**< The current position in the script */
    Std::list<xmlNodePtr> _translationContext;  /**< A list of nodes that make up our translation context */
    Common::String _target;                  /**< The name of a target script */
    InputType _inputType;            /**< The type of input required */
    Common::String _inputName;               /**< The variable in which to place the input (by default, "input") */
    int _inputMaxLen;                /**< The maximum length allowed for input */

    Common::String _nounName;                /**< The name that identifies a node name of noun nodes */
    Common::String _idPropName;              /**< The name of the property that uniquely identifies a noun node
                                         and is used to find a new translation context */
    
    Common::String _choices;
    int _iterator;   

    Std::map<Common::String, Variable*> _variables;    
    Std::map<Common::String, Provider*> _providers;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
