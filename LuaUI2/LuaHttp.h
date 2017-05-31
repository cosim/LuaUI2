/*
** Copyright (C) 2015-2016 Chen Shi. See Copyright Notice in LuaUI2.h
*/
#pragma once

#include "LuaObject.h"
#include "..\include\curl\curl.h"

namespace cs {

class LuaRefObject
{
public:
	LuaRefObject(lua_State *L) :
		m_ref(LUA_NOREF),
		m_L(L)
	{
	}
	~LuaRefObject() 
	{
		Unref();
	}

	// ����һ��lua����  �����ջû��Ӱ��
	void Ref(int idx)
	{
		lua_pushvalue(m_L, idx);
		luaL_ref(m_L, LUA_REGISTRYINDEX);
	}
	void Unref()
	{
		if (m_ref != LUA_NOREF)
		{
			luaL_unref(m_L, LUA_REGISTRYINDEX, m_ref);
			m_ref = LUA_NOREF;
		}
	}
	// �����õĶ���ѹջ luaջ+1
	void PushToLua()
	{
		assert(m_ref != LUA_NOREF);
		lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_ref);
	}
	
private:
	lua_State *m_L;
	int m_ref;
};

class LuaHttp : public LuaObject
{
public:
	LuaHttp(void);
	virtual ~LuaHttp(void);

	static int Get(lua_State *L);
	static int Post(lua_State *L);
	static int DownloadToFile(lua_State *L);

	BEGIN_LUA_METHOD_MAP(LuaHttp)
		LUA_METHOD_ENTRY(Get)
		LUA_METHOD_ENTRY(Post)
		LUA_METHOD_ENTRY(DownloadToFile)
	END_LUA_METHOD_MAP()
private:
	// TODO ������Բ�����ô���� easy handle�������̼߳䴫��
	// You can pass the handles around among threads,
	// but you must never use a single handle from more than one thread at any given time.
	struct HttpParam 
	{
		HttpParam() :
			curl(NULL),
			cbFinish(LUA_NOREF), 
			cbProgress(LUA_NOREF),
			L(NULL), 
			hwnd(NULL), 
			hFile(INVALID_HANDLE_VALUE),
			win32Error(ERROR_SUCCESS),
			curlError(CURLE_OK),
			responseCode(-1)
			{} // ������Ͳ����Զ����ɿ���������
		~HttpParam() {}
		std::string url;
		std::string data;  // ��������
		std::string file;  // �����ļ�·��
		std::string post;  // post ��
		std::vector<std::string> headers;
		CURL *curl;
		lua_State *L;
		int cbFinish; // lua�ص�
		int cbProgress;
		HWND hwnd; // ���֪ͨ
		HANDLE hFile; // �����ļ����
		DWORD win32Error;
		CURLcode curlError;
		long responseCode;
	private:
		DISALLOW_COPY_AND_ASSIGN(HttpParam);
	};
	struct ProgressParam
	{
		ProgressParam() :
			total_size(-1),
			current_size(-1),
			speed(-1),
			cbProgress(LUA_NOREF),
			L(NULL)
			{}
		~ProgressParam() {}
		double total_size;
		double current_size;
		double speed;
		int cbProgress;
		lua_State *L;
	private:
		DISALLOW_COPY_AND_ASSIGN(ProgressParam);
	};
	static DWORD __stdcall GetThread(void *);
	static DWORD __stdcall PostThread(void *);
	static DWORD __stdcall DownloadThread(void *);
	void RegisterClass();
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static size_t WriteToStringCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
	static size_t WriteToFileCallback(char *ptr, size_t size, size_t nmemb, void *userdata);

	HWND m_hwnd;
};

} // namespace cs
