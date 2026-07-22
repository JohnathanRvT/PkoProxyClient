#include "gateserver.h"
dbc::cuShort g_version = 103;

long ConnectGroupServer::Process() {
	try {
		_toGroupServer->m_calltotal++;

		while (!GetExitFlag() && !_toGroupServer->m_atexit) {
			if (_toGroupServer->_connected) {
				if (_toGroupServer->IsSync() && g_GateServer) {
					RunChainGetArmor<Player> l(g_GateServer->m_playerlist);

					WPacket pk = _toGroupServer->GetWPacket();
					pk.WriteCmd(CMD_TP_SYNC_PLYLST);

					int player_count = g_GateServer->m_playerlist.GetTotal();
					pk.WriteLong(player_count);
					pk.WriteString(_toGroupServer->_myself.c_str()); //"Gateserver name"

					auto ply_array = std::make_unique<Player*[]>(player_count);

					int index = 0;
					for (Player* player = g_GateServer->m_playerlist.GetNextItem(); player != nullptr; player = g_GateServer->m_playerlist.GetNextItem()) {
						pk.WriteAddress(ToAddress(player)); // "Attach the memory address on the GateServer"
						pk.WriteLong(player->m_loginID);
						pk.WriteLong(player->m_actid);

						ply_array[index++] = player;
					}

					RPacket retpk = _toGroupServer->SyncCall(pk);
					int err = retpk.ReadShort();

					if (!retpk.HasData() || err == ERR_PT_LOGFAIL) {
						Sleep(5000);
						_toGroupServer->Disconnect();
						continue;
					} else {
						int num = retpk.ReadShort();
						if (num != player_count) {
							Sleep(5000);
							_toGroupServer->Disconnect();
						}

						for (int i = 0; i < num; i++) {
							if (retpk.ReadShort() == 1) {
								ply_array[i]->gp_addr = retpk.ReadAddress();
							}
						}
						_toGroupServer->SetSync(false);
					}
				}

				// "Already connected"
				//NOTE: This is not necessarily true; We might have disconnected
				Sleep(1000);
			} else {
				LogLine l_line(g_gateconnect);

				//"Not connected or connected broken, reconnect!"
				l_line << newln << RES_STRING(GS_TOGROUPSERVER_CPP_00001) << endln;
				DataSocket* datasock = _toGroupServer->Connect();
				if (datasock == nullptr) { // Connection failed, retry in 5 seconds
					l_line << newln << RES_STRING(GS_TOGROUPSERVER_CPP_00002) << endln;
					Sleep(5000);
					continue;
				}

				// Connection established, try to login
				WPacket pk = _toGroupServer->GetWPacket();
				pk.WriteCmd(CMD_TP_LOGIN);
				pk.WriteShort(g_version);
				pk.WriteString(_toGroupServer->_myself.c_str());

				RPacket retpk = _toGroupServer->SyncCall(pk);
				int err = retpk.ReadShort();
				if (!retpk.HasData() || err == ERR_PT_LOGFAIL) { // Login failed, retry in 5 seconds
					l_line << newln << RES_STRING(GS_TOGROUPSERVER_CPP_00003) << endln;
					_toGroupServer->Disconnect();
					Sleep(5000);
					continue;
				}

				// Connection established & successful login
				l_line << newln << RES_STRING(GS_TOGROUPSERVER_CPP_00004) << endln;
				_toGroupServer->_gs.datasock = datasock;
				_toGroupServer->_connected = true;

				// Add by lark.li 20081119 begin
				_toGroupServer->SetSync();
				// End
			}
		}
	}
	T_FINAL

	return 0;
}

Task* ConnectGroupServer::Lastly() {
	--(_toGroupServer->m_calltotal);
	return Task::Lastly();
}

ToGroupServer::ToGroupServer(char const* fname, ThreadPool* proc, ThreadPool* comm)
	: TcpClientApp(this, proc, comm), RPCMGR(this) {
	IniFile inf(fname);
	IniSection& is = inf["GroupServer"];
	_myself = inf["Main"]["Name"];
	_gs.ip = is["IP"];
	_gs.port = atoi(is["Port"]);

	// 启动 PING 线程

	SetPKParse(0, 2, 64 * 1024, 400);
	BeginWork(atoi(is["EnablePing"]));

	//++m_calltotal;
	//proc->AddTask(new ConnectGroupServer(this));
}

ToGroupServer::~ToGroupServer() {
	m_atexit = 1;
	while (m_calltotal) {
		Sleep(1);
	}
	ShutDown(12 * 1000);
}

DataSocket* ToGroupServer::Connect() {
	return _gs.datasock = TcpClientApp::Connect(_gs.ip.c_str(), _gs.port);
}
void ToGroupServer::Disconnect(int reason, uLong remain) {
	TcpClientApp::Disconnect(_gs.datasock, remain, reason);
}
RPacket ToGroupServer::SyncCall(WPacket wpk, uLong milliseconds) {
	return RPCMGR::SyncCall(_gs.datasock, wpk, milliseconds);
}

bool ToGroupServer::OnConnect(DataSocket* datasock) //返回值:true-允许连接,false-不允许连接
{
	datasock->SetRecvBuf(64 * 1024);
	datasock->SetSendBuf(64 * 1024);
	LogLine l_line(g_gateconnect);
	l_line << newln << "connect GroupServer: " << datasock->GetPeerIP() << ",Socket num:" << GetSockTotal() + 1;
	return true;
}

void ToGroupServer::OnDisconnect(DataSocket* datasock, int reason) //reason值:0-本地程序正常退出；-3-网络被对方关闭；-1-Socket错误;-5-包长度超过限制。
{																   // 激活 ConnnectGroupServer 线程
	LogLine l_line(g_gateconnect);
	l_line << newln << "disconnection with GroupServer,Socket num: " << GetSockTotal() << ",reason =" << GetDisconnectErrText(reason).c_str() << ", reconnecting..." << endln;

	// Add by lark.li 20081210 begin
	RunChainGetArmor<Player> l(g_GateServer->m_playerlist);
	for (Player* player = g_GateServer->m_playerlist.GetNextItem(); player; player = g_GateServer->m_playerlist.GetNextItem()) {
		if (player) {
			if (player->m_status != 2) {
				g_GateServer->toClient->Disconnect(player->m_datasock, 100, -29);
			}
		}
	}
	// End

	if (!g_appexit) {
		_connected = false;
	}
}

WPacket ToGroupServer::OnServeCall(DataSocket* datasock, RPacket& in_para) {
	uShort l_cmd = in_para.ReadCmd();
	WPacket retpk = GetWPacket();

	switch (l_cmd) {
	case 0:
	default:
		break;
	}

	return retpk;
}

void ToGroupServer::OnProcessData(DataSocket* datasock, RPacket& recvbuf) {
	uShort l_cmd = recvbuf.ReadCmd();
	//LG("ToGroupServer", "-->l_cmd = %d\n", l_cmd);
	try {
		switch (l_cmd) {
		case CMD_PM_TEAM: {
			for (GameServer* l_game = g_GateServer->toGameServer->_game_list; l_game; l_game = l_game->next) {
				g_GateServer->toGameServer->SendData(l_game->m_datasock, recvbuf);
			}
			break;
		}
		case CMD_AP_KICKUSER:
		case CMD_PT_KICKUSER: {
			const uShort l_aimnum = recvbuf.ReverseReadShort();
			auto player = ToPointer<Player>(recvbuf.ReverseReadAddress());
			if (player && player->gp_addr == recvbuf.ReverseReadAddress()) {
				LogLine l_line(g_gatelog);
				l_line << newln << "GroupServer kill person,player->m_dbid =" << player->m_dbid << endln;
				g_GateServer->toClient->Disconnect(player->m_datasock, 0, -21);
			} else if (player) {
				LogLine l_line(g_gatelog);
				l_line << newln << "GroupServer kick person, but can't kick person,player->m_dbid =" << player->m_dbid << endln;
			}
			break;
		}
		case CMD_PT_DEL_ESTOPUSER: {
			//printf( "CMD_PT_DEL_ESTOPUSER" );
			uShort l_aimnum = recvbuf.ReverseReadShort();
			auto player = ToPointer<Player>(recvbuf.ReverseReadAddress());
			if (player && player->gp_addr == recvbuf.ReverseReadAddress()) {
				LogLine l_line(g_gatelog);
				//l_line<<newln<<"收到GroupServer的解除禁言包，操作成功,player->m_dbid ="<<player->m_dbid<<endln;
				l_line << newln << "GroupServer del estop user,operator success,player->m_dbid =" << player->m_dbid << endln;
				player->m_estop = false;
			} else if (player) {
				LogLine l_line(g_gatelog);
				//l_line<<newln<<"收到GroupServer的解除禁言包但没解掉,player->m_dbid ="<<player->m_dbid<<endln;
				l_line << newln << "GroupServer del estop user, but can't operator success,player->m_dbid =" << player->m_dbid << endln;
			}
		} break;
		case CMD_PT_ESTOPUSER: {
			//printf( "CMD_PT_ESTOPUSER" );
			uShort l_aimnum = recvbuf.ReverseReadShort();
			auto player = ToPointer<Player>(recvbuf.ReverseReadAddress());
			if (player && player->gp_addr == recvbuf.ReverseReadAddress()) {
				LogLine l_line(g_gatelog);
				//l_line<<newln<<"收到GroupServer的禁言包，操作成功,player->m_dbid ="<<player->m_dbid<<endln;
				l_line << newln << "GroupServer del estop user,operator success,player->m_dbid =" << player->m_dbid << endln;
				player->m_estop = true;
			} else if (player) {
				LogLine l_line(g_gatelog);
				//l_line<<newln<<"收到GroupServer的禁言包但没禁掉,player->m_dbid ="<<player->m_dbid<<endln;
				l_line << newln << "GroupServer del estop user, but can't operator success,player->m_dbid =" << player->m_dbid << endln;
			}
		} break;
		case CMD_MC_SYSINFO:
			l_cmd = CMD_PC_BASE;
		default: //缺省转发
		{
			if (l_cmd / 500 == CMD_PC_BASE / 500) {
				RPacket rpk = recvbuf;
				const uShort l_aimnum = rpk.ReverseReadShort();
				recvbuf.DiscardLast(sizeof(uintptr_t) * 2 * l_aimnum + sizeof(uShort));
				Player* player = nullptr;
				for (uShort i = 0; i < l_aimnum; i++) {
					player = ToPointer<Player>(rpk.ReverseReadAddress());
					if (player->gp_addr == rpk.ReverseReadAddress()) {
						g_GateServer->toClient->SendData(player->m_datasock, recvbuf);
					} else {
						player = nullptr;
					}
				}
				if (l_cmd == CMD_PC_CHANGE_PERSONINFO && player) {
					WPacket wpk = recvbuf;
					wpk.WriteCmd(CMD_TM_CHANGE_PERSONINFO);
					wpk.WriteAddress(ToAddress(player));
					wpk.WriteAddress(player->gm_addr); //附加上在GameServer上的内存地址
					g_GateServer->toGameServer->SendData(player->game->m_datasock, wpk);
					break;
				}
				if (l_cmd == CMD_PC_PING && player) {
					player->m_pingtime = GetTickCount();
					break;
				}
			} else if (l_cmd / 500 == CMD_PM_BASE / 500) {
				RPacket rpk = recvbuf;
				const uShort l_aimnum = rpk.ReverseReadShort();
				recvbuf.DiscardLast(sizeof(uintptr_t) * 2 * l_aimnum + sizeof(uShort));
				if (!l_aimnum) {
					WPacket wpk = WPacket(recvbuf).Duplicate();
					wpk.WriteLong(0);
					for (GameServer* l_game = g_GateServer->toGameServer->_game_list; l_game; l_game = l_game->next) {
						g_GateServer->toGameServer->SendData(l_game->m_datasock, wpk);
					}
				} else {
					WPacket wpk, wpk0 = WPacket(recvbuf).Duplicate();
					for (uShort i = 0; i < l_aimnum; i++) {
						auto player = ToPointer<Player>(rpk.ReverseReadAddress());
						if (player->gp_addr == rpk.ReverseReadAddress() && player->game) {
							wpk = wpk0;
							wpk.WriteAddress(ToAddress(player));
							wpk.WriteAddress(player->gm_addr);
							g_GateServer->toGameServer->SendData(player->game->m_datasock, wpk);
						}
					}
				}
			}
			break;
		}
		}
	} catch (...) {
		LG("ToGroupServerError", "l_cmd = %d\n", l_cmd);
	}
	//LG("ToGroupServer", "<--l_cmd = %d\n", l_cmd);
}

// 从 GroupServer 上得到所有用户列表
RPacket ToGroupServer::get_playerlist() {
	WPacket pk = GetWPacket();
	pk.WriteCmd(CMD_TP_REQPLYLST);

	return SyncCall(pk);
}
