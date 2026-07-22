#include "gateserver.h"

ToClient::ToClient(char const* fname, ThreadPool* proc, ThreadPool* comm)
	: TcpServerApp(this, proc, comm, false), RPCMGR(this) {
	IniFile inf(fname);
	m_maxcon = atoi(inf["ToClient"]["MaxConnection"]); //Modify by lark.li
	char buffer[255];
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GS_TOCLIENT_CPP_00001), m_maxcon);
	std::cout << buffer << std::endl;

	m_version = atoi(inf["Main"]["Version"]);
	IniSection& is = inf["ToClient"];
	cChar* ip = is["IP"];
	uShort port = atoi(is["Port"]);
	_comm_enc = atoi(is["CommEncrypt"]);

#ifdef NET_CAL

	m_checkSpan = atoi(is["CheckSpan"]);
	m_checkWaring = atoi(is["CheckWaring"]);
	m_checkError = atoi(is["CheckError"]);

	// 最小一秒
	if (m_checkSpan < 1) {
		m_checkSpan = 1;
	}

	// 每秒最少五次
	if (m_checkWaring < (5 * m_checkSpan)) {
		m_checkWaring = (5 * m_checkSpan);
	}

	// 每秒最少十次
	if (m_checkError < (7 * m_checkSpan)) {
		m_checkError = (7 * m_checkSpan);
	}

#endif

	//SetPKParse(0, 2, 32 * 1024, 100); BeginWork(atoi(is["EnablePing"]),1);
	SetPKParse(0, 2, 16 * 1024, 100);
	BeginWork(atoi(is["EnablePing"]), 1);

	if (OpenListenSocket(port, ip) != 0)
		THROW_EXCP(excp, "ToClient listen failed\n");
}

ToClient::~ToClient() {
	m_atexit = 1;
	while (m_calltotal) {
		Sleep(1);
	}
	ShutDown(12 * 1000);
}
#ifdef NET_CAL

void ToClient::SetCheckSpan(uShort checkSpan) {
	if (checkSpan > 1) {
		// 每秒最少十次
		if (m_checkError < (7 * checkSpan))
			m_checkError = (7 * checkSpan);

		// 每秒最少五次
		if (m_checkWaring < (5 * checkSpan))
			m_checkWaring = (5 * checkSpan);

		m_checkSpan = checkSpan;
	}
}

void ToClient::SetCheckWaring(uShort checkWaring) {
	if (checkWaring > (5 * m_checkSpan) && checkWaring < m_checkError) {
		m_checkWaring = checkWaring;
	}
}

void ToClient::SetCheckError(uShort checkError) {
	if (checkError > (7 * m_checkSpan))
		m_checkError = checkError;
}

#endif

bool ToClient::DoCommand(DataSocket* datasock, cChar* cmdline) {
	if (!strncmp(cmdline, "getbandwidth", sizeof("getbandwidth"))) {
		BandwidthStat l_band = GetBandwidthStat();
		char buffer[255];
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GS_TOCLIENT_CPP_00002), GetSockTotal(),
					l_band.m_sendpktps, l_band.m_sendpkts, l_band.m_sendbyteps / 1024, l_band.m_sendbytes / 1024,
					l_band.m_recvpktps, l_band.m_recvpkts, l_band.m_recvbyteps / 1024, l_band.m_recvbytes / 1024);

		WPacket wpk = GetWPacket();
		wpk.WriteCmd(CMD_MC_SYSINFO);
		wpk.WriteString(buffer);
		SendData(datasock, wpk);
		return true;
	}
	if (!strncmp(cmdline, "GameState", sizeof("GameState"))) {
		auto player = static_cast<Player*>(datasock->GetPointer());

		if (player && g_GateServer->toGameServer) {
			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_TM_STATE);
			wpk.WriteLong(player->m_dbid);
			wpk.WriteAddress(ToAddress(player));
			g_GateServer->toGameServer->SendAllGameServer(wpk);
		}
		return true;
	}
	return false;
}
bool ToClient::OnConnect(DataSocket* datasock) //返回值:true-允许连接,false-不允许连接
{
	if (!g_GateServer->toGroupServer->IsReady()) {
		LogLine l_line(g_gatelog);
		l_line << newln << "client: " << datasock->GetPeerIP() << "	come, groupserver is't ready, force disconnect...";
		return false;
	}

	if (GetSockTotal() <= m_maxcon) {
		datasock->SetRecvBuf(64 * 1024);
		datasock->SetSendBuf(64 * 1024);
		LogLine l_line(g_gatelog);
		l_line << newln << "client: " << datasock->GetPeerIP() << "	come...Socket num: " << GetSockTotal() + 1;
		return true;
	} else {
		LogLine l_line(g_gatelog);
		l_line << newln << "client: " << datasock->GetPeerIP() << "	come, greater than " << m_maxcon << " player, force disconnect...";
		return false;
	}
}
void ToClient::OnConnected(DataSocket* datasock) {
	Player* player = g_GateServer->player_heap.Get();
	if (!player) {
		printf("error: poor mem %s!\n", datasock->GetPeerIP());
		Disconnect(datasock);
		return;
	}

	if (!player->InitReference(datasock)) //阻止重复进入
	{
		printf("warning: forbid %s repeat connect !", datasock->GetPeerIP());
		player->Free();
		Disconnect(datasock);
		return;
	}
#ifdef NET_CAL
	player->m_cmdNum = 0;
	player->m_waringNum = 0;
	player->m_lashTick = GetCurrentTick();
#endif

	SYSTEMTIME l_st;
	::GetLocalTime(&l_st);

	auto size = std::snprintf(nullptr, 0, "[%02d-%02d %02d:%02d:%02d:%03d]", l_st.wMonth, l_st.wDay, l_st.wHour, l_st.wMinute, l_st.wSecond, l_st.wMilliseconds);
	std::string output(size + 1, '\0');
	_snprintf_s(&output[0], size + 1, _TRUNCATE, "[%02d-%02d %02d:%02d:%02d:%03d]", l_st.wMonth, l_st.wDay, l_st.wHour, l_st.wMinute, l_st.wSecond, l_st.wMilliseconds);

	player->m_chapstr = output;

	WPacket wpk = GetWPacket();
	wpk.WriteCmd(CMD_MC_CHAPSTR);
	wpk.WriteString(player->m_chapstr.c_str());
	SendData(datasock, wpk);

#if NET_PROTOCOL_ENCRYPT
	static int g_sKeyData = short(g_GateServer->toClient->GetVersion() * g_GateServer->toClient->GetVersion() * 0x1232222);
	int nNoise = g_sKeyData * int(*(int*)(player->m_chapstr + player->m_chapstr.len() - 4));
	init_Noise(nNoise, player->m_szSendKey);
	init_Noise(nNoise, player->m_szRecvKey);
#endif
}
void ToClient::OnDisconnect(DataSocket* datasock, int reason) {
	LogLine l_line(g_gatelog);
	l_line << newln << "client: " << datasock->GetPeerIP() << " gone...Socket num: " << GetSockTotal() << " ,reason=" << GetDisconnectErrText(reason).c_str();
	l_line << endln;
	RPacket rpkt = nullptr;
	CM_LOGOUT(datasock, rpkt);
}

std::string ToClient::GetDisconnectErrText(int reason) const {
	switch (reason) {
	case -21:
		return RES_STRING(GS_TOCLIENT_CPP_00011);
	case -23:
		return RES_STRING(GS_TOCLIENT_CPP_00012);
	case -24:
		return RES_STRING(GS_TOCLIENT_CPP_00013);
	case -25:
		return RES_STRING(GS_TOCLIENT_CPP_00014);
	case -27:
		return RES_STRING(GS_TOCLIENT_CPP_00015);
	case -29:
		return RES_STRING(GS_TOCLIENT_CPP_00016);
	case -31:
		return RES_STRING(GS_TOCLIENT_CPP_00017);
	default:
		return TcpServerApp::GetDisconnectErrText(reason);
	}
}

WPacket ToClient::OnServeCall(DataSocket* datasock, RPacket& in_para) {
	const uShort cmd = in_para.ReadCmd();
	switch (cmd) {
	case CMD_CM_LOGOUT: {
		CM_LOGOUT(datasock, in_para);
		Disconnect(datasock, 65, -27);
		return WPacket(nullptr);
	}
	default: { //"Default forwarding to GameServer"
		break;
	}
	}
	return WPacket(nullptr);
}

void ToClient::OnProcessData(DataSocket* datasock, RPacket& recvbuf) {
	const uShort cmd = recvbuf.ReadCmd();
	//LG("ToClient", "-->l_cmd = %d\n", l_cmd);
	try {
#ifdef NET_CAL
		auto player = static_cast<Player*>(datasock->GetPointer());
		if (player) {
			const uLong tick = GetCurrentTick();
			if (tick - player->m_lashTick < 1000u * m_checkSpan) {
				player->m_cmdNum++;

				if (player->m_cmdNum > m_checkError) {
					std::cout << "[" << datasock->GetPeerIP() << "] Disconnect" << player->m_actid << " cmd[" << cmd << "] (" << player->m_cmdNum << ") error!\n";
					LogLine l_line(g_chkattack);
					l_line << newln << "[" << datasock->GetPeerIP() << "] Disconnect" << player->m_actid << " cmd=" << cmd << " (" << player->m_cmdNum << ")error!";

					dbc::WPacket wpk = GetWPacket();
					wpk.WriteCmd(CMD_MC_LOGIN);
					wpk.WriteShort(ERR_MC_NETEXCP); //"error code"
					SendData(datasock, wpk);		//"send to client"
					this->Disconnect(datasock, 100, -31);
				} else if (player->m_cmdNum > m_checkWaring) {
					//std::cout<<"["<<datasock->GetPeerIP()<<"] id =" << player->m_actid << " sending cmd[" <<l_cmd << "] (" << player->m_cmdNum << ") waring!\n";
					LogLine l_line(g_chkattack);
					l_line << newln << "[" << datasock->GetPeerIP() << "] " << player->m_actid << " cmd=" << cmd << " (" << player->m_cmdNum << ")waring!";

					player->m_waringNum++;

					// "If the number of consecutive warnings is too high"
					if (player->m_waringNum > 2) {
						std::cout << "[" << datasock->GetPeerIP() << "] Disconnect" << player->m_actid << " cmd[" << cmd << "] (" << player->m_cmdNum << ") more warring !\n";

						LogLine l_line(g_chkattack);
						l_line << newln << "[" << datasock->GetPeerIP() << "] Disconnect" << player->m_actid << " cmd=" << cmd << " (" << player->m_cmdNum << ")more warring!";

						dbc::WPacket wpk = GetWPacket();
						wpk.WriteCmd(CMD_MC_LOGIN);
						wpk.WriteShort(ERR_MC_NETEXCP); //"error code"
						SendData(datasock, wpk);		//"send to client"
						this->Disconnect(datasock, 100, -31);
					}
				}
			} else {
				//std::cout<<"["<<datasock->GetPeerIP()<<"] id =" << player->m_actid << " sending cmd " << player->m_cmdNum << " !\n";
				if (player->m_cmdNum < m_checkWaring) {
					// "Not continuous, the number of warnings canceled"
					player->m_waringNum = 0;
				}

				player->m_cmdNum = 0;
				player->m_lashTick = tick;
			}
		}
#endif

		switch (cmd) {
		case CMD_CM_LOGIN:   //Accept the username/password pair for authentication and return a list of valid roles on all server groups under the username.
		case CMD_CM_LOGOUT:  //Synchronous call
		case CMD_CM_BGNPLAY: //Receive the selected role name into the GroupServer verification, and then notify GameServer to make the role into the body map space.
		case CMD_CM_ENDPLAY:
		case CMD_CM_NEWCHA:
		case CMD_CM_DELCHA:
		case CMD_CM_CREATE_PASSWORD2:
		case CMD_CM_UPDATE_PASSWORD2: {
			// "If the GroupServer is not ready, of course you can't log in."
			if (g_GateServer->toGroupServer->IsReady()) {
				++m_calltotal;
				if (m_atexit) {
					--m_calltotal;
					return;
				}
				TransmitCall* l_tc = g_GateServer->m_tch.Get();
				l_tc->Init(datasock, recvbuf);
				GetProcessor()->AddTask(l_tc);
			} else {
				LG("ToGroupServerError", "l_cmd = %d Login \n", cmd);
				dbc::WPacket wpk = GetWPacket();
				wpk.WriteCmd(CMD_MC_LOGIN);
				wpk.WriteShort(ERR_MC_NETEXCP); //"error code"
				SendData(datasock, wpk);		//"send to client"
				this->Disconnect(datasock, 100, -31);
			}
		} break;
		case CMD_CP_PING: {
			if (g_GateServer->toGroupServer->IsReady()) {
				auto player = static_cast<Player*>(datasock->GetPointer());
				if (player && player->gp_addr && player->gm_addr) {
					WPacket wpk = GetWPacket();
					wpk.WriteCmd(CMD_CP_PING);
					wpk.WriteLong(GetTickCount() - player->m_pingtime);
					player->m_pingtime = 0;

					wpk.WriteAddress(ToAddress(player));
					wpk.WriteAddress(player->gp_addr);
					g_GateServer->toGroupServer->SendData(g_GateServer->toGroupServer->get_datasock(), wpk);
				}
			} else {
				LG("ToGroupServerError", "l_cmd = %d Ping \n", cmd);
				dbc::WPacket wpk = GetWPacket();
				wpk.WriteCmd(CMD_MC_LOGIN);
				wpk.WriteShort(ERR_MC_NETEXCP); //"error code"
				SendData(datasock, wpk);		//"send to client"
				this->Disconnect(datasock, 100, -31);
			}
		} break;
		case CMD_CM_SAY: {
			const cChar* l_str = recvbuf.ReadString();
			if (!l_str) {
				break;
			}
			if (l_str[0] == '&' && DoCommand(datasock, l_str + 1)) {
				break;
			}

			auto player = static_cast<Player*>(datasock->GetPointer());
			if (player && player->m_estop) {
				if (GetTickCount() - player->m_lestoptick >= 1000 * 60 * 2) {
					WPacket wpk = GetWPacket();
					wpk.WriteCmd(CMD_TP_ESTOPUSER_CHECK);
					wpk.WriteLong(player->m_actid);
					g_GateServer->toGroupServer->SendData(g_GateServer->toGroupServer->get_datasock(), wpk);
				}
				WPacket wpk = GetWPacket();
				wpk.WriteCmd(CMD_MC_SYSINFO);
				wpk.WriteString(RES_STRING(GS_TOCLIENT_CPP_00018));
				g_GateServer->toGroupServer->SendData(player->m_datasock, wpk);
				break;
			}
		}
		default:								  //"Default forwarding to GroupServer"
			if (cmd / 500 == CMD_CM_BASE / 500) { //"Forward to GameServer"
				auto player = static_cast<Player*>(datasock->GetPointer());
				if (player) {
					uintptr_t l_gpaddr = player->gp_addr;
					uintptr_t l_gmaddr = player->gm_addr;
					GameServer* l_game = player->game;

					if (l_gpaddr && l_gmaddr && l_game) {
						WPacket wpk = WPacket(recvbuf).Duplicate();
						wpk.WriteAddress(ToAddress(player));
						wpk.WriteAddress(l_gmaddr);
						g_GateServer->toGameServer->SendData(player->game->m_datasock, wpk);
					}
				}
			} else if (cmd / 500 == CMD_CP_BASE / 500) { //"Forward to GroupServer"
				auto player = static_cast<Player*>(datasock->GetPointer());
				if (player) {
					//"Send messages when GroupServer is ready to avoid blocking"
					if (g_GateServer->toGroupServer->IsReady()) {
						uintptr_t l_gpaddr = player->gp_addr;
						uintptr_t l_gmaddr = player->gm_addr;
						if (l_gpaddr && l_gmaddr) {
							WPacket wpk = WPacket(recvbuf).Duplicate();
							wpk.WriteAddress(ToAddress(player));
							wpk.WriteAddress(l_gpaddr);
							g_GateServer->toGroupServer->SendData(g_GateServer->toGroupServer->get_datasock(), wpk);
						}
					} else {
						LG("ToGroupServerError", "l_cmd = %d IsReady \n", cmd);
						dbc::WPacket wpk = GetWPacket();
						wpk.WriteCmd(CMD_MC_LOGIN);
						wpk.WriteShort(ERR_MC_NETEXCP); //"error code"
						SendData(datasock, wpk);		//"send to client"
						this->Disconnect(datasock, 100, -31);
					}
				}
			}
			break;
		}
		static uLong l_last = GetTickCount();
		if (datasock->m_recvbyteps > 1024 * 5) {
			uLong l_tick = GetCurrentTick();
			if (l_tick - l_last > 1000 * 30) {
				l_last = l_tick;
				std::cout << "[" << datasock->GetPeerIP() << "] sending big packet (>5K/s) attack server,please deal with!\n";
				LogLine l_line(g_chkattack);
				l_line << newln << "[" << datasock->GetPeerIP() << "] sending big packet (>5K/s) attack server,please deal with!";
			}
		}
	} catch (...) {
		LG("ToClientError", "l_cmd = %d\n", cmd);
	}
	//LG("ToClient", "<--l_cmd = %d\n", l_cmd);
}
long TransmitCall::Process() {
	LogLine l_line(g_gatelog);

	if (!g_GateServer->toGroupServer->IsReady()) {
		g_GateServer->toClient->Disconnect(m_datasock, 50, -27);
		l_line << newln << "IsReady = false";
	}

	const uShort cmd = m_recvbuf.ReadCmd();

	//l_line<<newln<<"st:"<<l_cmd;

	try {
		switch (cmd) {
		case CMD_CM_LOGIN: { // 接受用户名/密码对进行认证,返回用户名下的所有服务器组上的有效角色列表.
			g_GateServer->toClient->CM_LOGIN(m_datasock, m_recvbuf);
		} break;
		case CMD_CM_LOGOUT: { //同步调用
			g_GateServer->toClient->CM_LOGOUT(m_datasock, m_recvbuf);
			g_GateServer->toClient->Disconnect(m_datasock, 50, -27);
		}

		break;
		case CMD_CM_BGNPLAY: { //接收选择的角色名进入GroupServer验证，然后通知GameServer使角色进入体图空间.
			g_GateServer->toClient->CM_BGNPLAY(m_datasock, m_recvbuf);
		} break;
		case CMD_CM_ENDPLAY: {
			g_GateServer->toClient->CM_ENDPLAY(m_datasock, m_recvbuf);
		} break;
		case CMD_CM_NEWCHA: {
			g_GateServer->toClient->CM_NEWCHA(m_datasock, m_recvbuf);
		} break;
		case CMD_CM_DELCHA: {
			g_GateServer->toClient->CM_DELCHA(m_datasock, m_recvbuf);
		} break;
		case CMD_CM_CREATE_PASSWORD2: {
			g_GateServer->toClient->CM_CREATE_PASSWORD2(m_datasock, m_recvbuf);
		} break;
		case CMD_CM_UPDATE_PASSWORD2: {
			g_GateServer->toClient->CM_UPDATE_PASSWORD2(m_datasock, m_recvbuf);
		} break;
		}
	} catch (...) {
		try {
			g_GateServer->toClient->Disconnect(m_datasock, 50, -27);
		} catch (...) {
		}
		l_line << newln << "IsReady = false exception:" << cmd;
	}

	--(g_GateServer->toClient->m_calltotal);
	//l_line<<newln<<"st:"<<l_cmd;
	return 0;
}

void ToClient::CM_LOGIN(DataSocket* datasock, RPacket& recvbuf) {
	uLong l_ulMilliseconds = 30 * 1000;
	uLong tick = GetTickCount() - recvbuf.GetTickCount();
	if (l_ulMilliseconds > tick) {
		l_ulMilliseconds = l_ulMilliseconds - tick;

		if (m_version != recvbuf.ReverseReadShort()) {
			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_MC_LOGIN);
			wpk.WriteShort(ERR_MC_VER_ERROR); //错误码
			SendData(datasock, wpk);		  // 发给客户端
			LogLine l_line(g_gatelog);
			l_line << newln << "client: " << datasock->GetPeerIP() << "	login error: client and server can't match!";
			Disconnect(datasock, 100, -31);
			return;
		}

		recvbuf.DiscardLast(static_cast<uLong>(sizeof(uShort)));

		ToGroupServer* l_gps = g_GateServer->toGroupServer.get();
		auto player = static_cast<Player*>(datasock->GetPointer());
		if (!player) { //组织重复进入
			return;
		}

		//Add by sunny.sun 20081223
		std::string szLocale = recvbuf.ReadString();

		WPacket wpk = WPacket(recvbuf).Duplicate();
		wpk.WriteCmd(CMD_TP_USER_LOGIN);
		wpk.WriteString(player->m_chapstr.c_str());
		wpk.WriteLong(inet_addr(datasock->GetPeerIP()));
		wpk.WriteAddress(ToAddress(player)); // 附加上在GateServer上的内存地址

		RPacket rpkFromGroupServer = l_gps->SyncCall(wpk, l_ulMilliseconds);
		if (rpkFromGroupServer.HasData() == 0) {
			wpk = GetWPacket();
			wpk.WriteCmd(CMD_MC_LOGIN);
			wpk.WriteShort(ERR_MC_NETEXCP); //错误码
			SendData(datasock, wpk);		// 发给客户端
			LogLine ll_line(g_gatelog);
			ll_line << newln << "client: " << datasock->GetPeerIP() << "	login error: GroupServer is disresponse!" << endln;
			Disconnect(datasock, 100, -31);
			return;
		}
		if (auto l_errno = rpkFromGroupServer.ReadShort()) {
			wpk = rpkFromGroupServer;
			wpk.WriteCmd(CMD_MC_LOGIN);
			SendData(datasock, wpk);
			LogLine l_line(g_gatelog);
			l_line << newln << "client: " << datasock->GetPeerIP() << "	login error, error:" << l_errno << endln;
			Disconnect(datasock, 100, -31);
			return;
		}

		player->m_status = 1; //置于选/建/删角色状态
		//Add by sunny.sun 20090205
		std::string sessionKey = rpkFromGroupServer.ReadString();
		player->gp_addr = rpkFromGroupServer.ReverseReadAddress();   //保存玩家在GroupServer上的内存地址
		player->m_loginID = rpkFromGroupServer.ReverseReadLong(); //  Account DB id
		player->m_actid = rpkFromGroupServer.ReverseReadLong();
		BYTE byPassword = rpkFromGroupServer.ReverseReadChar();
		player->comm_key_len = rpkFromGroupServer.ReverseReadShort();
		memcpy(player->comm_textkey, rpkFromGroupServer.GetDataAddr() + rpkFromGroupServer.GetDataLen() - 15 - player->comm_key_len, player->comm_key_len);
		rpkFromGroupServer.DiscardLast(sizeof(uintptr_t) + sizeof(uLong) * 2 + sizeof(uChar) + sizeof(uShort) + player->comm_key_len + sizeof(uShort));

		wpk = WPacket(rpkFromGroupServer).Duplicate();
		wpk.WriteCmd(CMD_MC_LOGIN);
		wpk.WriteChar(byPassword);
		wpk.WriteLong(_comm_enc);
		wpk.WriteLong(0x3214);
		SendData(datasock, wpk);
		LogLine l_line(g_gatelog);
		l_line << newln << "client: " << datasock->GetPeerIP() << "	login ok." << endln;

		// 开始加密
		player->enc = true;

		// Add by lark.li 20081119 begin
		player->BeginRun();
		// End

		// Add by lark.li 20090112 begin
		//g_gtsvr->m_ToMMS->Login(datasock->GetPeerIP(), datasock->GetPeerPort(), player->m_actid);
		// End
	} else {
		WPacket wpk = GetWPacket();
		wpk.WriteCmd(CMD_MC_LOGIN);
		wpk.WriteShort(ERR_MC_NETEXCP); //错误码
		SendData(datasock, wpk);		// 发给客户端
		LogLine l_line(g_gatelog);
		l_line << newln << "client: " << datasock->GetPeerIP() << "	login error: packet time out!" << endln;
		Disconnect(datasock, 100, -31);
	}
}

WPacket ToClient::CM_DISCONN(DataSocket* datasock, RPacket& recvbuf) {
	uLong l_ulMilliseconds = 30 * 1000;
	uLong l_tick = GetTickCount() - recvbuf.GetTickCount();
	l_ulMilliseconds = (l_ulMilliseconds > l_tick) ? l_ulMilliseconds - l_tick : 1;

	std::unique_lock<std::mutex> lock(g_GateServer->_mtxother);
	auto player = static_cast<Player*>(datasock->GetPointer());
	datasock->SetPointer(nullptr);
	lock.unlock();

	WPacket ret_wpk(nullptr);
	if (player) {
		// Add by lark.li 2008119 begin
		player->EndRun();
		// End

		std::lock_guard<std::mutex> l_lockStat(player->m_mtxstat);

		if (player->m_status == 0) //发出这个命令时机非法，因为当前玩家不处于选角色状态，不能选择另外一个角色
		{
			// NOTE: wpk != ret_wpk
			WPacket wpk = datasock->GetWPacket();
			ret_wpk.WriteShort(ERR_MC_NOTLOGIN);
		} else {
			//printf( "CM_LOGOUT 2.) id = %d, status = %d\n", player->m_actid, player->m_status );
			WPacket wpk = GetWPacket();
			wpk.WriteCmd(CMD_TP_DISC);
			wpk.WriteLong(player->m_actid);
			wpk.WriteLong(inet_addr(datasock->GetPeerIP()));
			wpk.WriteString(GetDisconnectErrText(datasock->GetDisconnectReason() ? datasock->GetDisconnectReason() : -27).c_str());
			g_GateServer->toGroupServer->SendData(g_GateServer->toGroupServer->get_datasock(), wpk);

			GameServer* l_game = player->game;
			if ((player->m_status == 2) && player->gm_addr && l_game && l_game->m_datasock) {
				//通知GameServer GoOut地图
				LogLine l_line(g_gatelog);
				l_line << newln << "client: " << datasock->GetPeerIP() << ":" << datasock->GetPeerPort() << "	GoOut map,Gate to["
					   << l_game->m_datasock->GetPeerIP() << "] send GoOutMap command,dbid:" << player->m_dbid
					   << std::uppercase << std::hex << ",Gate address:" << ToAddress(player) << ",Game address:" << player->gm_addr << std::dec << std::nouppercase << endln;

				WPacket wpk = l_game->m_datasock->GetWPacket();
				wpk.WriteCmd(CMD_TM_GOOUTMAP);
				wpk.WriteChar(0);
				wpk.WriteAddress(ToAddress(player));
				wpk.WriteAddress(player->gm_addr); //附加上在GameServer上的地址
				player->game = nullptr;			//阻止后面的到GameServer的数据
				player->gm_addr = 0;			//阻止后面的到GameServer的数据
				SendData(l_game->m_datasock, wpk);
			}
			//通知GroupServer Logout
			wpk = g_GateServer->toGroupServer->get_datasock()->GetWPacket();
			wpk.WriteCmd(CMD_TP_USER_LOGOUT);
			wpk.WriteAddress(ToAddress(player));
			wpk.WriteAddress(player->gp_addr);
			player->gp_addr = 0;
			ret_wpk = g_GateServer->toGroupServer->SyncCall(wpk, l_ulMilliseconds);
		}
		player->Free();
	}
	return ret_wpk;
}

WPacket ToClient::CM_LOGOUT(DataSocket* datasock, RPacket& recvbuf) {
	uLong l_ulMilliseconds = 30 * 1000;
	uLong l_tick = GetTickCount() - recvbuf.GetTickCount();
	l_ulMilliseconds = (l_ulMilliseconds > l_tick) ? l_ulMilliseconds - l_tick : 1;

	std::unique_lock<std::mutex> lock(g_GateServer->_mtxother);
	auto player = static_cast<Player*>(datasock->GetPointer());
	if (player) {
		player->m_datasock = nullptr;
	}
	datasock->SetPointer(nullptr);
	lock.unlock();

	WPacket ret_wpk(nullptr);
	if (player) {
		// Add by lark.li 20081119 begin
		player->EndRun();
		// End

		// Add by lark.li 20090112 begin
		//g_gtsvr->m_ToMMS->Logout(datasock->GetPeerIP(), datasock->GetPeerPort(), player->m_actid);
		// End

		std::lock_guard<std::mutex> l_lockStat(player->m_mtxstat);

		try {
			if (player->m_status == 0) //发出这个命令时机非法，因为当前玩家不处于选角色状态，不能选择另外一个角色
			{
				//NOTE: wpk != ret_wpk
				WPacket wpk = datasock->GetWPacket();
				ret_wpk.WriteShort(ERR_MC_NOTLOGIN);
			} else {
				WPacket wpk = GetWPacket();
				wpk.WriteCmd(CMD_TP_DISC);
				wpk.WriteLong(player->m_actid);
				wpk.WriteLong(inet_addr(datasock->GetPeerIP()));
				wpk.WriteString(GetDisconnectErrText(datasock->GetDisconnectReason() ? datasock->GetDisconnectReason() : -27).c_str());
				g_GateServer->toGroupServer->SendData(g_GateServer->toGroupServer->get_datasock(), wpk);

				/*
				//int tick = 0;

				//while(player->m_switch && tick < 5)
				//{
				//	::Sleep(5);
				//	LogLine l_line(g_gatelog);
				//	//l_line<<newln<<"客户端: "<<datasock->GetPeerIP()<<":"<<datasock->GetPeerPort()<<"	GoOut出地图,Gate向["
				//	l_line<<newln<<"client: "<<player->m_dbid <<" switch map!" << tick;

				//	tick ++;
				//}
				*/

				if (player->m_switch) {
					LogLine l_line(g_gatelog);
					//l_line<<newln<<"客户端: "<<datasock->GetPeerIP()<<":"<<datasock->GetPeerPort()<<"	GoOut出地图,Gate向["
					l_line << newln << "client: " << player->m_dbid << " switch map error!";
				}

				GameServer* l_game = player->game;
				if ((player->m_status == 2) && player->gm_addr && l_game && l_game->m_datasock) {
					//通知GameServer GoOut地图
					LogLine l_line(g_gatelog);
					l_line << newln << "client: " << datasock->GetPeerIP() << ":" << datasock->GetPeerPort() << "	GoOut map,Gate to ["
						   << l_game->m_datasock->GetPeerIP() << "]send GoOutMap command,dbid:" << player->m_dbid
						   << std::uppercase << std::hex << ",Gate address:" << ToAddress(player) << ",Game address:" << player->gm_addr << std::dec << std::nouppercase << endln;

					WPacket wpk = l_game->m_datasock->GetWPacket();
					wpk.WriteCmd(CMD_TM_GOOUTMAP);
					wpk.WriteChar(0);
					wpk.WriteAddress(ToAddress(player));
					wpk.WriteAddress(player->gm_addr); //附加上在GameServer上的地址
					player->game = nullptr;			//阻止后面的到GameServer的数据
					player->gm_addr = 0;			//阻止后面的到GameServer的数据
					SendData(l_game->m_datasock, wpk);
				}
				//通知GroupServer Logout
				wpk = g_GateServer->toGroupServer->get_datasock()->GetWPacket();
				wpk.WriteCmd(CMD_TP_USER_LOGOUT);
				wpk.WriteAddress(ToAddress(player));
				wpk.WriteAddress(player->gp_addr);
				player->gp_addr = 0;
				ret_wpk = g_GateServer->toGroupServer->SyncCall(wpk, l_ulMilliseconds);
			}
		} catch (...) {
			LogLine l_line(g_gatelog);
			l_line << newln << "Error exit!";
		}
#ifdef NET_CAL
		player->m_cmdNum = 0;
		player->m_waringNum = 0;
		player->Free();
#endif
	}
	return ret_wpk;
}

void ToClient::CM_BGNPLAY(DataSocket* datasock, RPacket& recvbuf) {
	uLong l_ulMilliseconds = 30 * 1000;
	uLong l_tick = GetTickCount() - recvbuf.GetTickCount();
	if (l_ulMilliseconds > l_tick) {
		l_ulMilliseconds = l_ulMilliseconds - l_tick;

		if (auto player = static_cast<Player*>(datasock->GetPointer()); player) {
			std::lock_guard<std::mutex> l_lockStat(player->m_mtxstat);
			if (player->m_status != 1 || !player->gp_addr) {
				//"The timing of issuing this command is illegal because the current player is not in the selected role state and cannot select another role."
				WPacket wpk = datasock->GetWPacket();
				wpk.WriteCmd(CMD_MC_BGNPLAY);
				wpk.WriteShort(ERR_MC_NOTSELCHA);
				SendData(datasock, wpk);
				return;
			}

			//"Verify the legality of the role played"
			WPacket wpk = WPacket(recvbuf).Duplicate();
			wpk.WriteCmd(CMD_TP_BGNPLAY);
			wpk.WriteAddress(ToAddress(player));
			wpk.WriteAddress(player->gp_addr);
			RPacket rpkFromGroupServer = g_GateServer->toGroupServer->SyncCall(wpk, l_ulMilliseconds);
			if (!rpkFromGroupServer.HasData()) { //"Network Error"
				wpk = datasock->GetWPacket();
				wpk.WriteCmd(CMD_MC_BGNPLAY);
				wpk.WriteShort(ERR_MC_NETEXCP);
				SendData(datasock, wpk);
				return;
			}
			if (auto l_errno = rpkFromGroupServer.ReadShort()) { //"The role played is illegal"
				wpk = rpkFromGroupServer;
				wpk.WriteCmd(CMD_MC_BGNPLAY);
				SendData(datasock, wpk);
				if (l_errno == ERR_PT_KICKUSER) {
					Disconnect(datasock, 100, -25);
				}
				return;
			}

			//"Select the role to succeed, return a success message, and send the map name and the successful role ID to the GameServer."
			// Modify by lark.li 20080317
			memset(player->m_password, 0, sizeof(player->m_password));
			//strncpy( player->m_password, rpk.ReadString(), ROLE_MAXSIZE_PASSWORD2 );	//角色二次密码
			strncpy_s(player->m_password, sizeof(player->m_password), rpkFromGroupServer.ReadString(), _TRUNCATE); //角色二次密码
			// End

			player->m_dbid = rpkFromGroupServer.ReadLong();	//角色ID;
			player->m_worldid = rpkFromGroupServer.ReadLong(); //GroupServer分配的唯一ID
			const char* l_map = rpkFromGroupServer.ReadString();
			player->m_sGarnerWiner = rpkFromGroupServer.ReadShort();

			GameServer* l_game = g_GateServer->toGameServer->find(l_map);
			if (!l_game) { //"Target map is unreachable"
				wpk = datasock->GetWPacket();
				wpk.WriteCmd(CMD_MC_BGNPLAY);
				wpk.WriteShort(ERR_MC_NOTARRIVE);
				SendData(datasock, wpk);
				return;
			}
			if (l_game->m_plynum > 15000) { //"Too many people" // NOTE: magic number '15000'
				wpk = datasock->GetWPacket();
				wpk.WriteCmd(CMD_MC_BGNPLAY);
				wpk.WriteShort(ERR_MC_TOOMANYPLY);
				SendData(datasock, wpk);
				return;
			}

			player->m_status = 2; //"Select the role to succeed, put it in the game state"
			l_game->m_plynum++;   //"No synchronization, just a simple reference"
			//"Inform GameServer to enter the map"
			LogLine l_line(g_gatelog);
			l_line << newln << "client: " << datasock->GetPeerIP() << ":" << datasock->GetPeerPort() << "	BeginPlay entry map,Gate to["
				   << l_game->m_datasock->GetPeerIP() << "]send EnterMap command,dbid:" << player->m_dbid
				   << std::uppercase << std::hex << ",Gate address:" << ToAddress(player) << std::dec << std::nouppercase << endln;

			//"Find the GameServer based on the map and then request GameServer to enter this map."
			l_game->EnterMap(player, player->m_loginID, player->m_dbid, player->m_worldid, l_map, -1, 0, 0, 0, player->m_sGarnerWiner);
		}
	} else {
		WPacket wpk = datasock->GetWPacket();
		wpk.WriteCmd(CMD_MC_BGNPLAY);
		wpk.WriteShort(ERR_MC_NETEXCP);
		SendData(datasock, wpk);
	}
}

void ToClient::CM_ENDPLAY(DataSocket* datasock, RPacket& recvbuf) {
	uLong l_ulMilliseconds = 30 * 1000;
	uLong l_tick = GetTickCount() - recvbuf.GetTickCount();
	l_ulMilliseconds = (l_ulMilliseconds > l_tick) ? l_ulMilliseconds - l_tick : 1;

	auto player = static_cast<Player*>(datasock->GetPointer());
	if (player) {
		std::lock_guard<std::mutex> l_lockStat(player->m_mtxstat);

		if (player->m_status != 2 || !player->gm_addr) //"The timing of issuing this command is illegal because the current player is not in a role and cannot reselect another character."
		{
			WPacket wpk = datasock->GetWPacket();
			wpk.WriteCmd(CMD_MC_ENDPLAY);
			wpk.WriteShort(ERR_MC_NOTPLAY);
			SendData(datasock, wpk);
			Disconnect(datasock, 100, -25);
		} else {
			GameServer* l_game = player->game;
			if (l_game && l_game->m_datasock) {

				player->m_status = 1; // "Enter the role screen status"

				l_game->m_plynum--;
				//"Inform GameServer to map"
				LogLine l_line(g_gatelog);
				l_line << newln << "client: " << datasock->GetPeerIP() << ":" << datasock->GetPeerPort() << "	GoOut map,Gate to["
					   << l_game->m_datasock->GetPeerIP() << "] send GoOutMap command,dbid:" << player->m_dbid
					   << std::uppercase << std::hex << ",Gate address:" << ToAddress(player) << ",Game address:" << player->gm_addr << std::dec << std::nouppercase << endln;

				WPacket wpk = WPacket(recvbuf).Duplicate();
				wpk.WriteCmd(CMD_TM_GOOUTMAP);
				wpk.WriteChar(0);
				wpk.WriteAddress(ToAddress(player));
				wpk.WriteAddress(player->gm_addr); //附加GameServer上的地址
				player->game = nullptr;			//阻止后面的到GameServer的数据
				player->gm_addr = 0;			//阻止后面的到GameServer的数据
				g_GateServer->toGameServer->SendData(l_game->m_datasock, wpk);
				//g_gtsvr->gm_conn->SyncCall(l_game->m_datasock,wpk);

				//通知GroupServer结束玩游戏
				wpk = WPacket(recvbuf).Duplicate();
				wpk.WriteCmd(CMD_TP_ENDPLAY);
				wpk.WriteAddress(ToAddress(player));
				wpk.WriteAddress(player->gp_addr);
				wpk = g_GateServer->toGroupServer->SyncCall(wpk, l_ulMilliseconds);
				if (!wpk.HasData()) {
					wpk = datasock->GetWPacket();
					wpk.WriteCmd(CMD_MC_ENDPLAY);
					wpk.WriteShort(ERR_MC_NETEXCP);
					SendData(datasock, wpk);
					Disconnect(datasock, 100, -25);
				} else {
					const auto err = RPacket(wpk).ReadShort();
					if (err == ERR_PT_INERR || err == ERR_PT_KICKUSER) {
						Disconnect(datasock, 100, -25);
					} else {
						//返回给Client
						wpk.WriteCmd(CMD_MC_ENDPLAY);
						SendData(datasock, wpk);
						player->m_dbid = 0;
						player->m_worldid = 0;
					}

					////返回给Client
					//wpk.WriteCmd(CMD_MC_ENDPLAY);
					//SendData(datasock,wpk);
					//player->m_dbid	=0;
					//player->m_worldid=0;
					//if(RPacket(wpk).ReadShort() ==ERR_PT_KICKUSER)
					//{
					//	Disconnect(datasock,100,-25);
					//}
				}
			}
		}
	}
}

void ToClient::CM_NEWCHA(DataSocket* datasock, RPacket& recvbuf) {
	uLong l_ulMilliseconds = 30 * 1000;
	uLong l_tick = GetTickCount() - recvbuf.GetTickCount();
	if (l_ulMilliseconds > l_tick) {
		l_ulMilliseconds = l_ulMilliseconds - l_tick;

		auto player = static_cast<Player*>(datasock->GetPointer());
		if (player) {
			std::lock_guard<std::mutex> l_lockStat(player->m_mtxstat);
			if (player->m_status != 1 || !player->gp_addr) {
				WPacket wpk = datasock->GetWPacket();
				wpk.WriteCmd(CMD_MC_NEWCHA);
				wpk.WriteShort(ERR_MC_NOTSELCHA);
				SendData(datasock, wpk);
			} else {
				//调用GroupServer
				WPacket wpk = WPacket(recvbuf).Duplicate();
				wpk.WriteCmd(CMD_TP_NEWCHA);
				wpk.WriteAddress(ToAddress(player));
				wpk.WriteAddress(player->gp_addr); //附带地址
				wpk = g_GateServer->toGroupServer->SyncCall(wpk, l_ulMilliseconds);
				if (!wpk.HasData()) {
					wpk = datasock->GetWPacket();
					wpk.WriteCmd(CMD_MC_NEWCHA);
					wpk.WriteShort(ERR_MC_NETEXCP);
					SendData(datasock, wpk);
					Disconnect(datasock, 100, -25);
				} else {
					//返回Client
					wpk.WriteCmd(CMD_MC_NEWCHA);
					SendData(datasock, wpk);
					if (RPacket(wpk).ReadShort() == ERR_PT_KICKUSER) {
						Disconnect(datasock, 100, -25);
					}
				}
			}
		}
	} else {
		WPacket wpk = datasock->GetWPacket();
		wpk.WriteCmd(CMD_MC_NEWCHA);
		wpk.WriteShort(ERR_MC_NETEXCP);
		SendData(datasock, wpk);
		Disconnect(datasock, 100, -25);
	}
}
void ToClient::CM_DELCHA(DataSocket* datasock, RPacket& recvbuf) {
	uLong l_ulMilliseconds = 30 * 1000;
	uLong l_tick = GetTickCount() - recvbuf.GetTickCount();
	if (l_ulMilliseconds > l_tick) {
		l_ulMilliseconds = l_ulMilliseconds - l_tick;

		auto player = static_cast<Player*>(datasock->GetPointer());
		if (player) {
			std::lock_guard<std::mutex> l_lockStat(player->m_mtxstat);
			if (player->m_status != 1 || !player->gp_addr) {
				WPacket wpk = datasock->GetWPacket();
				wpk.WriteCmd(CMD_MC_DELCHA);
				wpk.WriteShort(ERR_MC_NOTSELCHA);
				SendData(datasock, wpk);
			} else {
				//调用GroupServer
				WPacket wpk = WPacket(recvbuf).Duplicate();
				wpk.WriteCmd(CMD_TP_DELCHA);
				wpk.WriteAddress(ToAddress(player));
				wpk.WriteAddress(player->gp_addr); //附带地址
				wpk = g_GateServer->toGroupServer->SyncCall(wpk);
				if (!wpk.HasData()) {
					wpk = datasock->GetWPacket();
					wpk.WriteCmd(CMD_MC_DELCHA);
					wpk.WriteShort(ERR_MC_NETEXCP);
					SendData(datasock, wpk);
					Disconnect(datasock, 100, -25);
				} else {
					//返回Client
					wpk.WriteCmd(CMD_MC_DELCHA);
					SendData(datasock, wpk);
					if (RPacket(wpk).ReadShort() == ERR_PT_KICKUSER) {
						Disconnect(datasock, 100, -25);
					}
				}
			}
		}
	} else {
		WPacket wpk = datasock->GetWPacket();
		wpk.WriteCmd(CMD_MC_DELCHA);
		wpk.WriteShort(ERR_MC_NETEXCP);
		SendData(datasock, wpk);
		Disconnect(datasock, 100, -25);
	}
}

void ToClient::CM_CREATE_PASSWORD2(DataSocket* datasock, RPacket& recvbuf) {
	uLong l_ulMilliseconds = 30 * 1000;
	uLong l_tick = GetTickCount() - recvbuf.GetTickCount();
	if (l_ulMilliseconds > l_tick) {
		l_ulMilliseconds = l_ulMilliseconds - l_tick;

		auto player = static_cast<Player*>(datasock->GetPointer());
		if (player) {
			std::lock_guard<std::mutex> l_lockStat(player->m_mtxstat);
			if (player->m_status != 1 || !player->gp_addr) {
				WPacket wpk = datasock->GetWPacket();
				wpk.WriteCmd(CMD_MC_DELCHA);
				wpk.WriteShort(ERR_MC_NOTSELCHA);
				SendData(datasock, wpk);
			} else {
				//调用GroupServer
				const char* pszPW = recvbuf.ReadString();

				WPacket wpk = WPacket(recvbuf).Duplicate();
				wpk.WriteCmd(CMD_TP_CREATE_PASSWORD2);
				wpk.WriteAddress(ToAddress(player));
				wpk.WriteAddress(player->gp_addr); //附带地址
				wpk = g_GateServer->toGroupServer->SyncCall(wpk);
				if (!wpk.HasData()) {
					wpk = datasock->GetWPacket();
					wpk.WriteCmd(CMD_MC_CREATE_PASSWORD2);
					wpk.WriteShort(ERR_MC_NETEXCP);
					SendData(datasock, wpk);
					Disconnect(datasock, 100, -25);
				} else {
					//返回Client
					wpk.WriteCmd(CMD_MC_CREATE_PASSWORD2);
					SendData(datasock, wpk);
					if (RPacket(wpk).ReadShort() == ERR_PT_KICKUSER) {
						Disconnect(datasock, 100, -25);
					}
				}
			}
		}
	} else {
		WPacket wpk = datasock->GetWPacket();
		wpk.WriteCmd(CMD_MC_CREATE_PASSWORD2);
		wpk.WriteShort(ERR_MC_NETEXCP);
		SendData(datasock, wpk);
		Disconnect(datasock, 100, -25);
	}
}

void ToClient::CM_UPDATE_PASSWORD2(DataSocket* datasock, RPacket& recvbuf) {
	uLong l_ulMilliseconds = 30 * 1000;
	uLong l_tick = GetTickCount() - recvbuf.GetTickCount();
	if (l_ulMilliseconds > l_tick) {
		l_ulMilliseconds = l_ulMilliseconds - l_tick;

		auto player = static_cast<Player*>(datasock->GetPointer());
		if (player) {
			std::lock_guard<std::mutex> l_lockStat(player->m_mtxstat);
			if (player->m_status != 1 || !player->gp_addr) {
				WPacket wpk = datasock->GetWPacket();
				wpk.WriteCmd(CMD_MC_DELCHA);
				wpk.WriteShort(ERR_MC_NOTSELCHA);
				SendData(datasock, wpk);
			} else {
				//调用GroupServer
				WPacket wpk = WPacket(recvbuf).Duplicate();
				wpk.WriteCmd(CMD_TP_UPDATE_PASSWORD2);
				wpk.WriteAddress(ToAddress(player));
				wpk.WriteAddress(player->gp_addr); //附带地址
				wpk = g_GateServer->toGroupServer->SyncCall(wpk);
				if (!wpk.HasData()) {
					wpk = datasock->GetWPacket();
					wpk.WriteCmd(CMD_MC_UPDATE_PASSWORD2);
					wpk.WriteShort(ERR_MC_NETEXCP);
					SendData(datasock, wpk);
					Disconnect(datasock, 100, -25);
				} else {
					//返回Client
					wpk.WriteCmd(CMD_MC_UPDATE_PASSWORD2);
					SendData(datasock, wpk);
					if (RPacket(wpk).ReadShort() == ERR_PT_KICKUSER) {
						Disconnect(datasock, 100, -25);
					}
				}
			}
		}
	} else {
		WPacket wpk = datasock->GetWPacket();
		wpk.WriteCmd(CMD_MC_UPDATE_PASSWORD2);
		wpk.WriteShort(ERR_MC_NETEXCP);
		SendData(datasock, wpk);
		Disconnect(datasock, 100, -25);
	}
}

void ToClient::OnEncrypt(DataSocket* datasock, char* ciphertext, const char* text, uLong& len) {
	TcpCommApp::OnEncrypt(datasock, ciphertext, text, len);

	if (_comm_enc > 0) { // 加密
		auto player = static_cast<Player*>(datasock->GetPointer());
		if (player && player->enc) {
			try {
#if NET_PROTOCOL_ENCRYPT
				encrypt_Noise(player->m_szSendKey, ciphertext, len);
#endif
				//encrypt_B(ciphertext, len, player->comm_textkey, player->comm_key_len);
			} catch (...) {
				LogLine l_line(g_gateerr);
				l_line << newln << "GameServer= [" << datasock->GetPeerIP() << "] ToClient::OnEncrypt Error!";
				l_line << endln;
			}
		}
	}
}

void ToClient::OnDecrypt(DataSocket* datasock, char* ciphertext, uLong& len) {
	TcpCommApp::OnDecrypt(datasock, ciphertext, len);

	if (_comm_enc > 0) { // 解密
		auto player = static_cast<Player*>(datasock->GetPointer());
		if (player && player->enc) {
			try {
				//encrypt_B(ciphertext, len, player->comm_textkey, player->comm_key_len, false);

#if NET_PROTOCOL_ENCRYPT
				decrypt_Noise(player->m_szRecvKey, ciphertext, len);
#endif
			} catch (...) {
				LogLine l_line(g_gateerr);
				l_line << newln << "GameServer= [" << datasock->GetPeerIP() << "] ToClient::OnDecrypt Error!";
				l_line << endln;
			}
		}
	}
}

void ToClient::post_mapcrash_msg(Player* player) {
	if (player->m_datasock == nullptr) {
		return;
	}
	WPacket pk = player->m_datasock->GetWPacket();
	pk.WriteCmd(CMD_MC_MAPCRASH);
	pk.WriteString(RES_STRING(GS_TOCLIENT_CPP_00031));
	SendData(player->m_datasock, pk);
}
