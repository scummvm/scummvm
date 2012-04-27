/*
This file is part of WME Lite.
http://dead-code.org/redir.php?target=wmelite

Copyright (c) 2011 Jan Nedoma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef __WmeSXStore_H__
#define __WmeSXStore_H__
#include "engines/wintermute/BPersistMgr.h"
#include "engines/wintermute/utils.h"
#include "engines/wintermute/BObject.h"

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

			CBUtils::SetString(&_id, id);
			CBUtils::SetString(&_name, name);
			CBUtils::SetString(&_desc, desc);
			CBUtils::SetString(&_price, price);
		}

		~CBStoreProduct() {
			delete [] _id;
			delete [] _name;
			delete [] _desc;
			delete [] _price;
		}

		HRESULT Persist(CBPersistMgr *PersistMgr) {
			PersistMgr->Transfer(TMEMBER(_id));
			PersistMgr->Transfer(TMEMBER(_name));
			PersistMgr->Transfer(TMEMBER(_desc));
			PersistMgr->Transfer(TMEMBER(_price));
			return S_OK;
		}

		const char *GetId() {
			return _id;
		}
		const char *GetName() {
			return _name;
		}
		const char *GetDesc() {
			return _desc;
		}
		const char *GetPrice() {
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

			CBUtils::SetString(&_id, id);
			CBUtils::SetString(&_productId, productId);
			CBUtils::SetString(&_state, state);
		}

		~CBStoreTransaction() {
			delete [] _id;
			delete [] _productId;
			delete [] _state;
		}

		const char *GetId() {
			return _id;
		}
		const char *GetProductId() {
			return _productId;
		}
		const char *GetState() {
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
	virtual CScValue *ScGetProperty(char *name);
	virtual HRESULT ScCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, char *name);

	void AfterLoad();
	void OnObjectDestroyed(CBScriptHolder *obj);

	bool IsAvailable();
	void SetEventsEnabled(CScScript *script, bool val);
	bool GetEventsEnabled() const {
		return _eventsEnabled;
	}
	void ValidateProducts(const char *prodIdList);

	void ReceiveTransactionsStart();
	void ReceiveTransactionsEnd();
	void AddTransaction(const char *id, const char *productId, const char *state);

	void ReceiveProductsStart();
	void ReceiveProductsEnd();
	void AddValidProduct(const char *id, const char *name, const char *desc, const char *price);
	void AddInvalidProduct(const char *id);

	void OnRestoreFinished(bool error);

private:
	void Cleanup();
	bool Purchase(CScScript *script, const char *productId);
	bool FinishTransaction(CScScript *script, const char *transId);
	void RestoreTransactions(CScScript *script);

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
