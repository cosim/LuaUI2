/*
** Copyright (C) 2015-2016 Chen Shi. See Copyright Notice in LuaUI2.h
*/

#pragma once

#include "Sprite.h"

namespace cs {

class EditSprite : public Sprite
{
protected:
	virtual ~EditSprite();
public:
    EditSprite();

	virtual void OnImeInput( lua_State *L, LPCTSTR text);

	virtual void ClientDraw( Gdiplus::Graphics &g, const Gdiplus::RectF &rcDirty );

	virtual void OnMouseEvent( lua_State *L, MouseEvent *event );

	virtual void OnCapturedMouseEvent( lua_State *L, MouseEvent *event );

	virtual void OnKeyEvent( lua_State *L, UINT message, DWORD keyCode, DWORD flag );

	virtual void OnSetFocus();

	virtual void OnKillFocus();

	BEGIN_LUA_METHOD_MAP(EditSprite)
		//LUA_METHOD_ENTRY(GetText)
		LUA_CHAIN_METHOD_MAP(SpriteLua)
	END_LUA_METHOD_MAP()

private:
	struct TextInfo
	{
		wchar_t ch;
		float width;
	};
	struct LineInfo
	{
		std::vector<TextInfo> vecText;
	};
	enum {
		NO_SELCET = -1
	};
	void UpdateLayout2( Gdiplus::Graphics &g );
	void UpdateImePos( Gdiplus::Graphics &g );
	void DrawSring(Gdiplus::Graphics &g, UINT pos, UINT length, Gdiplus::Color color);
	UINT CharPosFromPoint(float x, float y);
	float CalcStringWidth(UINT length);
	void AdjustScroll(DWORD key);
	void DrawStringWithSelection( Gdiplus::Graphics &g );

private:
	std::wstring m_text;
	Gdiplus::Font *m_font;
	float m_textHeight;
	float m_textWidth;
	Gdiplus::StringFormat m_format;
	Gdiplus::PointF m_ptScroll; // Sprite�����scroll������ ���Ǵ���ؼ��Լ������ ��Ϊ�б߿���� ���й����� ��Ҫ�ܿ�ȫ��Ĺ��� ����Ч�� ���һ�Ҫ�ж�̬������ʾ���� ��Щ����
	UINT m_caretCharPos;
	UINT m_selectCharPos; // ����������Ǹ����һ��ѡ������
	UINT m_oldSelectCharPos;
	bool m_bMouseDown;
	float m_caretScreenPos;
	int m_showCaretCount;
	float m_widthSpace;

	std::vector<LineInfo> m_vecLineInfo; //���������
	std::vector<TextInfo> m_vecTextInfo; //���������
};

} // namespace cs
