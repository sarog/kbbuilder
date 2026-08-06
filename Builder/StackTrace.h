#ifndef BUILDKB_STACKTRACE_H
#define BUILDKB_STACKTRACE_H

#include <vcl.h>

// Captures current execution stack as a formatted VCL String
String GetCurrentStackTrace(ULONG framesToSkip = 0, ULONG framesToCapture = 32);

// Formats a VCL Exception with its stack trace
String FormatExceptionWithStackTrace(Exception* E, ULONG framesToSkip = 0);

#endif
