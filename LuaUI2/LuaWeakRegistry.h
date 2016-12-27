/*
** Copyright (C) 2015-2016 Chen Shi. See Copyright Notice in LuaUI2.h
*/
#pragma once

class LuaWeakRegistry
{
public:
	LuaWeakRegistry(void);
	~LuaWeakRegistry(void);

	// ��ʼ�� ֻ�ܵ���һ��
	static void Create(lua_State *L);

	// ��ջ��Ԫ�ط��������ñ� ������ջ��Ԫ��
	static void Put(lua_State *L, void *key);

	// ����key�õ�lua���� ջ��+1
	static void Get(lua_State *L, void *key);

private:
	static bool m_bInit;
};
