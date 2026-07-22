// main.cpp : Defines the entry point for the console application.
//

#include "gateserver.h"
_DBC_USING;

#include "ErrorHandler.h"

//#pragma init_seg( lib )
pi_LeakReporter pi_leakReporter("gatememleak.log");

//#include <ExceptionUtil.h>

int main(int argc, char* argv[]) {
	//SEHTranslator translator;

	ErrorHandler::Initialize();
	ErrorHandler::DisableErrorDialogs();

	CResourceBundleManage::Instance("GateServer.loc"); //Add by lark.li 20080130

	try {
		::SetLGDir("logfile/log");

		// Add by lark.li 20080731 begin
		pi_Memory m("logfile/log/memorymonitor.log");
		m.startMonitor(1);
		// End

		GateServerApp app;
		app.ServiceStart();
		g_GateServer->RunLoop();
		app.ServiceStop();

		// Add by lark.li 20080731 begin
		m.stopMonitor();
		m.wait();
		// End
	}
	T_FINAL

	return 0;
}
