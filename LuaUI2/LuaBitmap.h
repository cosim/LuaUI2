// ����ļ���Ҫ����GdiPlus��Դ��lua�� ����λͼ,�����.
// ��lua���ٰ�װһ�� ���������ñ���������������ƻ��� ��Դ����

#pragma once

#include "LuaObject.h"

namespace cs {

class Bitmap;

class BitmapLua : public LuaObject
{
public:
	BitmapLua(Bitmap *bmp);
	~BitmapLua();

    static int LoadFromFile(lua_State *L);
	static int GetSize(lua_State *L);
	static int GetFourStateInfo(lua_State *L);
	static int GetNineInOneInfo(lua_State *L);

	BEGIN_LUA_METHOD_MAP(Bitmap)
        LUA_METHOD_ENTRY(LoadFromFile)
		LUA_METHOD_ENTRY(GetSize)
		LUA_METHOD_ENTRY(GetFourStateInfo)
		LUA_METHOD_ENTRY(GetNineInOneInfo)
	END_LUA_METHOD_MAP()

    virtual Object * GetCppSide() override;

private:
    Bitmap *m_bmp;
    DISALLOW_COPY_AND_ASSIGN(BitmapLua);
};

} // namespace cs
