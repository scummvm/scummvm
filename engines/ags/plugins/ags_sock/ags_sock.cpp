/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "common/array.h"
#include "common/str.h"
#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"
#include "ags/plugins/ags_sock/ags_sock.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSock {

struct SockData : public IAGSScriptManagedObject, public Common::Array<byte> {
public:
	int Dispose(void *address, bool force) override {
		delete this;
		return true;
	}
	const char *GetType() override {
		return "SockData";
	};
	int Serialize(void *address, char *buffer, int bufsize) override {
		return 0;
	}
};

struct SockAddr : public IAGSScriptManagedObject {
public:
	int _port = 0;
	Common::String _address;
	Common::String _ip;

	int Dispose(void *address, bool force) override {
		delete this;
		return true;
	}
	const char *GetType() override {
		return "SockAddr";
	};
	int Serialize(void *address, char *buffer, int bufsize) override {
		return 0;
	}
};

struct Socket : public IAGSScriptManagedObject {
public:
	int _id = 0;
	int _domain = 0;
	int _type = 0;
	int _protocol = 0;
	int _lastError = 0;
	Common::String _tag;
	SockAddr *_local = nullptr;
	SockAddr *_remote = nullptr;
	bool _valid = false;
	Common::String _errorString;

	int Dispose(void *address, bool force) override {
		delete this;
		return true;
	}
	const char *GetType() override {
		return "Socket";
	};
	int Serialize(void *address, char *buffer, int bufsize) override {
		return 0;
	}
};


const char *AGSSock::AGS_GetPluginName() {
	return "AGS Sock";
}

void AGSSock::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(SockData::Create^2, AGSSock::SockData_Create);
	SCRIPT_METHOD(SockData::CreateEmpty^0, AGSSock::SockData_CreateEmpty);
	SCRIPT_METHOD(SockData::CreateFromString^1, AGSSock::SockData_CreateFromString);
	SCRIPT_METHOD(SockData::get_Size, AGSSock::SockData_get_Size);
	SCRIPT_METHOD(SockData::set_Size, AGSSock::SockData_set_Size);
	SCRIPT_METHOD(SockData::geti_Chars, AGSSock::SockData_geti_Chars);
	SCRIPT_METHOD(SockData::seti_Chars, AGSSock::SockData_seti_Chars);
	SCRIPT_METHOD(SockData::AsString^0, AGSSock::SockData_AsString);
	SCRIPT_METHOD(SockData::Clear^0, AGSSock::SockData_Clear);
	SCRIPT_METHOD(SockAddr::Create^1, AGSSock::SockAddr_Create);
	SCRIPT_METHOD(SockAddr::CreateFromString^2, AGSSock::SockAddr_CreateFromString);
	SCRIPT_METHOD(SockAddr::CreateFromData^1, AGSSock::SockAddr_CreateFromData);
	SCRIPT_METHOD(SockAddr::CreateIP^2, AGSSock::SockAddr_CreateIP);
	SCRIPT_METHOD(SockAddr::CreateIPv6^2, AGSSock::SockAddr_CreateIPv6);
	SCRIPT_METHOD(SockAddr::get_Port, AGSSock::SockAddr_get_Port);
	SCRIPT_METHOD(SockAddr::set_Port, AGSSock::SockAddr_set_Port);
	SCRIPT_METHOD(SockAddr::get_Address, AGSSock::SockAddr_get_Address);
	SCRIPT_METHOD(SockAddr::set_Address, AGSSock::SockAddr_set_Address);
	SCRIPT_METHOD(SockAddr::get_IP, AGSSock::SockAddr_get_IP);
	SCRIPT_METHOD(SockAddr::set_IP, AGSSock::SockAddr_set_IP);
	SCRIPT_METHOD(SockAddr::GetData^0, AGSSock::SockAddr_GetData);
	SCRIPT_METHOD(Socket::Create^3, AGSSock::Socket_Create);
	SCRIPT_METHOD(Socket::CreateUDP^0, AGSSock::Socket_CreateUDP);
	SCRIPT_METHOD(Socket::CreateTCP^0, AGSSock::Socket_CreateTCP);
	SCRIPT_METHOD(Socket::CreateUDPv6^0, AGSSock::Socket_CreateUDPv6);
	SCRIPT_METHOD(Socket::CreateTCPv6^0, AGSSock::Socket_CreateTCPv6);
	SCRIPT_METHOD(Socket::get_Tag, AGSSock::Socket_get_Tag);
	SCRIPT_METHOD(Socket::set_Tag, AGSSock::Socket_set_Tag);
	SCRIPT_METHOD(Socket::get_Local, AGSSock::Socket_get_Local);
	SCRIPT_METHOD(Socket::get_Remote, AGSSock::Socket_get_Remote);
	SCRIPT_METHOD(Socket::get_Valid, AGSSock::Socket_get_Valid);
	SCRIPT_METHOD(Socket::ErrorString^0, AGSSock::Socket_ErrorString);
	SCRIPT_METHOD(Socket::Bind^1, AGSSock::Socket_Bind);
	SCRIPT_METHOD(Socket::Listen^1, AGSSock::Socket_Listen);
	SCRIPT_METHOD(Socket::Connect^2, AGSSock::Socket_Connect);
	SCRIPT_METHOD(Socket::Accept^0, AGSSock::Socket_Accept);
	SCRIPT_METHOD(Socket::Close^0, AGSSock::Socket_Close);
	SCRIPT_METHOD(Socket::Send^1, AGSSock::Socket_Send);
	SCRIPT_METHOD(Socket::SendTo^2, AGSSock::Socket_SendTo);
	SCRIPT_METHOD(Socket::Recv^0, AGSSock::Socket_Recv);
	SCRIPT_METHOD(Socket::RecvFrom^1, AGSSock::Socket_RecvFrom);
	SCRIPT_METHOD(Socket::SendData^1, AGSSock::Socket_SendData);
	SCRIPT_METHOD(Socket::SendDataTo^2, AGSSock::Socket_SendDataTo);
	SCRIPT_METHOD(Socket::RecvData^0, AGSSock::Socket_RecvData);
	SCRIPT_METHOD(Socket::RecvDataFrom^1, AGSSock::Socket_RecvDataFrom);
	SCRIPT_METHOD(Socket::GetOption^2, AGSSock::Socket_GetOption);
	SCRIPT_METHOD(Socket::SetOption^3, AGSSock::Socket_SetOption);
}

void AGSSock::SockData_Create(ScriptMethodParams &params) {
	PARAMS2(int, size, char, defchar);

	SockData *data = new SockData();
	_engine->RegisterManagedObject(data, data);
	data->resize(size);
	Common::fill(&(*data)[0], &(*data)[0] + size, defchar);

	params._result = data;
}

void AGSSock::SockData_CreateEmpty(ScriptMethodParams &params) {
	params._result = new SockData();
}

void AGSSock::SockData_CreateFromString(ScriptMethodParams &params) {
	PARAMS1(const char *, str);
	size_t len = strlen(str);

	SockData *data = new SockData();
	_engine->RegisterManagedObject(data, data);
	data->resize(len + 1);
	Common::copy(str, str + len + 1, &(*data)[0]);

	params._result = data;
}

void AGSSock::SockData_get_Size(ScriptMethodParams &params) {
	PARAMS1(SockData *, sockData);
	params._result = sockData->size();
}

void AGSSock::SockData_set_Size(ScriptMethodParams &params) {
	PARAMS2(SockData *, sockData, size_t, size);
	sockData->resize(size);
}

void AGSSock::SockData_geti_Chars(ScriptMethodParams &params) {
	PARAMS1(SockData *, sockData);
	params._result = &(*sockData)[0];
}

void AGSSock::SockData_seti_Chars(ScriptMethodParams &params) {
	PARAMS2(SockData *, sockData, const byte *, chars);
	Common::copy(chars, chars + sockData->size(), &(*sockData)[0]);
}

void AGSSock::SockData_AsString(ScriptMethodParams &params) {
	PARAMS1(SockData *, sockData);
	params._result = (const char *)&(*sockData)[0];
}

void AGSSock::SockData_Clear(ScriptMethodParams &params) {
	PARAMS1(SockData *, sockData);
	sockData->clear();
}

void AGSSock::SockAddr_Create(ScriptMethodParams &params) {
//	PARAMS1(int, type);
	SockAddr *sockAddr = new SockAddr();
	_engine->RegisterManagedObject(sockAddr, sockAddr);

	params._result = sockAddr;
}

void AGSSock::SockAddr_CreateFromString(ScriptMethodParams &params) {
//	PARAMS2(const char *, address, int, type);
	PARAMS1(const char *, address);

	SockAddr *sockAddr = new SockAddr();
	_engine->RegisterManagedObject(sockAddr, sockAddr);

	sockAddr->_address = address;
	params._result = sockAddr;
}

void AGSSock::SockAddr_CreateFromData(ScriptMethodParams &params) {
//	PARAMS1(const SockData *, data);
	SockAddr *sockAddr = new SockAddr();
	_engine->RegisterManagedObject(sockAddr, sockAddr);

	params._result = sockAddr;
}

void AGSSock::SockAddr_CreateIP(ScriptMethodParams &params) {
	PARAMS2(const char *, address, int, port);

	SockAddr *sockAddr = new SockAddr();
	_engine->RegisterManagedObject(sockAddr, sockAddr);

	sockAddr->_address = address;
	sockAddr->_port = port;

	params._result = sockAddr;
}

void AGSSock::SockAddr_CreateIPv6(ScriptMethodParams &params) {
	//PARAMS2(const char *, address, int, port);
	PARAMS1(const char *, address);

	SockAddr *sockAddr = new SockAddr();
	_engine->RegisterManagedObject(sockAddr, sockAddr);

	sockAddr->_address = address;
	params._result = sockAddr;
}

void AGSSock::SockAddr_get_Port(ScriptMethodParams &params) {
	PARAMS1(const SockAddr *, sockAddr);
	params._result = sockAddr->_port;
}

void AGSSock::SockAddr_set_Port(ScriptMethodParams &params) {
	PARAMS2(SockAddr *, sockAddr, int, port);
	sockAddr->_port = port;
}

void AGSSock::SockAddr_get_Address(ScriptMethodParams &params) {
	PARAMS1(const SockAddr *, sockAddr);
	params._result = sockAddr->_address.c_str();
}

void AGSSock::SockAddr_set_Address(ScriptMethodParams &params) {
	PARAMS2(SockAddr *, sockAddr, const char *, address);
	sockAddr->_address = address;
}

void AGSSock::SockAddr_get_IP(ScriptMethodParams &params) {
	PARAMS1(const SockAddr *, sockAddr);
	params._result = sockAddr->_ip.c_str();
}

void AGSSock::SockAddr_set_IP(ScriptMethodParams &params) {
	PARAMS2(SockAddr *, sockAddr, const char *, IP);
	sockAddr->_ip = IP;
}

void AGSSock::SockAddr_GetData(ScriptMethodParams &params) {
//	PARAMS1(const SockAddr *, sockAddr);
	params._result = new SockData();
}


void AGSSock::Socket_Create(ScriptMethodParams &params) {
	//PARAMS3(int, domain, int, type, int, protocol);

	Socket *socket = new Socket();
	_engine->RegisterManagedObject(socket, socket);

	params._result = socket;
}

void AGSSock::Socket_CreateUDP(ScriptMethodParams &params) {
	Socket *socket = new Socket();
	_engine->RegisterManagedObject(socket, socket);

	params._result = socket;
}

void AGSSock::Socket_CreateTCP(ScriptMethodParams &params) {
	Socket *socket = new Socket();
	_engine->RegisterManagedObject(socket, socket);

	params._result = socket;
}

void AGSSock::Socket_CreateUDPv6(ScriptMethodParams &params) {
	Socket *socket = new Socket();
	_engine->RegisterManagedObject(socket, socket);

	params._result = socket;
}

void AGSSock::Socket_CreateTCPv6(ScriptMethodParams &params) {
	Socket *socket = new Socket();
	_engine->RegisterManagedObject(socket, socket);

	params._result = socket;
}

void AGSSock::Socket_get_Tag(ScriptMethodParams &params) {
	PARAMS1(const Socket *, socket);
	params._result = socket->_tag.c_str();
}

void AGSSock::Socket_set_Tag(ScriptMethodParams &params) {
	PARAMS2(Socket *, socket, const char *, tag);
	socket->_tag = tag;
}

void AGSSock::Socket_get_Local(ScriptMethodParams &params) {
	PARAMS1(const Socket *, socket);
	params._result = socket->_local;
}

void AGSSock::Socket_get_Remote(ScriptMethodParams &params) {
	PARAMS1(const Socket *, socket);
	params._result = socket->_remote;
}

void AGSSock::Socket_get_Valid(ScriptMethodParams &params) {
	PARAMS1(const Socket *, socket);
	params._result = socket->_valid;
}

void AGSSock::Socket_ErrorString(ScriptMethodParams &params) {
	PARAMS1(const Socket *, socket);
	params._result = socket->_errorString.c_str();
}

void AGSSock::Socket_Bind(ScriptMethodParams &params) {
}

void AGSSock::Socket_Listen(ScriptMethodParams &params) {
}

void AGSSock::Socket_Connect(ScriptMethodParams &params) {
	// Fail the connection
	params._result = 0;
}

void AGSSock::Socket_Accept(ScriptMethodParams &params) {
}

void AGSSock::Socket_Close(ScriptMethodParams &params) {
}

void AGSSock::Socket_Send(ScriptMethodParams &params) {
}

void AGSSock::Socket_SendTo(ScriptMethodParams &params) {
}

void AGSSock::Socket_Recv(ScriptMethodParams &params) {
}

void AGSSock::Socket_RecvFrom(ScriptMethodParams &params) {
}

void AGSSock::Socket_SendData(ScriptMethodParams &params) {
}

void AGSSock::Socket_SendDataTo(ScriptMethodParams &params) {
}

void AGSSock::Socket_RecvData(ScriptMethodParams &params) {
}

void AGSSock::Socket_RecvDataFrom(ScriptMethodParams &params) {
}

void AGSSock::Socket_GetOption(ScriptMethodParams &params) {
}

void AGSSock::Socket_SetOption(ScriptMethodParams &params) {
}

} // namespace AGSSock
} // namespace Plugins
} // namespace AGS3
