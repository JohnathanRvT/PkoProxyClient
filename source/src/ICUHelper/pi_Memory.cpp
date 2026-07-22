#include "pi_Memory.h"

pi_Memory::pi_Memory(const char* fileName) : m_bStop(false) {
	m_szfileName = fileName;
}

pi_Memory::~pi_Memory() {
}

bool pi_Memory::startMonitor(int minuteInterval, DWORD processID) {
	this->m_minuteInterval = minuteInterval;
	this->m_processID = processID;

	m_outputFile.open(m_szfileName.c_str(), std::ios::out);
	m_outputFile << "time,memory size,virtual memory size,physical available size" << std::endl;

	m_hThread = CreateThread(nullptr, 0, ThreadProc, this, 0, &m_dwThreadId);
	return true;
}

bool pi_Memory::stopMonitor() {
	m_bStop = true;

	return true;
}

bool pi_Memory::wait() {
	DWORD exitCode;

	if (GetExitCodeThread(m_hThread, &exitCode)) {
		if (exitCode == STILL_ACTIVE) {
			ResumeThread(m_hThread);
			if (TerminateThread(m_hThread, 1)) {
				printf_s("TerminateThread OK!");
			} else {
				DWORD error = GetLastError();
				printf_s("TerminateThread Error(%d)!", error);
			}
		}
	}

	WaitForSingleObject(m_hThread, INFINITE);
	CloseHandle(m_hThread);

	if (m_outputFile.is_open())
		m_outputFile.close();

	return true;
}

DWORD WINAPI pi_Memory::ThreadProc(LPVOID lpParam) {
	pi_Memory* pThis = (pi_Memory*)lpParam;

	while (!pThis->m_bStop) {
		HANDLE hProcess;
		PERFORMANCE_INFORMATION pi;
		PROCESS_MEMORY_COUNTERS pmc;

		try {
			// Print information about the memory usage of the process.
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pThis->m_processID);
			if (nullptr == hProcess)
				continue;

			if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)) && GetPerformanceInfo(&pi, sizeof(PERFORMANCE_INFORMATION))) {
				SYSTEMTIME st;
				char time[15] = {0};
				GetLocalTime(&st);
				_snprintf_s(time, sizeof(time), _TRUNCATE, "%02d-%02d %02d:%02d:%02d", st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

				pThis->m_outputFile << time << "," << pmc.WorkingSetSize / (1024.0 * 1024.0) << "," << pmc.PagefileUsage / (1024.0 * 1024.0) << ","
									<< pi.PhysicalAvailable * pi.PageSize / (1024.0 * 1024.0) << std::endl;
			}

			CloseHandle(hProcess);
		} catch (...) {
		}

		::Sleep(pThis->m_minuteInterval * 60 * 1000);
	}

	return 0;
}
