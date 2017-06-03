#pragma once

#include "Common.h"
#include "Object.h"

namespace cs {

class LuaObject;
class BitmapLua;

class Bitmap : public Object
{
public:
    Bitmap();
    virtual ~Bitmap();

    bool LoadFromFile(LPCWSTR path);
    Gdiplus::Bitmap * Get() { return m_bmp; }
    Gdiplus::SizeF GetSize();
    // TODO ��2����������� Ȼ��дһ��c++�� 9-patch.
    void GetFourStateInfo(std::vector<int> &info);
    void GetNineInOneInfo(std::vector<int> &info);

    virtual LuaObject * GetLuaSide() override;

private:
    Gdiplus::Bitmap *m_bmp;
    BitmapLua *m_luaSide;
    DISALLOW_COPY_AND_ASSIGN(Bitmap);
};

} // namespace cs
