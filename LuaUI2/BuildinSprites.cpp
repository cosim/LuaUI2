#include "stdafx.h"
#include "BuildinSprites.h"
#include "BuildinSpriteLua.h"
#include "ResourceManager.h"

extern lua_State *g_L; // GUI�߳��õ���lua state

namespace cs {

TextSprite::TextSprite() :m_color(255, 0, 255)
{
    m_font = NULL;
    SetFont(L"����", 12, Gdiplus::FontStyleRegular);
    m_vAlign = Gdiplus::StringAlignmentCenter;
    m_hAlign = Gdiplus::StringAlignmentCenter;
    m_luaSide = NULL;
}

TextSprite::~TextSprite()
{
    delete m_font;
    m_font = INVALID_POINTER(Gdiplus::Font);
    delete m_luaSide;
    m_luaSide = INVALID_POINTER(TextSpriteLua);
}

void TextSprite::SetText(LPCTSTR text)
{
    m_text = text;
    Sprite::Invalidate();
}

void TextSprite::SetFont(LPCTSTR familyName, float emSize, int style)
{
    Gdiplus::Font *font = new Gdiplus::Font(familyName, emSize, style);
    assert(font);
    if (font->IsAvailable())
    {
        delete m_font;
        m_font = font;
    }
	else
	{
		delete font;
	}
}

void TextSprite::SetColor(Gdiplus::Color clr)
{
    m_color = clr;
    Sprite::Invalidate();
}

void TextSprite::OnDraw(Gdiplus::Graphics &g, const Gdiplus::RectF &rcDirty)
{
    Gdiplus::SolidBrush brush(m_color);
    Gdiplus::StringFormat format;
    format.SetAlignment(m_hAlign);
    format.SetLineAlignment(m_vAlign);
    Gdiplus::RectF rc = Sprite::GetRect();
    rc.X = 0.0f;
    rc.Y = 0.0f;
    g.DrawString(m_text.c_str(), m_text.length(), m_font, rc, &format, &brush);
}

void TextSprite::SetHAlign(Gdiplus::StringAlignment align)
{
    m_hAlign = align;
}

LuaObject * TextSprite::GetLuaSide()
{
    if (!m_luaSide)
    {
        m_luaSide = new TextSpriteLua(this);
    }
    return m_luaSide;
}

RectangleSprite::RectangleSprite() :m_color(255, 0, 255), m_borderColor(255, 0, 255)
{
    m_luaSide = NULL;
}

RectangleSprite::~RectangleSprite()
{
    delete m_luaSide;
    m_luaSide = INVALID_POINTER(RectangleSpriteLua);
}

void RectangleSprite::SetColor(Gdiplus::Color color)
{
    if (m_color.GetValue() != color.GetValue())
    {
        m_color = color;
        Sprite::Invalidate();
    }
}

void RectangleSprite::SetBorderColor(Gdiplus::Color color)
{
    if (m_borderColor.GetValue() != color.GetValue())
    {
        m_borderColor = color;
        Sprite::Invalidate();
    }
}

void RectangleSprite::OnDraw(Gdiplus::Graphics &g, const Gdiplus::RectF &rcDirty)
{
    Gdiplus::SolidBrush brush(m_color);
    Gdiplus::RectF rc = Sprite::GetRect();
    // ����������Ͻ�©��һ���������
    // �ҵ������Gdiplus��Ϊ������0.5���� 
    // http://stackoverflow.com/questions/3147569/pixel-behaviour-of-fillrectangle-and-drawrectangle
    rc.X = -0.5f;
    rc.Y = -0.5f;
    g.FillRectangle(&brush, rc);
    Gdiplus::Pen pen(m_borderColor);
    g.DrawRectangle(&pen, rc);
    Sprite::OnDraw(g, rcDirty);
}

LuaObject * RectangleSprite::GetLuaSide()
{
    if (!m_luaSide)
    {
        m_luaSide = new RectangleSpriteLua(this);
    }
    return m_luaSide;
}

} // namespace cs
