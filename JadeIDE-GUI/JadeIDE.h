#pragma once

#include "resource.h"
#include "wndproc.h"
#include "structures.h"
#include <CommCtrl.h>
#include "treeview.h"
#include "redit.h"
#include <shlwapi.h>
#include "handlers.h"
#include "build.h"

#define MAINCLASS L".mainclass"
#define SRCPACKAGE L".sourcepackage"
#define JDKPATH L".jdkpath"

#define SOURCE_COLLECT_COMMAND "dir /s /B *.java > sources.txt"

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);