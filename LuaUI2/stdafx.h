/*
** Copyright (C) 2015-2016 Chen Shi. See Copyright Notice in LuaUI2.h
*/

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <stack>

using std::wstring;
using std::wstringstream;
using std::stringstream;

#include <lua/lua.hpp>

#include <new> // �Ҳ� û��� placement new �Ͳ��ܱ���
#include <cassert>
#include <atlstr.h>

#include <GdiPlus.h>
#include <ShlObj.h>
//#include <vld.h> //���������ZeroBrane�ͳ����� ����������

