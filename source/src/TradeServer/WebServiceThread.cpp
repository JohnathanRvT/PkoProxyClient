#include "infotype.h"
#include "webservicethread.h"

#include "webservice.h"
using namespace Moliyo;

WebServiceThread::WebServiceThread(void) {
}

WebServiceThread::~WebServiceThread(void) {
}

uint WebServiceThread::Run() {
	m_state = THREAD_INIT; //  初始化状态
	unsigned int threadID = GetCurrentThreadId();
	LGPRT("System", "Begin Web Service Thread %li" __LN, threadID);

	RDBilling ^ service = gcnew RDBilling();
	service->Url = gcnew String(g_servInfo.webHttp);

	service->AuthHeaderValue = gcnew AuthHeader();
	service->AuthHeaderValue->UserName = gcnew String(g_servInfo.webUserName);
	service->AuthHeaderValue->PassWord = gcnew String(g_servInfo.webPassWord);

	pWebServiceParam webParam;
	for (;;) {
		__BEGIN_TRY
		m_state = THREAD_RESTING; //  闲置状态
		if (INFO_STOPING == g_servInfo.serv_state) {
			m_state = THREAD_STOPPED; //  停止状态
			break;
		}
		if (0 == g_listInfo.web_list.Size()) //  队列为空时
		{
			Sleep(1); //  释放cpu资源
			continue;
		}
		m_state = THREAD_WORKING; //  工作状态
		while (g_listInfo.web_list.Size()) {
			webParam = g_listInfo.web_list.Pop();
			if (webParam) {
				int ret = 0;

				try {
					//
					//webParam->MemberPK = "dnking";
					//webParam->TranID = "sadf12318933";
					//webParam->Credit = 345;
					//webParam->RecTime = "2008-08-20 10:00:00";
					//webParam->Memo = "register vip";
					char buffer[20];
					memset(buffer, 0, sizeof(buffer));
					time_t t = time(0);
					tm* nowTm = localtime(&t);
					sprintf(buffer, "%04i-%02i-%02i %02i:%02i:%02i",
							(nowTm->tm_year + 1900), (nowTm->tm_mon + 1), nowTm->tm_mday, nowTm->tm_hour, nowTm->tm_min, nowTm->tm_sec);

					webParam->RecTime = buffer;

					LGPRT("webservice", "PSS_RDMemberTDRIns: MemberPK = %s TranID = %s Credit = %d, RecTime = %s Memo = %s type = %d typesub = %d " __LN, webParam->MemberPK.c_str(), webParam->TranID.c_str(), webParam->Credit,
						  webParam->RecTime.c_str(), webParam->Memo.c_str(), g_servInfo.type, g_servInfo.typesub);

					ret = service->PSS_RDMemberTDRIns(gcnew String(webParam->MemberPK.c_str()), gcnew String(webParam->TranID.c_str()), webParam->Credit,
													  gcnew String(webParam->RecTime.c_str()), gcnew String(webParam->Memo.c_str()), g_servInfo.type, g_servInfo.typesub);
				} catch (...) {
					ret = -999;
				}

				if (ret != 0) {
					LGPRT("webservice", "Error PSS_RDMemberTDRIns: MemberPK = %s TranID = %s " __LN, webParam->MemberPK.c_str(), webParam->TranID.c_str());
				}

				SAFE_DELETE(webParam);
			}
		}
		__END_TRY
	}

	LGPRT("System", "End Web Service Thread %li" __LN, threadID);
	return 0;
}
