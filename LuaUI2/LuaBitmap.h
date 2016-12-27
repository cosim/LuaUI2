// ����ļ���Ҫ����GdiPlus��Դ��lua�� ����λͼ,�����.
// ��lua���ٰ�װһ�� ���������ñ���������������ƻ��� ��Դ����

#pragma once

#include "LuaObject.h"

class LuaBitmap : public LuaObject
{
public:
	LuaBitmap();
	~LuaBitmap();

	static LuaBitmap* FromFile(LPCWSTR path);

	Gdiplus::Bitmap * Get() { return m_bmp; }
	
	static int GetSize(lua_State *L);
	static int GetFourStateInfo(lua_State *L);
	static int GetNineInOneInfo(lua_State *L);

	RTTI_DECLARATIONS(LuaBitmap, LuaObject)

	BEGIN_LUA_METHOD_MAP(LuaBitmap)
		LUA_METHOD_ENTRY(GetSize)
		LUA_METHOD_ENTRY(GetFourStateInfo)
		LUA_METHOD_ENTRY(GetNineInOneInfo)
	END_LUA_METHOD_MAP()

private:
	Gdiplus::Bitmap *m_bmp;
};