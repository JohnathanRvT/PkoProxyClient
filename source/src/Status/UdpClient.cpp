#include "udpclient.h"
#include "rng.h"

namespace client_udp {
CRegionMgr* GetRegionMgr() {
	static CRegionMgr Mgr;
	return &Mgr;
}
} // namespace client_udp

// 另外一种实现方法，不用从binary_function派生
//template <class T>
//class NameIsEqual
//{
//public:
//	NameIsEqual( const char* n ) { strcpy( szName, n ); }
//	bool operator() (const T p) const
//	{
//		return strcmp( p->szName, szName ) == 0;
//	}
//
//private:
//	char szName[32];
//
//};
//
//NameIsEqual<CRegionInfo*> var( szRegion );
//regions::iterator it = find_if( _Regions.begin(), _Regions.end(), bind2nd( NameIsEqual<CRegionInfo>(), var ) );

//template <class T, class LIST>
//T Find ( const char* szName, LIST list )
//{
//	for( LIST::iterator it=list.begin(); it!=list.end(); it++ )
//		if( stricmp( (*it)->szName, szName )==0 )
//			return *it;
//	return NULL;
//}

//---------------------------------------------------------------------------
// class CGateInfo
//---------------------------------------------------------------------------
client_udp::CGateInfo::CGateInfo(CGroupInfo* pGroup, const char* name)
	: pOwner(pGroup), nGateNum(-1) {
	//strncpy( szName, name, sizeof(szName) );
	::strncpy_s(szName, sizeof(szName), name, _TRUNCATE);
}

void client_udp::CGateInfo::Start() {
	if (Socket.IsCreate()) {
		return;
	}

	if (!Socket.Init(szName, 1976, 15000)) {
		return;
	}

	client_udp::GetRegionMgr()->GateCreateSocketCallBack(this);
}

void client_udp::CGateInfo::Exit() {
	if (!Socket.IsCreate())
		return;

	client_udp::GetRegionMgr()->GateCloseSocketCallBack(this);
	Socket.Close();
}

bool client_udp::CGateInfo::Read() {
	if (!Socket.IsCreate())
		return false;

	char szBuf[64] = {0};
	if (!Socket.Read(szBuf, sizeof(szBuf)))
		return false;

	nGateNum = atoi(szBuf);
	char* pStr = strchr(szBuf, ',');
	if (pStr) {
		pStr++;
		pOwner->nShowMin = atoi(pStr);
		pStr = strchr(pStr, ',');
		if (pStr) {
			pStr++;
			pOwner->nShowMax = atoi(pStr);
		}
	}
	return true;
}

bool client_udp::CGateInfo::Send() {
	if (!Socket.IsCreate())
		return false;

	char szBuf[] = {"#"};
	return Socket.Send(szBuf, (int)strlen(szBuf));
}

//---------------------------------------------------------------------------
// class CGroupInfo
//---------------------------------------------------------------------------
client_udp::CGroupInfo::CGroupInfo(CRegionInfo* pRegion, const char* name)
	: pOwner(pRegion), nShowMin(-1), nShowMax(-1) {
	//strncpy( szName, name, sizeof(szName) );
	::strncpy_s(szName, sizeof(szName), name, _TRUNCATE);
}

client_udp::CGateInfo* client_udp::CGroupInfo::Find(const char* Name) {
	auto it = std::find_if(Gates.begin(), Gates.end(), [Name](CGateInfo* info) {
		return strcmp(info->szName, Name) == 0;
	});
	return it != Gates.end() ? *it : nullptr;
}

void client_udp::CGroupInfo::Start() {
	for (auto & Gate : Gates)
		Gate->Start();
}

void client_udp::CGroupInfo::Exit() {
	for (auto & Gate : Gates)
		Gate->Exit();
}

void client_udp::CGroupInfo::Send() {
	for (auto & Gate : Gates)
		Gate->Send();
}

int client_udp::CGroupInfo::GetNum() {
	int nSum = 0;
	bool IsAdd = false;
	for (gates::iterator it = Gates.begin(); it != Gates.end(); it++) {
		if ((*it)->nGateNum >= 0) {
			nSum += (*it)->nGateNum;
			IsAdd = true;
		}
	}

	if (!IsAdd)
		return 0;
	if (nSum <= nShowMin)
		return 1;
	if (nSum <= nShowMax)
		return 2;
	return 3;
}

client_udp::CGateInfo* client_udp::CGroupInfo::GetMinGate() {
	
	if (Gates.empty())
		return nullptr;

	CGateInfo* pGate = nullptr;
	int nCount = INT_MAX;
	for (gates::iterator it = Gates.begin(); it != Gates.end(); it++) {
		if ((*it)->nGateNum < 0)
			return nullptr;

		if (nCount > (*it)->nGateNum) {
			pGate = (*it);
			nCount = pGate->nGateNum;
		}
	}

	if (!pGate) {
		int nGateNo = rng.uniform(0, (int)Gates.size() - 1);
		return Gates[nGateNo];
	}
	return pGate;
}

//---------------------------------------------------------------------------
// class CRegionInfo
//---------------------------------------------------------------------------
client_udp::CRegionInfo::CRegionInfo(CRegionMgr* Mgr, const char* name)
	: pOwner(Mgr) {
	//strncpy( szName, name, sizeof(szName) );
	::strncpy_s(szName, sizeof(szName), name, _TRUNCATE);
}

client_udp::CGroupInfo* client_udp::CRegionInfo::Find(const char* Name) {
	auto it = std::find_if(Groups.begin(), Groups.end(), [Name](CGroupInfo* info) {
		return strcmp(info->szName, Name) == 0;
	});
	return it != Groups.end() ? *it : nullptr;
}

void client_udp::CRegionInfo::Start() {
	for (auto & Group : Groups)
		Group->Start();
}

void client_udp::CRegionInfo::Exit() {
	for (auto & Group : Groups)
		Group->Exit();
}

void client_udp::CRegionInfo::Send() {
	for (auto & Group : Groups)
		Group->Send();
}

//---------------------------------------------------------------------------
// class CRegionMgr
//---------------------------------------------------------------------------
client_udp::CRegionMgr::CRegionMgr()
	: _pRegion(nullptr), _hWnd(nullptr), evtGateChanteEvent(nullptr), _dwLastSendTime(0) {
	_uNetMessage = WM_USER + 100;
}

void client_udp::CRegionMgr::Init(HWND hWnd, UINT uRegeditNetMes) {
	_hWnd = hWnd;
	_uNetMessage = uRegeditNetMes;
}

client_udp::CRegionInfo* client_udp::CRegionMgr::Find(const char* Name) {
	auto it = std::find_if(_Regions.begin(), _Regions.end(), [Name](CRegionInfo* info) {
		return strcmp(info->szName, Name) == 0;
	});
	return it != _Regions.end() ? *it : nullptr;
}

bool client_udp::CRegionMgr::Add(const char* szRegion, const char* szGroup, const char* szGate) {
	CRegionInfo* pRegion = Find(szRegion);
	if (!pRegion) {
		pRegion = new CRegionInfo(this, szRegion);
		Add(pRegion);
	}

	CGroupInfo* pGroup = pRegion->Find(szGroup);
	if (!pGroup) {
		pGroup = new CGroupInfo(pRegion, szGroup);
		pRegion->Add(pGroup);
	}

	CGateInfo* pGate = pGroup->Find(szGate);
	if (!pGate) {
		pGate = new CGateInfo(pGroup, szGate);
		pGroup->Add(pGate);
		return true;
	}
	return false;
}

bool client_udp::CRegionMgr::EnterRegion(const char* szRegion) {
	//if( _pRegion && stricmp( _pRegion->szName, szRegion )==0  )
	if (_pRegion && _stricmp(_pRegion->szName, szRegion) == 0) {
		return true;
	}

	ExitRegion();

	CRegionInfo* pRegion = Find(szRegion);
	if (pRegion) {
		_dwLastSendTime = 0;
		_pRegion = pRegion;
		_pRegion->Start();
		return true;
	}
	return false;
}

int client_udp::CRegionMgr::GetGroupNum(const char* szGroup) {
	if (!_pRegion)
		return -1;

	CGroupInfo* pGroup = _pRegion->Find(szGroup);
	if (!pGroup)
		return -1;

	return pGroup->GetNum();
}

bool client_udp::CRegionMgr::ExitRegion() {
	if (!_pRegion)
		return false;

	_pRegion = nullptr;
	return true;
}

void client_udp::CRegionMgr::Exit() {
	_pRegion = nullptr;
	_gates.clear();
	for (auto & Region : _Regions)
		Region->Exit();
}

bool client_udp::CRegionMgr::OnMessage(UINT Message, WPARAM wParam, LPARAM lParam) {
	if (Message != _uNetMessage)
		return false;

	switch (WSAGETSELECTEVENT(lParam)) {
	case FD_READ: {
		auto it = _gates.find((SOCKET)wParam);
		if (it == _gates.end())
			break;

		CGateInfo* pGate = it->second;
		if (pGate->Read() && evtGateChanteEvent) {
			evtGateChanteEvent(pGate);
		}
	} break;
	}
	return true;
}

void client_udp::CRegionMgr::FrameMove(DWORD dwTime) {
	if (!_pRegion)
		return;

	if (dwTime > _dwLastSendTime) {
		_dwLastSendTime = dwTime + 10000;
		_pRegion->Send();
	}
}

void client_udp::CRegionMgr::GateCreateSocketCallBack(CGateInfo* pGate) {
	pGate->GetUdp().RegeditReadEvent(_hWnd, _uNetMessage);
	_gates.insert(gates::value_type(pGate->GetUdp().GetSocket(), pGate));
}

void client_udp::CRegionMgr::GateCloseSocketCallBack(CGateInfo* pGate) {
	if (auto it = _gates.find((SOCKET)pGate->GetUdp().GetSocket());
		it != _gates.end())
		_gates.erase(it);
}
