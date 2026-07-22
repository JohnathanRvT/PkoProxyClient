#pragma once

#include <Rpc.h>
#include <string>
#include <cguid.h>

class Win32Guid {
public:
	Win32Guid();
	~Win32Guid();

	RPC_STATUS Generate();

	std::string AsString() const;

private:
	UUID uuid{IID_NULL};
};
