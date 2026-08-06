#include "StackTrace.h"
#include <windows.h>
#include <dbghelp.h>
#include <memory>

#pragma comment(lib, "dbghelp.lib")

String GetCurrentStackTrace(ULONG framesToSkip, ULONG framesToCapture) {
    HANDLE hProcess = GetCurrentProcess();

    // Initialize DbgHelp symbol engine
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    if (!SymInitialize(hProcess, NULL, TRUE)) {
        return L"Failed to initialize symbol handler.";
    }

    // Allocate frame pointer buffer
    std::unique_ptr<void*[]> backTrace(new void*[framesToCapture]);
    
    // Capture the stack back trace
    // Skip framesToSkip + 1 (skips GetCurrentStackTrace itself)
    USHORT captured = CaptureStackBackTrace(framesToSkip + 1, framesToCapture, backTrace.get(), NULL);

    // Buffer for symbol structure
    ULONG64 symbolBuffer[(sizeof(SYMBOL_INFO) + 256 * sizeof(char) + sizeof(ULONG64) - 1) / sizeof(ULONG64)];
    PSYMBOL_INFO pSymbol = reinterpret_cast<PSYMBOL_INFO>(symbolBuffer);
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = 255;

    IMAGEHLP_LINE64 line;
    ZeroMemory(&line, sizeof(IMAGEHLP_LINE64));
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    String traceResult = L"--- Stack Trace ---\n";

    for (USHORT i = 0; i < captured; ++i) {
        DWORD64 address = reinterpret_cast<DWORD64>(backTrace[i]);
        DWORD displacementLine = 0;
        DWORD64 displacementSym = 0;

        String lineInfo = L"";

        // Resolve Symbol Name (Function)
        if (SymFromAddr(hProcess, address, &displacementSym, pSymbol)) {
            lineInfo += String(pSymbol->Name);
        } else {
            lineInfo += L"[Unknown Symbol]";
        }

        // Resolve Source File and Line Number
        if (SymGetLineFromAddr64(hProcess, address, &displacementLine, &line)) {
            lineInfo += L" (" + String(line.FileName) + L":" + String(line.LineNumber) + L")";
        }

        // Append Address and Information
        traceResult += Format(L"[%2d] 0x%p : %s\n", ARRAYOFCONST((i, backTrace[i], lineInfo)));
    }

    SymCleanup(hProcess);
    return traceResult;
}

String FormatExceptionWithStackTrace(Exception *E, ULONG framesToSkip) {
    String msg = L"Exception: " + E->ClassName() + L"\n";
    msg += L"Message: " + E->Message + L"\n\n";
    msg += GetCurrentStackTrace(framesToSkip + 1);
    return msg;
}
