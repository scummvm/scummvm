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
			m_Id = m_Name = m_Desc = m_Price = NULL;
		}

		CBStoreProduct(const char *id, const char *name, const char *desc, const char *price) {
			m_Id = m_Name = m_Desc = m_Price = NULL;

			CBUtils::SetString(&m_Id, id);
			CBUtils::SetString(&m_Name, name);
			CBUtils::SetString(&m_Desc, desc);
			CBUtils::SetString(&m_Price, price);
		}

		~CBStoreProduct() {
			delete [] m_Id;
			delete [] m_Name;
			delete [] m_Desc;
			delete [] m_Price;
		}

		HRESULT Persist(CBPersistMgr *PersistMgr) {
			PersistMgr->Transfer(TMEMBER(m_Id));
			PersistMgr->Transfer(TMEMBER(m_Name));
			PersistMgr->Transfer(TMEMBER(m_Desc));
			PersistMgr->Transfer(TMEMBER(m_Price));
			return S_OK;
		}

		const char *GetId() {
			return m_Id;
		}
		const char *GetName() {
			return m_Name;
		}
		const char *GetDesc() {
			return m_Desc;
		}
		const char *GetPrice() {
			return m_Price;
		}

	private:
		char *m_Id;
		char *m_Name;
		char *m_Desc;
		char *m_Price;
	};

	//////////////////////////////////////////////////////////////////////////
	class CBStoreTransaction {
	public:
		CBStoreTransaction() {
			m_Id = m_ProductId = m_State = NULL;
		}

		CBStoreTransaction(const char *id, const char *productId, const char *state) {
			m_Id = m_ProductId = m_State = NULL;

			CBUtils::SetString(&m_Id, id);
			CBUtils::SetString(&m_ProductId, productId);
			CBUtils::SetString(&m_State, state);
		}

		~CBStoreTransaction() {
			delete [] m_Id;
			delete [] m_ProductId;
			delete [] m_State;
		}

		const char *GetId() {
			return m_Id;
		}
		const char *GetProductId() {
			return m_ProductId;
		}
		const char *GetState() {
			return m_State;
		}

	private:
		char *m_Id;
		char *m_ProductId;
		char *m_State;
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
		return m_EventsEnabled;
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

	bool m_EventsEnabled;
	CBArray<CBStoreProduct *, CBStoreProduct *> m_ValidProducts;
	AnsiStringArray m_InvalidProducts;
	CBScriptHolder *m_LastProductRequestOwner;
	CBScriptHolder *m_LastPurchaseOwner;
	CBScriptHolder *m_LastRestoreOwner;

	CBArray<CBStoreTransaction *, CBStoreTransaction *> m_Transactions;

};

} // end of namespace WinterMute

#endif
