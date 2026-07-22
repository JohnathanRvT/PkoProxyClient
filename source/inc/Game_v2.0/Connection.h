#ifndef CONNECTION_H
#define CONNECTION_H

#include "ThreadPool.h"
#include "DataSocket.h"

class NetIF;
#pragma pack(push)
#pragma pack(4)
class Connection : public dbc::Task {
public:
	enum Connection_Status {
		INVALID = 0,
		CONNECTING = 1,
		FAILURE = 2,
		CONNECTED = 3,
		TIMEOUT = 4
	};
	Connection(NetIF* netif) : m_netif(netif) {}
	void Clear() {
		m_status = Connection_Status::INVALID;
		m_datasock = nullptr;
	}
	bool Connect(dbc::cChar* hostname, dbc::uShort port, dbc::uLong timeout = 0); //ΊΑΓλ
	void Disconnect(int reason);
	void OnDisconnect();
	bool IsConnected() { return m_status == Connection_Status::CONNECTED; }
	int GetConnStat();
	void CHAPSTR();

	dbc::DataSocket* GetDatasock() { return m_datasock; }

	long Process() override;
	Task* Lastly() override { return nullptr; }

#ifdef _TEST_CLIENT
	void SwitchSocket(dbc::DataSocket* pSock, int status) {
		m_datasock = pSock;
		m_status = status;
	}
#endif

private:
	dbc::Mutex m_mtx;
	NetIF* const m_netif;
	std::atomic<long> m_status{Connection_Status::INVALID};
	dbc::DataSocket* volatile m_datasock{nullptr};

	dbc::uLong m_timeout;
	dbc::uLong m_tick;
	SOCKET m_sock;
	char m_hostname[129];
	dbc::uShort m_port;
};
#pragma pack(pop)

#endif //CONNECTION_H
