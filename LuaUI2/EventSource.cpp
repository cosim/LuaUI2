/*
** Copyright (C) 2015-2016 Chen Shi. See Copyright Notice in LuaUI2.h
*/


#include "StdAfx.h"
#include "EventSource.h"

// ��Щ����ʵ����Ӧ�÷ŵ� LuaObject����
// ���˺ܶ� ����������ǲ����Ҵ���
// ������ڶദAttatchListener����Ͳ����ҳ�ȫ����Listener ��Ϊlua��û��"������������"��...
// 

EventSource::EventSource(lua_State *L) :
	m_L (L)
{
}

EventSource::~EventSource(void)
{
	for (EventMapType::iterator iter = m_mapEvents.begin(); iter != m_mapEvents.end(); iter ++)
	{
		for (std::list<int>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2 ++)
		{
			luaL_unref(m_L, LUA_REGISTRYINDEX, *iter2);
		}
	}
}

void EventSource::RegisterEvent(const char* name)
{
	std::string strName(name);
	if (m_mapEvents.find(strName) == m_mapEvents.end())
	{
		m_mapEvents[strName] = std::list<int>();
	}
}

void EventSource::FireEvent( lua_State *L, const char* name, int nargs)
{
	EventMapType::iterator iter = m_mapEvents.find(std::string(name));
	if (iter != m_mapEvents.end())
	{
		for (std::list<int>::iterator iter2 = iter->second.begin();
			iter2 != iter->second.end(); iter2 ++)
		{
			// PushToLua
			int ref = (*iter2);
			lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
			// FIXME ���ﲻ�� call�˾����ջ�� 
			lua_call(L, nargs, 1); // TODO ��취��sender������
			if (lua_isstring(L, -1))
			{
				const char *szRet = lua_tostring(L, -1);
				// ��ֹ����
				if (strcmp(szRet, "abort") == 0)
				{
					break;
				}
			}
		}
	}
}

void EventSource::AttachListener( lua_State *L, const char* name, int idx, bool bFront)
{
	if (!lua_isfunction(L, idx))
	{
		luaL_error(L, "AttachListener need a function.");
	}
	EventMapType::iterator iter = m_mapEvents.find(std::string(name));
	if (iter != m_mapEvents.end())
	{
		lua_pushvalue(L, idx);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		std::list<int>::iterator iter2;
		if (bFront)
		{
			iter2 = iter->second.begin();
		}
		else
		{
			iter2 = iter->second.end();
		}
		iter->second.insert(iter2, ref);
	}
}
