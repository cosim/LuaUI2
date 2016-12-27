/*
** Copyright (C) 2015-2016 Chen Shi. See Copyright Notice in LuaUI2.h
*/

#include "StdAfx.h"
#include "Sprite.h"
#include "ResourceManager.h"
#include <cmath>

extern lua_State *g_L; // GUI�߳��õ���lua state

Sprite::Sprite(void)
{
	m_rect.X = 0;
	m_rect.Y = 0;
	m_rect.Width = 10;
	m_rect.Height = 10;

	m_hostWnd = NULL;

	m_firstChild = NULL;
	m_lastChild = NULL;
	m_prevSibling = NULL;
	m_nextSibling = NULL;
	m_parent = NULL;

	m_bVisible = true;
	m_enableFocus = false;
	m_bMouseIn = false;
	m_bClipChildren = false;
}

Sprite::~Sprite(void)
{
	m_hostWnd = NULL;
	Sprite *sp = m_firstChild;
	while(sp)
	{
		Sprite *tmp = sp->m_nextSibling;
		sp->Unref();
		sp = tmp;
	}
	m_firstChild = NULL;
	m_lastChild = NULL;
	m_prevSibling = NULL;
	m_nextSibling = NULL;
	m_parent = NULL;
	LOG(<<"deleted:"<< GetClassName()); // FIXME ����Ϊɶ���ܶ�̬��?
}

Gdiplus::RectF Sprite::GetRect()
{
	return m_rect;
}

int Sprite::GetRect( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	Gdiplus::RectF rc = thiz->GetRect();
	lua_pushnumber(L, rc.X);
	lua_pushnumber(L, rc.Y);
	lua_pushnumber(L, rc.Width);
	lua_pushnumber(L, rc.Height);
	return 4;
}

int Sprite::GetRectT( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	Gdiplus::RectF rc = thiz->GetRect();
	RectF2LuaRect(L, rc);
	return 1;
}

Gdiplus::RectF Sprite::GetAbsRect()
{
	Sprite *sp = m_parent;
	Gdiplus::RectF rcSelf = GetRect();
	Gdiplus::RectF rcParent;
	while(sp)
	{
		rcParent = sp->GetRect();
		rcSelf.Offset(rcParent.X, rcParent.Y);
		sp = sp->m_parent;
	}
	return rcSelf;
}

int Sprite::GetAbsRect( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	Gdiplus::RectF rc = thiz->GetAbsRect();
	lua_pushnumber(L, rc.X);
	lua_pushnumber(L, rc.Y);
	lua_pushnumber(L, rc.Width);
	lua_pushnumber(L, rc.Height);
	return 4;
}

int Sprite::GetAbsRectT(lua_State *L)
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	Gdiplus::RectF rc = thiz->GetAbsRect();
	RectF2LuaRect(L, rc);
	return 1;
}

void Sprite::SetRect( Gdiplus::RectF rect )
{
	// ����¿���Ƿ�С��0 ������Ϊ0 Ȼ��0���0��Ҫ��OnDraw��Щ�������⴦��һ��
	rect.Width = max(0.0f, rect.Width);
	rect.Height = max(0.0f, rect.Height);
	if (!m_rect.Equals(rect))
	{
		Gdiplus::RectF rcOld = m_rect;
		// TODO OnMove
		Invalidate(); // �ɾ���
		m_rect = rect;
		Invalidate(); // �¾���
		// ԭ�Ȼص���˳����� ������OnSize����GetRect��OnSize�Ĳ�����һ�� �������
		if (rect.Width != rcOld.Width || rect.Height != rcOld.Height)
		{
			lua_State *L = g_L;
			lua_pushnumber(L, rect.Width);
			lua_pushnumber(L, rect.Height);
			InvokeCallback(L, "OnSize", 2, 0);
		}
	}
}

int Sprite::SetRect( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	Gdiplus::RectF rc;
	if (lua_istable(L, 2))
	{
		lua_getfield(L, 2, "x");
		rc.X = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 2, "y");
		rc.Y = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 2, "w");
		rc.Width = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 2, "h");
		rc.Height = (float)luaL_checknumber(L, -1);
		lua_pop(L, 1);
	}
	else
	{
		rc.X = (float)luaL_checknumber(L, 2);
		rc.Y = (float)luaL_checknumber(L, 3);
		rc.Width = (float)luaL_checknumber(L, 4);
		rc.Height = (float)luaL_checknumber(L, 5);

	}
	thiz->SetRect(rc);
	return 0;
}

void Sprite::Invalidate()
{
	// 0ָ����� ��������Ϊx64ϵͳһ��bug �ǵô򿪵����е�Win32�쳣�ϵ�
	HostWindow *wnd = GetHostWindow();
	if (wnd)
	{
		RECT rc;
		Gdiplus::RectF rf = GetAbsRect();
		rc.left = (LONG)(rf.GetLeft() - 0.5f);
		rc.top = (LONG)(rf.GetTop() - 0.5f);  // TODO FIXME ���ֵ���Գ����� ��֪������Ȼ
		rc.right = (LONG)(rf.GetRight() + 1.5f); // ��С����TabCtrl������Ӱ ����ĳ�2 �Ϳ���������Ӱ����
		rc.bottom = (LONG)(rf.GetBottom() + 1.5f); // �ܹ��� ��������ΪGdiPlus��Ϊxȡ��� widthҲȡ���
		::InvalidateRect(wnd->GetHWND(), &rc, TRUE);
	}
}

// ֻ��CUIWindow::AddSprite��ʹ��
void Sprite::SetHostWnd( HostWindow *wnd )
{
	LOG("BEGIN");
	// FIXME �������������� AddChildʱ�������� ��ʹ�㷨���Ӷȱ���
	m_hostWnd = wnd;
	// m_hostWnd->Ref(); Ӧ����������
	//Sprite *sp = m_firstChild;
	//while(sp)
	//{
	//	sp->SetHostWnd(wnd);
	//	sp = sp->m_nextSibling;
	//}
	LOG("END");
}

void Sprite::OnDraw( lua_State *L, Gdiplus::Graphics &g, const Gdiplus::RectF &rcDirty )
{
	if (!m_bVisible)
	{
		return; // �ӽڵ�Ҳ���ᱻ����
	}
	// ǰ����� �ø��ڵ��Ȼ���
	//Gdiplus::RectF rc = GetRect();
	//if (10 == rc.Width && 10 == rc.Height)
	//{
	//	LOGW(<<L"Orignal Size 10 10"); // �������û�ж�����ػ�
	//}
	if (m_bClipChildren)
	{
		Gdiplus::RectF rcClip = GetRect();
		rcClip.X = 0.0f;
		rcClip.Y = 0.0f;
		g.SetClip(rcClip);
	}
	Sprite *sp = m_firstChild;
	while(sp)
	{
		// �����Ҫ�ػ沿�־��κ�sp�ཻ���ػ��� �����ػ�
		// ���ﻹ�и�������� �����α�����Ӿ���Ҫ�� ������ܸ����ཻ���� ���ӵ��ཻ�ĵ�
		// ����Ҫǿ����һԭ�� ����������� ��Ŵ�
		Gdiplus::RectF rc2 = sp->GetRect();
		Gdiplus::RectF rcAbs = sp->GetAbsRect();
		rcAbs.X -= 0.5f; // FIXME ��ʱ�޷��õ��ػ����±߽�1������ʧ
		rcAbs.Y -= 0.5f;
		rcAbs.Width += 1.0f;
		rcAbs.Height += 1.0f;
		if (rcDirty.IntersectsWith(rcAbs))
		{
			g.TranslateTransform(rc2.X, rc2.Y);
			sp->OnDraw(L, g, rcDirty);
			g.TranslateTransform(-rc2.X, -rc2.Y);
		}
		sp = sp->m_nextSibling;
	}
	if (m_bClipChildren)
	{
		g.ResetClip();
	}
}

int Sprite::AddChild( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	Sprite *sprite = CheckLuaObject<Sprite>(L, 2);
	thiz->AddChild(sprite);
	return 0;
}

void Sprite::AddChild( Sprite *sp )
{
	if (!m_firstChild)
	{
		assert(NULL == m_lastChild);
		m_firstChild = sp;
		m_lastChild = sp;
	}
	else
	{
		Sprite *tmp = m_lastChild;
		m_lastChild = sp;
		sp->m_prevSibling = tmp;
		tmp->m_nextSibling = sp;
	}
	//sp->SetHostWnd(m_hostWnd); // �ݹ�ȥ���� ��ֹ����˳����뵼������ FIXME ��������������
	sp->m_parent = this;
	sp->Ref();
}

void Sprite::OnMouseEvent( lua_State *L, MouseEvent *event )
{
	/*
	Sprite *sp = m_lastChild; // ����ĳɴӺ���ǰ ��Ϊ��ʾ��ʱ���Ǻ����ڵ�ǰ��� ����ֻҪ��һ�������¼�(ͬ��) �Ͳ��ټ�������
	bool bHit = false;
	while(sp)
	{
		Gdiplus::RectF rc2 = sp->GetRect();
		if (rc2.Contains(event->x, event->y))
		{
			if (!bHit)
			{
				MouseEvent event2 = *event;
				event2.x = event->x -  rc2.X;
				event2.y = event->y - rc2.Y;
				sp->OnMouseEvent(L, &event2);
				bHit = true; // ����ֻ����һ��
			}
		}
		else
		{
			// ��OnMouseLeaveҪȷ������
			if (WM_MOUSEMOVE == event->message && sp->m_bMouseIn)
			{
				sp->m_bMouseIn = false;
				sp->InvokeCallback(L, "OnMouseLeave", 0, 0);
			}
		}
		sp = sp->m_prevSibling;
	}
	*/
	lua_pushnumber(L, event->x);
	lua_pushnumber(L, event->y);
	lua_pushinteger(L, event->flag);
	switch(event->message)
	{
	case WM_MOUSEMOVE:
		InvokeCallback(L, "OnMouseMove", 3, 0); // TODO lua������ֹ��Ϣ"ð��" ͨ������"abort"
		if (!m_bMouseIn)
		{
			m_bMouseIn = true;
			InvokeCallback(L, "OnMouseEnter", 0, 0);
		}
		break;
	case WM_MOUSEWHEEL:
		lua_pushnumber(L, event->delta);
		InvokeCallback(L, "OnMouseWheel", 4, 0);
		break;
	case WM_LBUTTONDOWN:
		InvokeCallback(L, "OnLButtonDown", 3, 0);
		if (m_enableFocus)
		{
			GetHostWindow()->SetFocusSprite(this);
		}
		break;
	case WM_LBUTTONUP:
		InvokeCallback(L, "OnLButtonUp", 3, 0);
		break;
	case WM_MOUSELEAVE:
		m_bMouseIn = false;
		InvokeCallback(L, "OnMouseLeave", 3, 0);
		break;
	}		
}

void Sprite::OnCapturedMouseEvent( lua_State *L, MouseEvent *event)
{
	lua_pushnumber(L, event->x);
	lua_pushnumber(L, event->y);
	lua_pushinteger(L, event->flag);
	switch(event->message)
	{
	case WM_MOUSEMOVE:
		InvokeCallback(L, "OnMouseMove", 3, 0);
		break;
	case WM_LBUTTONDOWN:
		InvokeCallback(L, "OnLButtonDown", 3, 0);
		break;
	case WM_LBUTTONUP:
		InvokeCallback(L, "OnLButtonUp", 3, 0);
		break;
	}			
}

void Sprite::OnKeyEvent( lua_State *L, UINT message, DWORD keyCode, DWORD flag )
{
	// ֻ�����������
	lua_pushinteger(L, keyCode);
	lua_pushinteger(L, flag);

	switch(message)
	{
	case WM_KEYDOWN:
		InvokeCallback(L, "OnKeyDown", 2, 0);
		break;
	case WM_KEYUP:
		InvokeCallback(L, "OnKeyUp", 2, 0);
		break;
	case WM_CHAR:
		InvokeCallback(L, "OnChar", 2, 0);
		break;
	}
}


void Sprite::EnableFocus( bool enable )
{
	m_enableFocus = enable;
}

int Sprite::EnableFocus( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	bool enbale = false;
	if (lua_isnone(L, 2))
	{
		enbale = true;
	}
	else if (lua_isboolean(L, 2))
	{
		enbale = (lua_toboolean(L, 2) != 0);
	}
	else
	{
		luaL_error(L, "need a boolean value.");
	}
	thiz->EnableFocus(enbale);
	return 0;
}

HostWindow * Sprite::GetHostWindow()
{
	Sprite *sp = this;
	while (sp->m_parent)
	{
		sp = sp->m_parent;
	}
	return sp->m_hostWnd;
}

void Sprite::CreateInstance( Sprite **ppObj )
{
	*ppObj = new Sprite;
}

void Sprite::SetCapture()
{
	assert(GetHostWindow()); // �����lua��װ�������� ����lua����
	if (GetHostWindow())
	{
		GetHostWindow()->SetCapture(this);
	}
}

int Sprite::SetCapture( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	thiz->SetCapture();
	return 0;
}

void Sprite::ReleaseCapture()
{
	assert(GetHostWindow()); // �����lua��װ�������� ����lua����
	if (GetHostWindow())
	{
		GetHostWindow()->ReleaseCapture();
	}
}

int Sprite::ReleaseCapture( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	thiz->ReleaseCapture();
	return 0;
}

void Sprite::BringToFront()
{
	if (m_parent)
	{
		if (this == m_parent->m_lastChild)
		{
			return;
		}
		// 
		if (m_prevSibling)
		{
			m_prevSibling->m_nextSibling = m_nextSibling;
		}
		else
		{
			m_parent->m_firstChild = m_nextSibling;
		}
		m_nextSibling->m_prevSibling = m_prevSibling; // û��next˵�������һ�� �����������

		m_prevSibling = m_parent->m_lastChild;
		m_nextSibling = NULL;
		m_parent->m_lastChild->m_nextSibling = this;
		m_parent->m_lastChild = this;

		Invalidate();
	}
}

int Sprite::BringToFront( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	thiz->BringToFront();
	return 0;
}

void Sprite::OnSetFocus()
{
	lua_State *L = g_L; // TODO ����øĵ� ������ĳ������ĳ�Ա m_L
	InvokeCallback(L, "OnSetFocus", 0, 0);
}

void Sprite::OnKillFocus()
{
	lua_State *L = g_L;
	InvokeCallback(L, "OnKillFocus", 0, 0);
}

void Sprite::SetVisible( bool v )
{
	if (m_bVisible != v)
	{
		Invalidate();
	}
	m_bVisible = v;
}

int Sprite::SetVisible( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	int b = lua_toboolean(L, 2);
	thiz->SetVisible(b != 0);
	return 0;
}

bool Sprite::GetVisible()
{
	return m_bVisible;
}

int Sprite::GetVisible( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	bool bVisible = thiz->GetVisible();
	lua_pushboolean(L, bVisible ? 1 : 0);
	return 1;
}

int Sprite::SetClipChildren( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	bool bClip = (lua_toboolean(L, 2) != 0);
	thiz->SetClipChildren(bClip);
	return 0;
}

void Sprite::SetClipChildren( bool bClip )
{
	if (m_bClipChildren != bClip)
	{
		m_bClipChildren = bClip;
		Invalidate();
	}
}

bool Sprite::GetClipChildren()
{
	return m_bClipChildren;
}

int Sprite::GetClipChildren( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	bool bClip = thiz->GetClipChildren();
	lua_pushboolean(L, bClip ? 1 : 0);
	return 1;
}

// http://blog.csdn.net/magic_feng/article/details/6618206
Sprite * Sprite::DispatchMouseEvent(MouseEvent *event)
{
	std::stack<Sprite *> stack;
	std::stack<Sprite *> reverse;
	stack.push(this);
	Sprite *sp = NULL;
	
	while(!stack.empty())
	{
		sp = stack.top();
		stack.pop();
		reverse.push(sp); // visit in reversed order.
		
		Sprite *sp2 = sp->m_lastChild;
		while (sp2)
		{
			stack.push(sp2);
			sp2 = sp2->m_prevSibling;
		}
	}
	while (!reverse.empty())
	{
		sp = reverse.top();
		reverse.pop();

		Gdiplus::RectF rc = sp->GetAbsRect();
		//LOG("HitTest:"<< rc.X << "," <<rc.Y << "," << rc.Width << "," << rc.Height);
		if (rc.Contains(event->x, event->y))
		{
			MouseEvent e2 = *event;
			e2.x -= rc.X;
			e2.y -= rc.Y;
			sp->OnMouseEvent(g_L, &e2);
		}
	}
	return NULL;
}

Sprite * Sprite::GetAncestor()
{
	Sprite *sp = this;
	while (sp->m_parent)
	{
		sp = sp->m_parent;
	}
	return sp;
}

int Sprite::GetAncestor( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	Sprite *sp = thiz->GetAncestor();
	sp->PushToLua(L);
	return 1;
}

Sprite * Sprite::GetParent()
{
	return m_parent;
}

int Sprite::GetParent( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	Sprite *sp = thiz->GetParent();
	sp->PushToLua(L);
	return 1;
}

void Sprite::TrackMouseLeave()
{
	HostWindow *wnd = GetHostWindow();
	if (wnd)
	{
		wnd->TrackMouseLeave(this);
	}
}

int Sprite::TrackMouseLeave( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	thiz->TrackMouseLeave();
	return 0;
}

void Sprite::RemoveChild( Sprite *sp )
{
	assert(sp != this);
	assert(sp->m_parent == this);
	// TODO Ҫ��Ҫ���Բ���һ�� �ǲ������������Ŀ��
#ifdef _DEBUG
	Sprite *f = m_firstChild;
	bool bFound = false;
	while(f)
	{
		if (f == sp)
		{
			bFound = true;
			break;
		}
		f = f->m_nextSibling;
	}
	assert(bFound);
#endif

	if (sp == this->m_lastChild)
	{
		this->m_lastChild = sp->m_prevSibling;
	}
	if (sp == this->m_firstChild)
	{
		this->m_firstChild = sp->m_nextSibling;
	}
	if (sp->m_prevSibling)
	{
		sp->m_prevSibling->m_nextSibling = sp->m_nextSibling;
	}
	if (sp->m_nextSibling)
	{
		sp->m_nextSibling->m_prevSibling = sp->m_prevSibling;
	}
	sp->Unref();
}

int Sprite::RemoveChild( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	Sprite *sp = CheckLuaObject<Sprite>(L, 2);
	thiz->RemoveChild(sp);
	return 0;
}

Sprite * Sprite::GetNextSprite()
{
	return m_nextSibling;
}

int Sprite::GetNextSprite( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	Sprite *sp = thiz->GetNextSprite();
	sp->PushToLua(L);
	return 1;
}

Sprite * Sprite::GetPrevSprite()
{
	return m_prevSibling;
}

int Sprite::GetPrevSprite( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	Sprite *sp = thiz->GetPrevSprite();
	sp->PushToLua(L);
	return 1;
}

Sprite * Sprite::GetFirstSubSprite()
{
	return m_firstChild;
}

int Sprite::GetFirstSubSprite( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	Sprite *sp = thiz->GetFirstSubSprite();
	sp->PushToLua(L);
	return 1;
}

Sprite * Sprite::GetLastSubSprite()
{
	return m_lastChild;
}

int Sprite::GetLastSubSprite( lua_State *L )
{
	Sprite *thiz = CheckLuaObject<Sprite>(L, 1);
	Sprite *sp = thiz->GetLastSubSprite();
	sp->PushToLua(L);
	return 1;
}

TextSprite::TextSprite():m_color(255,0,255)
{
	m_font = NULL;
	SetFont(L"����", 12, Gdiplus::FontStyleRegular);
	m_vAlign = Gdiplus::StringAlignmentCenter;
	m_hAlign = Gdiplus::StringAlignmentCenter;
}

TextSprite::~TextSprite()
{
	m_font = NULL;
}

void TextSprite::SetText( LPCTSTR text )
{
	m_text = text;
	Invalidate();
}

int TextSprite::SetText( lua_State *L )
{
	TextSprite *thiz = CheckLuaObject<TextSprite>(L, 1);
	CString str = luaL_checkwstring(L, 2);
	thiz->SetText(str);
	return 0;
}

void TextSprite::SetFont( LPCTSTR familyName, float emSize, int style )
{
	Gdiplus::Font *font = ResourceManager::Instance()->GetFont(familyName, emSize, style);
	ATLASSERT(font);
	m_font = font;
}

int TextSprite::SetFont( lua_State *L )
{
	TextSprite *thiz = CheckLuaObject<TextSprite>(L, 1);
	CString face = luaL_checkwstring(L, 2);
	float size = (float)luaL_checknumber(L, 3);
	int style = luaL_checkinteger(L, 4);
	thiz->SetFont(face, size, style);
	return 0;
}

void TextSprite::SetColor( Gdiplus::Color clr )
{
	m_color = clr;
	Invalidate();
}

int TextSprite::SetColor( lua_State *L )
{
	TextSprite *thiz = CheckLuaObject<TextSprite>(L, 1);
	Gdiplus::Color color = luaL_checkcolor(L, 2);
	thiz->SetColor(color);
	return 0;
}

void TextSprite::OnDraw( lua_State *L, Gdiplus::Graphics &g, const Gdiplus::RectF &rcDirty )
{
	Gdiplus::SolidBrush brush(m_color);
	Gdiplus::StringFormat format;
	format.SetAlignment(m_hAlign);
	format.SetLineAlignment(m_vAlign);
	Gdiplus::RectF rc = GetRect();
	rc.X = 0.0f;
	rc.Y = 0.0f;
	g.DrawString(m_text.c_str(), m_text.length(), m_font, rc, &format,&brush);
}

void TextSprite::SetHAlign( Gdiplus::StringAlignment align )
{
	m_hAlign = align;
}

void TextSprite::CreateInstance( TextSprite **ppObj )
{
	*ppObj = new TextSprite;
}

RectangleSprite::RectangleSprite() :m_color(255, 0, 255), m_borderColor(255, 0, 255)
{
}

RectangleSprite::~RectangleSprite()
{
}

void RectangleSprite::SetColor( Gdiplus::Color color )
{
	if (m_color.GetValue() != color.GetValue())
	{
		m_color = color;
		Invalidate();
	}
}

int RectangleSprite::SetColor( lua_State *L )
{
	RectangleSprite *thiz = CheckLuaObject<RectangleSprite>(L, 1);
	Gdiplus::Color color = luaL_checkcolor(L, 2);
	thiz->SetColor(color);
	return 0;
}

void RectangleSprite::SetBorderColor( Gdiplus::Color color )
{
	if (m_borderColor.GetValue() != color.GetValue())
	{
		m_borderColor = color;
		Invalidate();
	}
}

int RectangleSprite::SetBorderColor( lua_State *L )
{
	RectangleSprite *thiz = CheckLuaObject<RectangleSprite>(L, 1);
	Gdiplus::Color color = luaL_checkcolor(L, 2);
	thiz->SetBorderColor(color);
	return 0;
}


void RectangleSprite::OnDraw( lua_State *L, Gdiplus::Graphics &g, const Gdiplus::RectF &rcDirty )
{
	Gdiplus::SolidBrush brush(m_color);
	Gdiplus::RectF rc = GetRect();
	rc.X = 0.0f;
	rc.Y = 0.0f;
	g.FillRectangle(&brush, rc);
	Gdiplus::Pen pen(m_borderColor);
	g.DrawRectangle(&pen,rc);
	Sprite::OnDraw(L, g, rcDirty);
}

void RectangleSprite::CreateInstance( RectangleSprite **ppObj )
{
	*ppObj = new RectangleSprite;
}
