// ����ļ���Ҫ����GdiPlus��Դ��lua�� ����λͼ,�����.
// ��lua���ٰ�װһ�� ���������ñ���������������ƻ��� ��Դ����

#pragma once

#include "LuaObject.h"
#include "LBitmap.h"

namespace cs {

class BitmapLua : public virtual LBitmap, public virtual LuaObject
{
public:
	BitmapLua();
	~BitmapLua();

    static int LoadFromFile(lua_State *L);
	static int GetSize(lua_State *L);
	static int GetFourStateInfo(lua_State *L);
	static int GetNineInOneInfo(lua_State *L);

	BEGIN_LUA_METHOD_MAP(BitmapLua)
        LUA_METHOD_ENTRY(LoadFromFile)
		LUA_METHOD_ENTRY(GetSize)
		LUA_METHOD_ENTRY(GetFourStateInfo)
		LUA_METHOD_ENTRY(GetNineInOneInfo)
	END_LUA_METHOD_MAP()

};

} // namespace cs
