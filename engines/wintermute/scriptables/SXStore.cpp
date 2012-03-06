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

#include "BGame.h"
#include "BRegistry.h"
#include "scriptables/SXStore.h"
#include "ScValue.h"
#include "ScScript.h"
#include "ScStack.h"
#include "StringUtil.h"

#ifdef __IPHONEOS__
#   include "IOS_StoreKit_interface.h"
#endif

namespace WinterMute {

IMPLEMENT_PERSISTENT(CSXStore, false)

//////////////////////////////////////////////////////////////////////////
CSXStore::CSXStore(CBGame *inGame) : CBObject(inGame) {
#ifdef __IPHONEOS__
	StoreKit_SetExternalData((void *)this);
#endif

	m_EventsEnabled = false;
	m_LastProductRequestOwner = NULL;
	m_LastPurchaseOwner = NULL;
	m_LastRestoreOwner = NULL;
}


//////////////////////////////////////////////////////////////////////////
CSXStore::~CSXStore() {
	Cleanup();
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::Cleanup() {
	SetEventsEnabled(NULL, false);

	for (int i = 0; i < m_ValidProducts.GetSize(); i++) {
		delete m_ValidProducts[i];
	}
	m_ValidProducts.RemoveAll();


	for (int i = 0; i < m_Transactions.GetSize(); i++) {
		delete m_Transactions[i];
	}
	m_Transactions.RemoveAll();


	m_LastProductRequestOwner = m_LastPurchaseOwner = m_LastRestoreOwner = NULL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CSXStore::ScCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, char *name) {
	//////////////////////////////////////////////////////////////////////////
	// EnableEvents
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "EnableEvents") == 0) {
		stack->CorrectParams(0);
		SetEventsEnabled(script, true);
		stack->PushBool(GetEventsEnabled() == true);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// DisableEvents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DisableEvents") == 0) {
		stack->CorrectParams(0);
		SetEventsEnabled(script, false);
		stack->PushBool(GetEventsEnabled() == false);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// ValidateProducts
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ValidateProducts") == 0) {
		stack->CorrectParams(1);
		char *prodIdList = stack->Pop()->GetString();
		m_LastProductRequestOwner = script->m_Owner;
		ValidateProducts(prodIdList);
		stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetValidProduct
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetValidProduct") == 0) {
		stack->CorrectParams(1);
		int index = stack->Pop()->GetInt();
		if (index >= 0 && index < m_ValidProducts.GetSize()) {
			CScValue *prod = stack->GetPushValue();
			if (prod) {
				prod->SetProperty("Id", m_ValidProducts[index]->GetId());
				prod->SetProperty("Name", m_ValidProducts[index]->GetName());
				prod->SetProperty("Description", m_ValidProducts[index]->GetDesc());
				prod->SetProperty("Price", m_ValidProducts[index]->GetPrice());
			}
		} else
			stack->PushNULL();

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetInvalidProduct
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetInvalidProduct") == 0) {
		stack->CorrectParams(1);
		int index = stack->Pop()->GetInt();
		if (index >= 0 && index < m_InvalidProducts.size())
			stack->PushString(m_InvalidProducts[index].c_str());
		else
			stack->PushNULL();

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetTransaction
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetTransaction") == 0) {
		stack->CorrectParams(1);
		int index = stack->Pop()->GetInt();
		if (index >= 0 && index < m_Transactions.GetSize()) {
			CScValue *trans = stack->GetPushValue();
			if (trans) {
				trans->SetProperty("Id", m_Transactions[index]->GetId());
				trans->SetProperty("ProductId", m_Transactions[index]->GetProductId());
				trans->SetProperty("State", m_Transactions[index]->GetState());
			}
		} else
			stack->PushNULL();

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Purchase
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Purchase") == 0) {
		stack->CorrectParams(1);
		char *prodId = stack->Pop()->GetString();
		stack->PushBool(Purchase(script, prodId));

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// FinishTransaction
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FinishTransaction") == 0) {
		stack->CorrectParams(1);
		char *transId = stack->Pop()->GetString();
		stack->PushBool(FinishTransaction(script, transId));

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// RestoreTransactions
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RestoreTransactions") == 0) {
		stack->CorrectParams(0);
		RestoreTransactions(script);
		stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnlockProduct
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UnlockProduct") == 0) {
		stack->CorrectParams(1);
		char *prodId = stack->Pop()->GetString();

		Game->m_Registry->WriteBool("Purchases", prodId, true);
		Game->m_Registry->SaveValues();

		stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsProductUnlocked
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsProductUnlocked") == 0) {
		stack->CorrectParams(1);
		char *prodId = stack->Pop()->GetString();

		stack->PushBool(Game->m_Registry->ReadBool("Purchases", prodId, false));

		return S_OK;
	}

	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXStore::ScGetProperty(char *name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		m_ScValue->SetString("store");
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Available (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Available") == 0) {
		m_ScValue->SetBool(IsAvailable());
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// EventsEnabled (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EventsEnabled") == 0) {
		m_ScValue->SetBool(GetEventsEnabled());
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumValidProducts (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumValidProducts") == 0) {
		m_ScValue->SetInt(m_ValidProducts.GetSize());
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumInvalidProducts (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumInvalidProducts") == 0) {
		m_ScValue->SetInt(m_InvalidProducts.size());
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumTransactions (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumTransactions") == 0) {
		m_ScValue->SetInt(m_Transactions.GetSize());
		return m_ScValue;
	}

	else return m_ScValue;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXStore::Persist(CBPersistMgr *PersistMgr) {
	if (!PersistMgr->m_Saving) Cleanup();

	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_EventsEnabled));
	PersistMgr->Transfer(TMEMBER(m_LastProductRequestOwner));
	PersistMgr->Transfer(TMEMBER(m_LastPurchaseOwner));
	PersistMgr->Transfer(TMEMBER(m_LastRestoreOwner));
	PersistMgr->Transfer(TMEMBER(m_InvalidProducts));

	// persist valid products
	int numProducts;
	if (PersistMgr->m_Saving) {
		numProducts = m_ValidProducts.GetSize();
		PersistMgr->Transfer(TMEMBER(numProducts));
		for (int i = 0; i < numProducts; i++) m_ValidProducts[i]->Persist(PersistMgr);
	} else {
		numProducts = m_ValidProducts.GetSize();
		PersistMgr->Transfer(TMEMBER(numProducts));
		for (int i = 0; i < numProducts; i++) {
			CBStoreProduct *prod = new CBStoreProduct;
			prod->Persist(PersistMgr);
			m_ValidProducts.Add(prod);
		}
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CSXStore::AfterLoad() {
	if (m_EventsEnabled) {
		SetEventsEnabled(NULL, true);
	}
#ifdef __IPHONEOS__
	StoreKit_SetExternalData((void *)this);
#endif
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::OnObjectDestroyed(CBScriptHolder *obj) {
	if (m_LastProductRequestOwner == obj) m_LastProductRequestOwner = NULL;
	if (m_LastPurchaseOwner == obj) m_LastPurchaseOwner = NULL;
	if (m_LastRestoreOwner == obj) m_LastRestoreOwner = NULL;
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::SetEventsEnabled(CScScript *script, bool val) {
	m_EventsEnabled = val;

	if (val) {
		if (script) m_LastPurchaseOwner = script->m_Owner;
#ifdef __IPHONEOS__
		StoreKit_EnableEvents();
#endif
	} else {
		m_LastPurchaseOwner = NULL;
#ifdef __IPHONEOS__
		StoreKit_DisableEvents();
#endif
	}
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::ValidateProducts(const char *prodIdList) {
#ifdef __IPHONEOS__
	StoreKit_ValidateProducts(prodIdList);
#endif
}

//////////////////////////////////////////////////////////////////////////
bool CSXStore::IsAvailable() {
#ifdef __IPHONEOS__
	return StoreKit_IsStoreAvailable() > 0;
#else
	return false;
#endif
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::ReceiveProductsStart() {
	for (int i = 0; i < m_ValidProducts.GetSize(); i++) {
		delete m_ValidProducts[i];
	}
	m_ValidProducts.RemoveAll();

	m_InvalidProducts.clear();
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::ReceiveProductsEnd() {
	if (m_LastProductRequestOwner) m_LastProductRequestOwner->ApplyEvent("ProductsValidated");
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::AddValidProduct(const char *id, const char *name, const char *desc, const char *price) {
	CBStoreProduct *prod = new CBStoreProduct(id, name, desc, price);
	m_ValidProducts.Add(prod);
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::AddInvalidProduct(const char *id) {
	m_InvalidProducts.push_back(id);
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::ReceiveTransactionsStart() {
	for (int i = 0; i < m_Transactions.GetSize(); i++) {
		delete m_Transactions[i];
	}
	m_Transactions.RemoveAll();
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::ReceiveTransactionsEnd() {
	if (m_LastPurchaseOwner) m_LastPurchaseOwner->ApplyEvent("TransactionsUpdated");
	else Game->ApplyEvent("TransactionsUpdated");
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::AddTransaction(const char *id, const char *productId, const char *state) {
	CBStoreTransaction *trans = new CBStoreTransaction(id, productId, state);
	m_Transactions.Add(trans);
}

//////////////////////////////////////////////////////////////////////////
bool CSXStore::Purchase(CScScript *script, const char *productId) {
	if (!productId) return false;

#ifdef __IPHONEOS__
	for (int i = 0; i < m_ValidProducts.GetSize(); i++) {
		if (strcmp(productId, m_ValidProducts[i]->GetId()) == 0) {
			m_LastPurchaseOwner = script->m_Owner;

			StoreKit_Purchase(productId);
			return true;
		}
	}
#endif
	script->RuntimeError("Store.Purchase() - '%s' is not a valid product id", productId);
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CSXStore::FinishTransaction(CScScript *script, const char *transId) {
	if (!transId) return false;
#ifdef __IPHONEOS__
	for (int i = 0; i < m_Transactions.GetSize(); i++) {
		if (strcmp(transId, m_Transactions[i]->GetId()) == 0) {
			if (StoreKit_FinishTransaction(transId) > 0) {
				SAFE_DELETE(m_Transactions[i]);
				m_Transactions.RemoveAt(i);
				return true;
			} else return false;
		}
	}
#endif
	script->RuntimeError("Store.FinishTransaction() - '%s' is not a valid transaction id", transId);
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::RestoreTransactions(CScScript *script) {
	m_LastRestoreOwner = script->m_Owner;
#ifdef __IPHONEOS__
	StoreKit_RestoreTransactions();
#endif
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::OnRestoreFinished(bool error) {
	if (m_LastRestoreOwner) {
		if (error) m_LastRestoreOwner->ApplyEvent("TransactionsRestoreFailed");
		else m_LastRestoreOwner->ApplyEvent("TransactionsRestoreFinished");
	}
}



#ifdef __IPHONEOS__

//////////////////////////////////////////////////////////////////////////
// StoreKit callbacks (called from ObjC)
//////////////////////////////////////////////////////////////////////////
void StoreKit_AddValidProductCallback(const char *id, const char *name, const char *desc, const char *price, void *data) {
	CSXStore *store = static_cast<CSXStore *>(data);
	if (store) store->AddValidProduct(id, name, desc, price);
}

//////////////////////////////////////////////////////////////////////////
void StoreKit_AddInvalidProductCallback(const char *id, void *data) {
	CSXStore *store = static_cast<CSXStore *>(data);
	if (store) store->AddInvalidProduct(id);
}

//////////////////////////////////////////////////////////////////////////
void StoreKit_ReceiveProductsStartCallback(void *data) {
	CSXStore *store = static_cast<CSXStore *>(data);
	if (store) store->ReceiveProductsStart();
}

//////////////////////////////////////////////////////////////////////////
void StoreKit_ReceiveProductsEndCallback(void *data) {
	CSXStore *store = static_cast<CSXStore *>(data);
	if (store) store->ReceiveProductsEnd();
}

//////////////////////////////////////////////////////////////////////////
void StoreKit_AddTransactionCallback(const char *id, const char *productId, const char *state, void *data) {
	CSXStore *store = static_cast<CSXStore *>(data);
	if (store) store->AddTransaction(id, productId, state);
}

//////////////////////////////////////////////////////////////////////////
void StoreKit_ReceiveTransactionsStartCallback(void *data) {
	CSXStore *store = static_cast<CSXStore *>(data);
	if (store) store->ReceiveTransactionsStart();
}

//////////////////////////////////////////////////////////////////////////
void StoreKit_ReceiveTransactionsEndCallback(void *data) {
	CSXStore *store = static_cast<CSXStore *>(data);
	if (store) store->ReceiveTransactionsEnd();
}
//////////////////////////////////////////////////////////////////////////
void StoreKit_RestoreFinishedCallback(void *data, int error) {
	CSXStore *store = static_cast<CSXStore *>(data);
	if (store) store->OnRestoreFinished(error > 0);
}

#endif // __IPHONEOS__

} // end of namespace WinterMute