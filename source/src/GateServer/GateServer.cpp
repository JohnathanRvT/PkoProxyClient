#include <iostream>
#include <signal.h>
#include "timer.h"
#include "gateserver.h"

//#pragma comment( lib, "../../../status/lib/Status.lib" )

size_t NetBuffer[] = {100, 10, 0};
bool g_logautobak = true;
LogStream g_gateerr("ErrServer");
LogStream g_gatelog("GateServer");
LogStream g_chkattack("AttackMonitor");
LogStream g_gateconnect("Connect");
//LogStream g_gatepacket("PacketProc");

// Add by lark.li 20081119 begin
LogStream g_Loginlog("TestLogin");
// End

std::atomic<long> g_exit = 0;
std::atomic<long> g_ref = 0;

TimerMgr g_timermgr;
//=========Timer==============
extern "C" {
WINBASEAPI HWND APIENTRY GetConsoleWindow(VOID);
}
class DisableCloseButton : public Timer {
public:
	DisableCloseButton(uLong interval) : Timer(interval), m_hMenu(nullptr) {
		HWND hWnd = ::GetConsoleWindow();
		m_hMenu = GetSystemMenu(hWnd, FALSE);
	}

private:
	~DisableCloseButton() {
	}
	void Process() override {
		RefArmor l(g_ref);
		if (!g_exit && m_hMenu) {
			EnableMenuItem(m_hMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
		}
	}
	HMENU m_hMenu;
};
class DelayLogout : public Timer, public RunBiDirectChain<Player> {
public:
	DelayLogout(uLong interval) : Timer(interval) {}
	void AddPlayer(Player* ply) {
		ply->_BeginRun(this);
	}
	void DelPlayer(Player* ply) {
		ply->_EndRun();
	}

private:
	void Process() override {
		Player* player = nullptr;
		RunChainGetArmor<Player> l_lock(*this);
		while (player = GetNextItem()) {
		}
		l_lock.unlock();
	}
};

void __cdecl ctrlc_dispatch(int sig) {
	if (sig == SIGINT) {
		g_exit = 1;
		signal(SIGINT, ctrlc_dispatch);
	}
}

//---------------------------------------------------------------------------
// class GateServer
//---------------------------------------------------------------------------
GateServer::GateServer(char const* fname)
	: player_heap(1, 2000), m_tch(1, 100),
	  m_clcomm(nullptr), m_gpcomm(nullptr), m_gmcomm(nullptr), m_clproc(nullptr) {
	TcpCommApp::WSAStartup();
	srand((unsigned int)time(nullptr)); // 初始化随机数种子

	m_tch.Init();
	player_heap.Init();

	m_clproc = ThreadPool::CreatePool(24, 32, 4096);
	m_clcomm = ThreadPool::CreatePool(6, 12, 4096, THREAD_PRIORITY_ABOVE_NORMAL);
	m_gpproc = ThreadPool::CreatePool(4, 8, 1024, THREAD_PRIORITY_ABOVE_NORMAL);
	m_gpcomm = ThreadPool::CreatePool(12, 24, 2048, THREAD_PRIORITY_ABOVE_NORMAL);
	m_gmcomm = ThreadPool::CreatePool(4, 4, 2048, THREAD_PRIORITY_ABOVE_NORMAL);

	//m_mmsproc = ThreadPool::CreatePool(4, 8, 1024, THREAD_PRIORITY_ABOVE_NORMAL);
	//m_mmscomm = ThreadPool::CreatePool(12, 24, 2048, THREAD_PRIORITY_ABOVE_NORMAL);

	try {
		toGameServer = std::make_unique<ToGameServer>(fname, nullptr, m_gmcomm);
		toGroupServer = std::make_unique<ToGroupServer>(fname, m_gpproc, m_gpcomm);
		toClient = std::make_unique<ToClient>(fname, m_clproc, m_clcomm);

		//m_ToMMS = new ToMMS(fname, m_mmsproc, m_mmscomm);
		//m_mmsproc->AddTask(new ConnectMMS(m_ToMMS));

		m_gpproc->AddTask(new ConnectGroupServer(toGroupServer.get()));
		m_clproc->AddTask(&g_timermgr);
		g_timermgr.AddTimer(new DisableCloseButton(200));
		signal(SIGINT, ctrlc_dispatch);
	} catch (...) {
		//if(m_ToMMS)
		//{
		//	delete m_ToMMS;
		//	m_ToMMS = NULL;
		//}

		m_gmcomm->DestroyPool();
		m_gpcomm->DestroyPool();
		m_clcomm->DestroyPool();
		m_clproc->DestroyPool();

		//m_mmsproc->DestroyPool();
		//m_mmscomm->DestroyPool();

		TcpCommApp::WSACleanup();
		throw;
	}
}

GateServer::~GateServer() {
	g_exit = 1;
	while (g_ref) {
		Sleep(1);
	}

	//delete m_ToMMS;
	//m_mmsproc->DestroyPool();
	//m_mmscomm->DestroyPool();

	m_gmcomm->DestroyPool();
	m_gpcomm->DestroyPool();
	m_clcomm->DestroyPool();
	m_clproc->DestroyPool();
	TcpCommApp::WSACleanup();
}

void GateServer::RunLoop() {
	BandwidthStat l_band;
	LLong recvpkps_max = 0, recvbandps_max = 0, sendpkps_max = 0, sendbandps_max = 0;

	std::string input;
	while (!g_exit) {
		std::cout << RES_STRING(GS_GATESERVER_CPP_00001);
		std::getline(std::cin, input);

		if (input == "exit" || g_exit) {
			std::cout << RES_STRING(GS_GATESERVER_CPP_00002) << std::endl;
			break;
		} else if (input == "help" || input == "?") {
			for (const auto& command : {"exit", "getinfo", "clmax", "getmaxcon ", "setmaxcon ", "reconnect",
										"calltotal", "getcheck", "setcheckspan", "setcheckwaring", "setcheckerror"}) {
				std::cout << command << '\n';
			}
			std::cout << std::endl;
		} else if (input == "getinfo") {
			std::cout << "getinfo..." << std::endl;

			l_band = toClient->GetBandwidthStat();
			std::cout << "getinfo: GetBandwidthStat..." << std::endl;
			std::cout << RES_STRING(GS_GATESERVER_CPP_00003) << toClient->GetSockTotal() << std::endl;
			std::cout << RES_STRING(GS_GATESERVER_CPP_00004) << l_band.m_sendpktps << "}{pkt:" << l_band.m_sendpkts << "}{KB/s:" << l_band.m_sendbyteps / 1024 << "}{KB:" << l_band.m_sendbytes / 1024 << "}" << std::endl;
			std::cout << RES_STRING(GS_GATESERVER_CPP_00005) << l_band.m_recvpktps << "}{pkt:" << l_band.m_recvpkts << "}{KB/s:" << l_band.m_recvbyteps / 1024 << "}{KB:" << l_band.m_recvbytes / 1024 << "}" << std::endl;

			if (l_band.m_sendpktps > sendpkps_max) {
				sendpkps_max = l_band.m_sendpktps;
			}
			if (l_band.m_sendbyteps / 1024 > sendbandps_max) {
				sendbandps_max = l_band.m_sendbyteps / 1024;
			}
			if (l_band.m_recvpktps > recvpkps_max) {
				recvpkps_max = l_band.m_recvpktps;
			}
			if (l_band.m_recvbyteps / 1024 > recvbandps_max) {
				recvbandps_max = l_band.m_recvbyteps / 1024;
			}
			std::cout << RES_STRING(GS_GATESERVER_CPP_00006) << sendpkps_max << "}{KB/s:" << sendbandps_max << "}" << std::endl;
			std::cout << RES_STRING(GS_GATESERVER_CPP_00007) << recvpkps_max << "}{KB/s:" << recvbandps_max << "}" << std::endl;
		} else if (input == "clmax") {
			recvpkps_max = recvbandps_max = sendpkps_max = sendbandps_max = 0;
#ifdef NET_CAL
		} else if (input == "getcheck") {
			std::cout << "Span = " << g_GateServer->toClient->GetCheckSpan() << "Waring = " << g_GateServer->toClient->GetCheckWaring() << "Error = " << g_GateServer->toClient->GetCheckError() << std::endl;
		} else if (!strncmp(input.c_str(), "setcheckspan", 12)) {
			uShort l_span = atoi(input.c_str() + 12);
			g_GateServer->toClient->SetCheckSpan(l_span);
		} else if (!strncmp(input.c_str(), "setcheckwaring", 14)) {
			uShort l_span = atoi(input.c_str() + 14);
			g_GateServer->toClient->SetCheckWaring(l_span);
		} else if (!strncmp(input.c_str(), "setcheckerror", 13)) {
			uShort l_span = atoi(input.c_str() + 13);
			g_GateServer->toClient->SetCheckError(l_span);
#endif
		} else if (input == "getmaxcon") {
			//std::cout<<"当前允许最大连接值："<<g_gtsvr->cli_conn->GetMaxCon()<<std::endl;
			std::cout << RES_STRING(GS_GATESERVER_CPP_00008) << g_GateServer->toClient->GetMaxCon() << std::endl;
		} else if (!strncmp(input.c_str(), "setmaxcon", 9)) {
			uShort l_maxcon = atoi(input.c_str() + 9);
			if (l_maxcon > 1500) {
				//std::cout<<"最大连接数不能超过1500,当前的最大连接已设置成最大值1500"<<std::endl;
				std::cout << RES_STRING(GS_GATESERVER_CPP_00009) << std::endl;
				l_maxcon = 1500;
			} else {
				//std::cout<<"设置成功，最大连接数:"<<l_maxcon<<std::endl;
				std::cout << RES_STRING(GS_GATESERVER_CPP_00010) << l_maxcon << std::endl;
			}
			g_GateServer->toClient->SetMaxCon(l_maxcon);
		} /*else	if(input	=="logbak")
		{
			LogStream::Backup();
		}*/
		else if (input == "getqueparm") {
			std::cout << "ToClient Process Queue:" << m_clproc->GetTaskCount() << "\tToClint Comm Queue:" << m_clcomm->GetTaskCount() << std::endl;
			std::cout << "ToGroup Comm Queue:" << m_gpcomm->GetTaskCount() << "\tToGame Comm Queue:" << m_gmcomm->GetTaskCount() << std::endl;
			//}else	if(!strncmp(input.c_str(),"setshowrange",12))
			//{
			//	const char* pstring = input.c_str();
			//	pstring += 12;
			//	int min = atoi(pstring);
			//	pstring = strchr( pstring, ',' );
			//	if( !pstring )
			//	{
			//		//std::cout<<"setshowrange 参数1,参数2" <<std::endl;
			//		std::cout<<RES_STRING(GS_GATESERVER_CPP_00011) <<std::endl;
			//	}
			//	else
			//	{
			//		pstring++;
			//		int max = atoi( pstring );
			//		std::cout<<"SetShowRnage:["<< min << "-" << max << "]" <<std::endl;
			//		g_app->SetShowRange( min, max );
			//	}
			//}
			//else	if(input	=="getshowrange")
			//{
			//	std::cout<<"ShowRnage:["<< g_app->GetShowMin() << "-" << g_app->GetShowMax() << "]" <<std::endl;
		} else if (input == "reconnect") {
			if (g_GateServer->toGroupServer) {
				g_GateServer->toGroupServer->Disconnect(-9);
				std::cout << "reconnect success!" << std::endl;
			} else {
				std::cout << "reconnect failed! null pointer!" << std::endl;
			}
		} else if (input == "calltotal") {
			std::cout << "clinet::calltotal:[" << g_GateServer->toClient->GetCallTotal() << "]" << std::endl;
			std::cout << "group::calltotal:[" << g_GateServer->toGroupServer->GetCallTotal() << "]" << std::endl;
		} else {
			//std::cout<<"不支持的命令！"<<std::endl;
			std::cout << RES_STRING(GS_GATESERVER_CPP_00012) << std::endl;
		}
	}
}

//---------------------------------------------------------------------------
// class Player
//---------------------------------------------------------------------------
bool Player::InitReference(DataSocket* datasock) {
	std::lock_guard<std::mutex> lock(g_GateServer->_mtxother); //组织重复进入
	if (datasock && !datasock->GetPointer()) {
		datasock->SetPointer(this);
		m_datasock = datasock;
		return true;
	} else {
		if (datasock) {
			try {
				//printf( "InitReference warning: %s重复进入连接信息！", datasock->GetPeerIP() );
				printf(RES_STRING(GS_GATESERVER_CPP_00013), datasock->GetPeerIP());
				auto player = static_cast<Player*>(datasock->GetPointer());
				if (player) {
					player->m_datasock = nullptr;
					datasock->SetPointer(nullptr);
				}
			} catch (...) {
				//printf( "InitReference warning: %s重复进入连接信息！exception", datasock->GetPeerIP() );
				printf(RES_STRING(GS_GATESERVER_CPP_00014), datasock->GetPeerIP());
			}
		}
		return false;
	}
	return false;
}

void Player::Initially() {
	m_worldid = m_actid = m_dbid = m_status = m_switch = 0;
	gm_addr = gp_addr = 0;
	m_chapstr[0] = 0;
	m_password[0] = 0;
	m_datasock = nullptr;
	game = nullptr;
	memset(comm_textkey, 0, sizeof comm_textkey);

	enc = false;
	m_pingtime = 0;
	m_lestoptick = GetTickCount();
	m_estop = false;
	m_sGarnerWiner = 0;
}

void Player::Finally() {
	m_worldid = m_actid = m_dbid = m_status = 0;
	gm_addr = gp_addr = 0;
	m_chapstr[0] = 0;
	game = nullptr;
	memset(comm_textkey, 0, sizeof comm_textkey);
	enc = false;
	if (m_datasock != nullptr) {
		m_datasock->SetPointer(nullptr);
		m_datasock = nullptr;
	}
}

// Add by lark.li 20081119 begin
bool Player::BeginRun() {
	return RunBiDirectItem<Player>::_BeginRun(&(g_GateServer->m_playerlist)) ? true : false;
}
bool Player::EndRun() {
	return RunBiDirectItem<Player>::_EndRun() ? true : false;
}
// End

//---------------------------------------------------------------------------
// class GateServerApp
//---------------------------------------------------------------------------
//int	GateServerApp::_nShowMin = 0;
//int	GateServerApp::_nShowMax = 0;
GateServerApp* g_app = nullptr;

GateServerApp::GateServerApp()
//: _pUdpManage(NULL)
{
	g_app = this;
}
void GateServerApp::ServiceStart() {
	// 启动服务器
	try {
		constexpr auto file_cfg = "GateServer.cfg";
		g_GateServer = std::make_unique<GateServer>(file_cfg);

		//IniFile inf(file_cfg);
		//_nShowMin = atoi(inf["ShowRange"]["ShowMin"]);
		//_nShowMax = atoi(inf["ShowRange"]["ShowMax"]);
		//if( atoi(inf["ShowRange"]["IsUse"])!=0 )
		//{
		//	_pUdpManage = new CUdpManage;
		//	if( !_pUdpManage->Init( 1976, _NotifySocketNumEvent ) )
		//		//cout << "监听数量功能创建失败" << endl;
		//		cout << RES_STRING(GS_GATESERVER_CPP_00015) << endl;
		//}
	} catch (excp& e) {
		std::cout << e.what() << std::endl;
		Sleep(10 * 1000);
		exit(-1);
	} catch (...) {
		//cout << "GateServer 初始化期间发生未知错误，请通知开发者!" << endl;
		std::cout << RES_STRING(GS_GATESERVER_CPP_00016) << std::endl;
		Sleep(10 * 1000);
		exit(-2);
	}

	std::cout << RES_STRING(GS_GATESERVER_CPP_00017) << std::endl;
}
void GateServerApp::ServiceStop() {
	//if( _pUdpManage )
	//{
	//	delete _pUdpManage;
	//	_pUdpManage = NULL;
	//}

	// 服务器退出
	g_app = nullptr;

	//cout << "GateServer 成功退出!" << endl;
	std::cout << RES_STRING(GS_GATESERVER_CPP_00018) << std::endl;
	Sleep(2000);
}

//void GateServerApp::_NotifySocketNumEvent( CUdpManage* pManage, CUdpServer* pUdpServer, const char* szClientIP, unsigned int nClientPort, const char* pData, int len )
//{
//	static char szBuf[255] = { 0 };
//	if( len==1 && pData[0]=='#' )
//	{
//		static DWORD dwTime = 0;
//		static DWORD dwLastTime = 0;
//		static DWORD dwCount = 0;
//
//		// 每五秒取一次人数,等待龚健的取总数的新接口Jerry
//		dwTime = ::GetTickCount();
//		if( dwTime>dwLastTime )
//		{
//			dwCount = g_gtsvr->cli_conn->GetSockTotal();
//			dwLastTime = dwTime + 60000;
//		}
//
//		//sprintf( szBuf, "%d,%d,%d", dwCount, _nShowMin, _nShowMax );
//		_snprintf_s( szBuf, sizeof(szBuf), _TRUNCATE, "%d,%d,%d", dwCount, _nShowMin, _nShowMax );
//		pUdpServer->Send( szClientIP, nClientPort, szBuf, (unsigned int)strlen(szBuf) );
//	}
//}

// 全局 GateServer 对象
std::unique_ptr<GateServer> g_GateServer{nullptr};
bool volatile g_appexit = false;
