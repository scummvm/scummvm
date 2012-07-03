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

#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BRegistry.h"
#include "engines/wintermute/Base/scriptables/SXStore.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/utils/StringUtil.h"

#ifdef __IPHONEOS__
#   include "IOS_StoreKit_interface.h"
#endif

namespace WinterMute {

IMPLEMENT_PERSISTENT(CSXStore, false)

CBScriptable *makeSXStore(CBGame *inGame, CScStack *stack) {
	return new CSXStore(inGame);
}

//////////////////////////////////////////////////////////////////////////
CSXStore::CSXStore(CBGame *inGame) : CBObject(inGame) {
#ifdef __IPHONEOS__
	StoreKit_SetExternalData((void *)this);
#endif

	_eventsEnabled = false;
	_lastProductRequestOwner = NULL;
	_lastPurchaseOwner = NULL;
	_lastRestoreOwner = NULL;
}


//////////////////////////////////////////////////////////////////////////
CSXStore::~CSXStore() {
	cleanup();
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::cleanup() {
	SetEventsEnabled(NULL, false);

	for (int i = 0; i < _validProducts.GetSize(); i++) {
		delete _validProducts[i];
	}
	_validProducts.RemoveAll();


	for (int i = 0; i < _transactions.GetSize(); i++) {
		delete _transactions[i];
	}
	_transactions.RemoveAll();


	_lastProductRequestOwner = _lastPurchaseOwner = _lastRestoreOwner = NULL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CSXStore::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// EnableEvents
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "EnableEvents") == 0) {
		stack->correctParams(0);
		SetEventsEnabled(script, true);
		stack->pushBool(GetEventsEnabled() == true);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// DisableEvents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DisableEvents") == 0) {
		stack->correctParams(0);
		SetEventsEnabled(script, false);
		stack->pushBool(GetEventsEnabled() == false);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// ValidateProducts
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ValidateProducts") == 0) {
		stack->correctParams(1);
		const char *prodIdList = stack->pop()->getString();
		_lastProductRequestOwner = script->_owner;
		ValidateProducts(prodIdList);
		stack->pushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetValidProduct
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetValidProduct") == 0) {
		stack->correctParams(1);
		int index = stack->pop()->getInt();
		if (index >= 0 && index < _validProducts.GetSize()) {
			CScValue *prod = stack->getPushValue();
			if (prod) {
				prod->setProperty("Id", _validProducts[index]->GetId());
				prod->setProperty("Name", _validProducts[index]->GetName());
				prod->setProperty("Description", _validProducts[index]->GetDesc());
				prod->setProperty("Price", _validProducts[index]->GetPrice());
			}
		} else
			stack->pushNULL();

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetInvalidProduct
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetInvalidProduct") == 0) {
		stack->correctParams(1);
		int index = stack->pop()->getInt();
		if (index >= 0 && index < _invalidProducts.size())
			stack->pushString(_invalidProducts[index].c_str());
		else
			stack->pushNULL();

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetTransaction
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetTransaction") == 0) {
		stack->correctParams(1);
		int index = stack->pop()->getInt();
		if (index >= 0 && index < _transactions.GetSize()) {
			CScValue *trans = stack->getPushValue();
			if (trans) {
				trans->setProperty("Id", _transactions[index]->GetId());
				trans->setProperty("ProductId", _transactions[index]->GetProductId());
				trans->setProperty("State", _transactions[index]->GetState());
			}
		} else
			stack->pushNULL();

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Purchase
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Purchase") == 0) {
		stack->correctParams(1);
		const char *prodId = stack->pop()->getString();
		stack->pushBool(Purchase(script, prodId));

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// FinishTransaction
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FinishTransaction") == 0) {
		stack->correctParams(1);
		const char *transId = stack->pop()->getString();
		stack->pushBool(FinishTransaction(script, transId));

		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// RestoreTransactions
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RestoreTransactions") == 0) {
		stack->correctParams(0);
		RestoreTransactions(script);
		stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnlockProduct
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UnlockProduct") == 0) {
		stack->correctParams(1);
		const char *prodId = stack->pop()->getString();

		Game->_registry->WriteBool("Purchases", prodId, true);
		Game->_registry->SaveValues();

		stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsProductUnlocked
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsProductUnlocked") == 0) {
		stack->correctParams(1);
		const char *prodId = stack->pop()->getString();

		stack->pushBool(Game->_registry->ReadBool("Purchases", prodId, false));

		return S_OK;
	}

	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXStore::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("store");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Available (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Available") == 0) {
		_scValue->setBool(IsAvailable());
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// EventsEnabled (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EventsEnabled") == 0) {
		_scValue->setBool(GetEventsEnabled());
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumValidProducts (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumValidProducts") == 0) {
		_scValue->setInt(_validProducts.GetSize());
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumInvalidProducts (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumInvalidProducts") == 0) {
		_scValue->setInt(_invalidProducts.size());
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumTransactions (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumTransactions") == 0) {
		_scValue->setInt(_transactions.GetSize());
		return _scValue;
	}

	else return _scValue;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXStore::persist(CBPersistMgr *persistMgr) {
	if (!persistMgr->_saving) cleanup();

	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_eventsEnabled));
	persistMgr->transfer(TMEMBER(_lastProductRequestOwner));
	persistMgr->transfer(TMEMBER(_lastPurchaseOwner));
	persistMgr->transfer(TMEMBER(_lastRestoreOwner));
	persistMgr->transfer(TMEMBER(_invalidProducts));

	// persist valid products
	int numProducts;
	if (persistMgr->_saving) {
		numProducts = _validProducts.GetSize();
		persistMgr->transfer(TMEMBER(numProducts));
		for (int i = 0; i < numProducts; i++) _validProducts[i]->persist(persistMgr);
	} else {
		numProducts = _validProducts.GetSize();
		persistMgr->transfer(TMEMBER(numProducts));
		for (int i = 0; i < numProducts; i++) {
			CBStoreProduct *prod = new CBStoreProduct;
			prod->persist(persistMgr);
			_validProducts.Add(prod);
		}
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CSXStore::afterLoad() {
	if (_eventsEnabled) {
		SetEventsEnabled(NULL, true);
	}
#ifdef __IPHONEOS__
	StoreKit_SetExternalData((void *)this);
#endif
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::OnObjectDestroyed(CBScriptHolder *obj) {
	if (_lastProductRequestOwner == obj) _lastProductRequestOwner = NULL;
	if (_lastPurchaseOwner == obj) _lastPurchaseOwner = NULL;
	if (_lastRestoreOwner == obj) _lastRestoreOwner = NULL;
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::SetEventsEnabled(CScScript *script, bool val) {
	_eventsEnabled = val;

	if (val) {
		if (script) _lastPurchaseOwner = script->_owner;
#ifdef __IPHONEOS__
		StoreKit_EnableEvents();
#endif
	} else {
		_lastPurchaseOwner = NULL;
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
	for (int i = 0; i < _validProducts.GetSize(); i++) {
		delete _validProducts[i];
	}
	_validProducts.RemoveAll();

	_invalidProducts.clear();
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::ReceiveProductsEnd() {
	if (_lastProductRequestOwner) _lastProductRequestOwner->applyEvent("ProductsValidated");
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::AddValidProduct(const char *id, const char *name, const char *desc, const char *price) {
	CBStoreProduct *prod = new CBStoreProduct(id, name, desc, price);
	_validProducts.Add(prod);
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::AddInvalidProduct(const char *id) {
	_invalidProducts.push_back(id);
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::ReceiveTransactionsStart() {
	for (int i = 0; i < _transactions.GetSize(); i++) {
		delete _transactions[i];
	}
	_transactions.RemoveAll();
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::ReceiveTransactionsEnd() {
	if (_lastPurchaseOwner) _lastPurchaseOwner->applyEvent("TransactionsUpdated");
	else Game->applyEvent("TransactionsUpdated");
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::AddTransaction(const char *id, const char *productId, const char *state) {
	CBStoreTransaction *trans = new CBStoreTransaction(id, productId, state);
	_transactions.Add(trans);
}

//////////////////////////////////////////////////////////////////////////
bool CSXStore::Purchase(CScScript *script, const char *productId) {
	if (!productId) return false;

#ifdef __IPHONEOS__
	for (int i = 0; i < _validProducts.GetSize(); i++) {
		if (strcmp(productId, _validProducts[i]->GetId()) == 0) {
			_lastPurchaseOwner = script->_owner;

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
	for (int i = 0; i < _transactions.GetSize(); i++) {
		if (strcmp(transId, _transactions[i]->GetId()) == 0) {
			if (StoreKit_FinishTransaction(transId) > 0) {
				SAFE_DELETE(_transactions[i]);
				_transactions.RemoveAt(i);
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
	_lastRestoreOwner = script->_owner;
#ifdef __IPHONEOS__
	StoreKit_RestoreTransactions();
#endif
}

//////////////////////////////////////////////////////////////////////////
void CSXStore::OnRestoreFinished(bool error) {
	if (_lastRestoreOwner) {
		if (error) _lastRestoreOwner->applyEvent("TransactionsRestoreFailed");
		else _lastRestoreOwner->applyEvent("TransactionsRestoreFinished");
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
