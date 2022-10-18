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

#ifndef ULTIMA4_CONVERSATION_CONVERSATION_H
#define ULTIMA4_CONVERSATION_CONVERSATION_H

#include "ultima/ultima4/core/utils.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

class Debug;
class Person;
class Script;

/**
 * A response part can be text or a "command" that triggers an
 * action.
 */
class ResponsePart {
public:
	ResponsePart(const Common::String &value, const Common::String &arg = "", bool command = false);

	operator Common::String() const;
	bool operator==(const ResponsePart &rhs) const;
	bool isCommand() const;

private:
	Common::String _value, _arg;
	bool _command;
};

/**
 * A list of valid command response parts
 */
class ResponseParts {
public:
	const ResponsePart NONE;
	const ResponsePart ASK;
	const ResponsePart END;
	const ResponsePart ATTACK;
	const ResponsePart BRAGGED;
	const ResponsePart HUMBLE;
	const ResponsePart ADVANCELEVELS;
	const ResponsePart HEALCONFIRM;
	const ResponsePart STARTMUSIC_LB;
	const ResponsePart STARTMUSIC_HW;
	const ResponsePart STOPMUSIC;
	const ResponsePart HAWKWIND;

	ResponseParts();
	~ResponseParts();
};

extern ResponseParts *g_responseParts;

/**
 * A static response.  Each response can be made up of any number of
 * ResponseParts, which are either text fragments or commands.
 */
class Response {
public:
	Response(const Common::String &response);
	virtual ~Response() {}

	void add(const ResponsePart &part);

	virtual const Std::vector<ResponsePart> &getParts() const;

	operator Common::String() const;

	Response *addref();
	void release();

private:
	int _references;
	Std::vector<ResponsePart> _parts;
};

/**
 * A dynamically generated response.  This class allows the response
 * to be generated dynamically at the time of the conversation instead
 * of when the conversation data is loaded.
 */
class DynamicResponse : public Response {
public:
	DynamicResponse(Response * (*generator)(const DynamicResponse *), const Common::String &param = "");
	virtual ~DynamicResponse();

	const Std::vector<ResponsePart> &getParts() const override;

	const Common::String &getParam() const {
		return _param;
	}

private:
	Response *(*_generator)(const DynamicResponse *);
	Response *_currentResponse;
	Common::String _param;
};

/**
 * The dialogue class, which holds conversation information for
 * townspeople and others who may talk to you.  It includes information
 * like pronouns, keywords, actual conversation text (of course),
 * questions, and what happens when you answer these questions.
 */
class Dialogue {
public:
	/**
	 * A question-response to a keyword.
	 */
	class Question {
	public:
		Question(const Common::String &txt, Response *yes, Response *no);

		Common::String getText();
		Response *getResponse(bool yes);

	private:
		Common::String _text;
		Response *_yesResp, *_noResp;
	};

	/**
	 * A dialogue keyword.
	 * It contains all the keywords that the talker will respond to, as
	 * well as the responses to those keywords.
	 */
	class Keyword {
	public:
		Keyword(const Common::String &kw, Response *resp);
		Keyword(const Common::String &kw, const Common::String &resp);
		~Keyword();

		bool operator==(const Common::String &kw) const;

		/*
		 * Accessor methods
		 */
		const Common::String &getKeyword() {
			return _keyword;
		}
		Response *getResponse() {
			return _response;
		}

	private:
		Common::String _keyword;
		Response *_response;
	};

	/**
	 * A mapping of keywords to the Keyword object that represents them
	 */
	typedef Common::HashMap<Common::String, Keyword *> KeywordMap;

	/*
	 * Constructors/Destructors
	 */
	Dialogue();
	virtual ~Dialogue();

	/*
	 * Accessor methods
	 */
	const Common::String &getName() const {
		return _name;
	}
	const Common::String &getPronoun() const {
		return _pronoun;
	}
	const Common::String &getPrompt() const {
		return _prompt;
	}
	Response *getIntro(bool familiar = false) {
		return _intro;
	}
	Response *getLongIntro(bool familiar = false) {
		return _longIntro;
	}
	Response *getDefaultAnswer() {
		return _defaultAnswer;
	}
	Dialogue::Question *getQuestion() {
		return _question;
	}

	/*
	 * Getters
	 */
	void setName(const Common::String &n) {
		_name           = n;
	}
	void setPronoun(const Common::String &pn) {
		_pronoun        = pn;
	}
	void setPrompt(const Common::String &prompt) {
		this->_prompt   = prompt;
	}
	void setIntro(Response *i) {
		_intro          = i;
	}
	void setLongIntro(Response *i) {
		_longIntro      = i;
	}
	void setDefaultAnswer(Response *a) {
		_defaultAnswer  = a;
	}
	void setTurnAwayProb(int prob) {
		_turnAwayProb   = prob;
	}
	void setQuestion(Question *q) {
		_question       = q;
	}
	void addKeyword(const Common::String &kw, Response *response);

	const ResponsePart &getAction() const;
	Common::String dump(const Common::String &arg);

	/*
	 * Operators
	 */
	Keyword *operator[](const Common::String &kw);

private:
	Common::String _name;
	Common::String _pronoun;
	Common::String _prompt;
	Response *_intro;
	Response *_longIntro;
	Response *_defaultAnswer;
	KeywordMap _keywords;
	union {
		int _turnAwayProb;
		int _attackProb;
	};
	Question *_question;
};

/**
 * The conversation class, which handles the flow of text from the
 * player to the talker and vice-versa.  It is responsible for beginning
 * and termination conversations and handing state changes during.
 */
class Conversation {
public:
	/** Different states the conversation may be in */
	enum State {
		INTRO,                  /**< The initial state of the conversation, before anything is said */
		TALK,                   /**< The "default" state of the conversation */
		ASK,                    /**< The talker is asking the player a question */
		ASKYESNO,               /**< The talker is asking the player a yes/no question */
		VENDORQUESTION,         /**< A vendor is asking the player a question */
		BUY_ITEM,               /**< Asked which item to buy */
		SELL_ITEM,              /**< Asked which item to sell */
		BUY_QUANTITY,           /**< Asked how many items to buy */
		SELL_QUANTITY,          /**< Asked how many items to sell */
		BUY_PRICE,              /**< Asked how much money to give someone */
		CONFIRMATION,           /**< Asked by a vendor to confirm something */
		CONTINUEQUESTION,       /**< Asked whether or not to continue */
		TOPIC,                  /**< Asked a topic to speak about */
		PLAYER,                 /**< Input for which player is required */
		FULLHEAL,               /**< Heal the entire party before continuing conversation */
		ADVANCELEVELS,          /**< Check and advance the party's levels before continuing */
		GIVEBEGGAR,             /**< Asked how much to give a beggar */
		ATTACK,                 /**< The conversation ends with the talker attacking you */
		DONE                    /**< The conversation is over */
	};

	/** Different types of conversation input required */
	enum InputType {
		INPUT_STRING,
		INPUT_CHARACTER,
		INPUT_NONE
	};

	/* Constructor/Destructors */
	Conversation();
	~Conversation();

	/* Member functions */
	InputType getInputRequired(int *bufferLen);

	/* Static variables */
	static const uint BUFFERLEN;    /**< The default maxixum length of input */

public:
	State _state;                /**< The state of the conversation */
	Common::String _playerInput;         /**< A Common::String holding the text the player inputs */
	Common::List<Common::String> _reply;         /**< What the talker says */
	class Script *_script;       /**< A script that this person follows during the conversation (may be nullptr) */
	Dialogue::Question *_question; /**< The current question the player is being asked */
	int _quant;                  /**< For vendor transactions */
	int _player;                 /**< For vendor transactions */
	int _price;                  /**< For vendor transactions */
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
