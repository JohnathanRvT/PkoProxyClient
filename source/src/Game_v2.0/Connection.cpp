#include "StdAfx.h"
#pragma warning(disable : 4018)
#include "Connection.h"
#include "PacketCmd.h"
#pragma warning(default : 4018)

_DBC_USING

bool Connection::Connect(dbc::cChar* hostname, dbc::uShort port, dbc::uLong timeout) {
	LG("connect", "Connect: %s, %d\n", hostname, port);

	if (m_datasock) {
		return false;
	}
	if ((m_status == Connection_Status::CONNECTING) || (m_status == Connection_Status::CONNECTED)) {
		return false;
	}
	m_timeout = max(timeout, 1000);
	if (m_timeout) {
		m_tick = GetTickCount();
	} else {
		m_tick = 0;
	}
	m_status = Connection_Status::CONNECTING;
	strncpy_s(m_hostname, sizeof(m_hostname), hostname, _TRUNCATE);
	m_port = port;
	if (m_netif->GetProcessor()) {
		m_netif->GetProcessor()->AddTask(this);
	} else {
		m_netif->GetCommunicator()->AddTask(this);
	}
	return true;
}
void Connection::Disconnect(int reason) {
	LG("connect", "Disconnect\n");

	m_netif->Disconnect(m_datasock, 0, reason);
}
void Connection::OnDisconnect() {
	m_datasock = nullptr;
	m_status = Connection_Status::INVALID;
}

long Connection::Process() {
	if (m_timeout) {
		LG("connect", RES_STRING(CL_LANGUAGE_MATCH_34), m_hostname);
		m_datasock = m_netif->Connect(m_hostname, m_port, &m_sock);
	} else {
		LG("connect", RES_STRING(CL_LANGUAGE_MATCH_34), m_hostname);
		m_datasock = m_netif->Connect(m_hostname, m_port);
	}
	MutexArmor l_lock(m_mtx);
	if (m_datasock) {
		if (atomic_exchange(&m_status, Connection_Status::CONNECTING) == Connection_Status::TIMEOUT) {
			m_status = Connection_Status::TIMEOUT;
			if (m_datasock) {
				m_datasock->Free();
				m_datasock = nullptr;
			}
		}
	} else if (atomic_exchange(&m_status, Connection_Status::FAILURE) == Connection_Status::TIMEOUT) {
		m_status = Connection_Status::TIMEOUT;
	}
	return 0;
}
void Connection::CHAPSTR() {
	LONG l_stat;
	MutexArmor l_lock(m_mtx);
	if ((l_stat = atomic_exchange(&m_status, Connection_Status::CONNECTED)) == Connection_Status::TIMEOUT) {
		m_status = Connection_Status::TIMEOUT;
		if (m_datasock) {
			m_datasock->Free();
			m_datasock = nullptr;
		}
	} else if (l_stat == Connection_Status::FAILURE) {
		l_stat = Connection_Status::FAILURE;
		if (m_datasock) {
			m_datasock->Free();
			m_datasock = nullptr;
		}
	}
}

int Connection::GetConnStat() {
	if (m_status == Connection_Status::CONNECTING && m_timeout && (GetTickCount() - m_tick) > m_timeout) {
		MutexArmor l_lock(m_mtx);
		if (atomic_exchange(&m_status, Connection_Status::TIMEOUT) == Connection_Status::CONNECTED) {
			m_status = Connection_Status::CONNECTED;
		} else if (m_datasock) {
			m_datasock->Free();
			m_datasock = nullptr;
		} else {
			closesocket(m_sock);
		}
	}

	return m_status;
}

/*
	//xuedong 2004.08.18
	cChar chAppendName[512] = {0};

	if (cbNameAppendInfo)
	{
		cChar chIp[16] = {0}, chPort[16] = {0}, chHostName[64] = {0};

		gethostname((char *)chHostName, 64);
		_snprintf_s((char *)chIp, _TRUNCATE, "%s", m_datasock->GetLocalIP());
		_snprintf_s((char *)chPort, _TRUNCATE, "%d", m_datasock->GetLocalPort());
		strcat((char *)chAppendName, chHostName);
		strcat((char *)chAppendName, ".");
		strcat((char *)chAppendName, (char *)chPort);
	}
	//end

	WPacket pk	=g_NetIF->GetWPacket();;
	pk.WriteCmd(CMD_CM_SELROLE);			//ÃüÁî
	pk.WriteLong(m_cat);
	pk.WriteSequence(m_name, uShort(strlen(m_name) + 1));
	if (cbNameAppendInfo)
		pk.WriteSequence(chAppendName, uShort(strlen(chAppendName) + 1));
	m_nt->SendData(m_datasock,pk);
	return true;
*/
