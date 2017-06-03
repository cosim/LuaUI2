/*
** Copyright (C) 2015-2016 Chen Shi. See Copyright Notice in LuaUI2.h
*/

#include "StdAfx.h"
#include "Sprite.h"
#include "SpriteLua.h"
#include "ResourceManager.h"
#include <cmath>

extern lua_State *g_L; // GUI�߳��õ���lua state

namespace cs {

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
    m_bShowCaret = false;

    m_luaSide = NULL;
    m_notify = NULL;
}

Sprite::~Sprite(void)
{
	m_hostWnd = NULL;
	Sprite *sp = m_firstChild;
	while(sp)
	{
		Sprite *tmp = sp->m_nextSibling;
		sp->Unref(); // FIXME ��û�����������ʵ�������̫�� �᲻�ᱬջ��? ��ʵ����һ���ݹ����.
		sp = tmp;
	}
	m_firstChild = NULL;
	m_lastChild = NULL;
	m_prevSibling = NULL;
	m_nextSibling = NULL;
	m_parent = NULL;

    delete m_luaSide;
	LOG(<<"sprite deleted"); // TODO �Ӹ�����
}

Gdiplus::RectF Sprite::GetRect()
{
	return m_rect;
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
            // OnSize(rect.Width, rect.Height);
            Gdiplus::SizeF size;
            rect.GetSize(&size);
            SendNotify(eSizeChanged, &size);
		}
	}
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

void Sprite::OnDraw( Gdiplus::Graphics &g, const Gdiplus::RectF &rcDirty )
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
    PaintEvent ev;
    ev.graphics = &g;
    ev.rcDirty = rcDirty;
    SendNotify(ePaint, &ev);
    //this->ClientDraw(g, rcDirty);
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
			sp->OnDraw(g, rcDirty);
			g.TranslateTransform(-rc2.X, -rc2.Y);
		}
		sp = sp->m_nextSibling;
	}
	if (m_bClipChildren)
	{
		g.ResetClip();
	}
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

void Sprite::OnMouseEvent( lua_State *L, MouseEvent *ev )
{
	switch(ev->message)
	{
	case WM_MOUSEMOVE:
        SendNotify(eMouseMove, ev);
        //OnMouseMove(event->x, event->y, event->flag);
		if (!m_bMouseIn)
		{
			m_bMouseIn = true;
            SendNotify(eMouseEnter, ev);
			//OnMouseEnter();
		}
		break;
	case WM_MOUSEWHEEL:
		//OnMouseWheel(event);
        SendNotify(eMouseWheel, ev);
		break;
	case WM_LBUTTONDOWN:
        //OnLButtonDown(event->x, event->y, event->flag);
        SendNotify(eLBtnDown, ev);
		if (m_enableFocus)
		{
			GetHostWindow()->SetFocusSprite(this);
		}
		break;
	case WM_LBUTTONUP:
        //OnLButtonUp(event->x, event->y, event->flag);
        SendNotify(eLBtnUp, ev);
        break;
	case WM_MOUSELEAVE:
		m_bMouseIn = false;
        //OnMouseLeave();
        SendNotify(eMouseLeave, ev);
        break;
	}		
}

void Sprite::OnCapturedMouseEvent( lua_State *L, MouseEvent *ev)
{
	switch(ev->message)
	{
	case WM_MOUSEMOVE:
        SendNotify(eMouseMove, ev);
        //OnMouseMove(event->x, event->y, event->flag);
		break;
	case WM_LBUTTONDOWN:
        SendNotify(eLBtnDown, ev);
        //OnLButtonDown(event->x, event->y, event->flag);
		break;
	case WM_LBUTTONUP:
        SendNotify(eLBtnUp, ev);
        //OnLButtonUp(event->x, event->y, event->flag);
        break;
	}			
}

void Sprite::OnKeyEvent( lua_State *L, UINT message, DWORD keyCode, DWORD flag )
{
	KeyEvent ev;
    ev.keyCode = keyCode;
    ev.flag = flag;

	switch(message)
	{
	case WM_KEYDOWN:
        SendNotify(eKeyDown, &ev);
        //OnKeyDown(keyCode, flag);
		break;
	case WM_KEYUP:
        SendNotify(eKeyUp, &ev);
        //OnKeyUp(keyCode, flag);
		break;
	case WM_CHAR:
        SendNotify(eCharInput, &ev);
        //OnChar(keyCode, flag);
		break;
	}
}

void Sprite::OnImeInput(lua_State *L, LPCTSTR text)
{
    SendNotify(eImeInput, const_cast<wchar_t *>(text));
}

void Sprite::EnableFocus( bool enable )
{
	m_enableFocus = enable;
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

void Sprite::SetCapture()
{
	assert(GetHostWindow()); // �����lua��װ�������� ����lua����
	if (GetHostWindow())
	{
		GetHostWindow()->SetCapture(this);
	}
}

void Sprite::ReleaseCapture()
{
	assert(GetHostWindow()); // �����lua��װ�������� ����lua����
	if (GetHostWindow())
	{
		GetHostWindow()->ReleaseCapture();
	}
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

void Sprite::SetVisible( bool v )
{
	if (m_bVisible != v)
	{
		Invalidate();
	}
	m_bVisible = v;
}

bool Sprite::GetVisible()
{
	return m_bVisible;
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

Sprite * Sprite::GetParent()
{
	return m_parent;
}

void Sprite::TrackMouseLeave()
{
	HostWindow *wnd = GetHostWindow();
	if (wnd)
	{
		wnd->TrackMouseLeave(this);
	}
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

Sprite * Sprite::GetNextSprite()
{
	return m_nextSibling;
}



Sprite * Sprite::GetPrevSprite()
{
	return m_prevSibling;
}

Sprite * Sprite::GetFirstSubSprite()
{
	return m_firstChild;
}

Sprite * Sprite::GetLastSubSprite()
{
	return m_lastChild;
}

void Sprite::ShowCaret()
{
    GetHostWindow()->ShowCaret();
    m_bShowCaret = true;
}

void Sprite::SetCaretPos(Gdiplus::RectF rc)
{
    Gdiplus::RectF arc = Sprite::GetAbsRect();
    GetHostWindow()->SetImePosition(rc.X + arc.X, rc.Y + arc.Y);
    HWND hwnd = GetHostWindow()->GetHWND();
    ::DestroyCaret(); // �����������½��� ���ܸı�߶�
    ::CreateCaret(hwnd, NULL, (int)rc.Width, (int)rc.Height); // ���ԼӸ������ƶ����߹��(HBITMAP)1
    ::ShowCaret(hwnd);
    GetHostWindow()->SetCaretHeight(rc.Height);
    ::SetCaretPos((int)(rc.X + arc.X), (int)(rc.Y + arc.Y));
}

void Sprite::HideCaret()
{
    GetHostWindow()->HideCaret();
    m_bShowCaret = false;
}

LuaObject * Sprite::GetLuaSide()
{
    if (!m_luaSide)
    {
        m_luaSide = new SpriteLua(this);
    }
    return m_luaSide;
}

void Sprite::SendNotify(UINT idMessage, void *message)
{
    if (m_notify)
    {
        if (m_notify->OnNotify(m_id, this, idMessage, message))
            return;
    }
    GetLuaSide()->OnNotify(m_id, this, idMessage, message);
}

void Sprite::SetNotify(INotify *notify)
{
    m_notify = notify;
}

} // namespace cs