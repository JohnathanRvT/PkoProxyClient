/* -------------------------------------------------------------------------- *
   WinUnit - Maria Blees (maria.blees@microsoft.com)
 * -------------------------------------------------------------------------- */

/**
 *  @file ErrorHandler.h
 *  The header file for the application-wide error handling functions used by
 *  WinUnit.exe.
 */

#pragma once

#include <windows.h>
#include <string>

/// This class contains static functions used for system-wide error handling.
class ErrorHandler {
	using SignalHandlerPointer = void (*)(int);

public:
	// Sets up application-wide exception handling.
	static void Initialize();

	// Sets process- and CRT-wide variables that disable dialogs for several
	// classes of errors and asserts.
	static void DisableErrorDialogs();

	static void SetCurrent(const char* current);

private:
	/// This is for use by the unhandled exception filter--if false, the filter
	/// allows the crash dialog to go up after printing an error message.
	static bool s_nonInteractive;

	// This is the function that gets called when an unhandled exception
	// bubbles up to the top.
	static LONG WINAPI UnhandledExceptionFilter(
		EXCEPTION_POINTERS* pExceptionPointers);

	static void WINAPI InvalidParameterHandler(const wchar_t* expression,
															 const wchar_t* function,
															 const wchar_t* file,
															 unsigned int line,
															 uintptr_t pReserved);

	// The function that replaces terminate().
	static void TerminateFunction();

	// The function that is called when abort() is called.
	static void AbortFunction(int /* signal */);

	// Called by the other error handlers to display the error message.
	static void DisplayError(const wchar_t* errorMessage,
							 const wchar_t* details = L"");

	static std::string _current;

private:
	~ErrorHandler();
};
