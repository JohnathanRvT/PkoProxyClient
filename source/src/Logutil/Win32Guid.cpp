#include "Win32Guid.h"

Win32Guid::Win32Guid() {
}

Win32Guid::~Win32Guid() {
}

RPC_STATUS Win32Guid::Generate() {
	return UuidCreate(&this->uuid);
}

std::string Win32Guid::AsString() const {
	const char* uuidStr;
	if (UuidToString((UUID*)&this->uuid, (RPC_CSTR*)&uuidStr) != RPC_S_OK) {
		return "";
	}
	std::string result = uuidStr;
	RpcStringFree((RPC_CSTR*)&uuidStr);
	return result;
}
