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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_SXSTORE_H
#define WINTERMUTE_SXSTORE_H
#include "engines/wintermute/Base/BPersistMgr.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/Base/BObject.h"

namespace WinterMute {

class CSXStore : public CBObject {
public:

	//////////////////////////////////////////////////////////////////////////
	class CBStoreProduct {
	public:
		CBStoreProduct() {
			_id = _name = _desc = _price = NULL;
		}

		CBStoreProduct(const char *id, const char *name, const char *desc, const char *price) {
			_id = _name = _desc = _price = NULL;

			CBUtils::setString(&_id, id);
			CBUtils::setString(&_name, name);
			CBUtils::setString(&_desc, desc);
			CBUtils::setString(&_price, price);
		}

		~CBStoreProduct() {
			delete [] _id;
			delete [] _name;
			delete [] _desc;
			delete [] _price;
		}

		ERRORCODE persist(CBPersistMgr *persistMgr) {
			persistMgr->transfer(TMEMBER(_id));
			persistMgr->transfer(TMEMBER(_name));
			persistMgr->transfer(TMEMBER(_desc));
			persistMgr->transfer(TMEMBER(_price));
			return STATUS_OK;
		}

		const char *getId() {
			return _id;
		}
		const char *getName() {
			return _name;
		}
		const char *getDesc() {
			return _desc;
		}
		const char *getPrice() {
			return _price;
		}

	private:
		char *_id;
		char *_name;
		char *_desc;
		char *_price;
	};

	//////////////////////////////////////////////////////////////////////////
	class CBStoreTransaction {
	public:
		CBStoreTransaction() {
			_id = _productId = _state = NULL;
		}

		CBStoreTransaction(const char *id, const char *productId, const char *state) {
			_id = _productId = _state = NULL;

			CBUtils::setString(&_id, id);
			CBUtils::setString(&_productId, productId);
			CBUtils::setString(&_state, state);
		}

		~CBStoreTransaction() {
			delete [] _id;
			delete [] _productId;
			delete [] _state;
		}

		const char *getId() {
			return _id;
		}
		const char *getProductId() {
			return _productId;
		}
		const char *getState() {
			return _state;
		}

	private:
		char *_id;
		char *_productId;
		char *_state;
	};


	DECLARE_PERSISTENT(CSXStore, CBObject)
	CSXStore(CBGame *inGame);
	virtual ~CSXStore();
	virtual CScValue *scGetProperty(const char *name);
	virtual ERRORCODE scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);

	void afterLoad();
	void OnObjectDestroyed(CBScriptHolder *obj);

	bool isAvailable();
	void setEventsEnabled(CScScript *script, bool val);
	bool getEventsEnabled() const {
		return _eventsEnabled;
	}
	void validateProducts(const char *prodIdList);

	void receiveTransactionsStart();
	void receiveTransactionsEnd();
	void addTransaction(const char *id, const char *productId, const char *state);

	void receiveProductsStart();
	void receiveProductsEnd();
	void addValidProduct(const char *id, const char *name, const char *desc, const char *price);
	void addInvalidProduct(const char *id);

	void onRestoreFinished(bool error);

private:
	void cleanup();
	bool purchase(CScScript *script, const char *productId);
	bool finishTransaction(CScScript *script, const char *transId);
	void restoreTransactions(CScScript *script);

	bool _eventsEnabled;
	CBArray<CBStoreProduct *, CBStoreProduct *> _validProducts;
	AnsiStringArray _invalidProducts;
	CBScriptHolder *_lastProductRequestOwner;
	CBScriptHolder *_lastPurchaseOwner;
	CBScriptHolder *_lastRestoreOwner;

	CBArray<CBStoreTransaction *, CBStoreTransaction *> _transactions;

};

} // end of namespace WinterMute

#endif
