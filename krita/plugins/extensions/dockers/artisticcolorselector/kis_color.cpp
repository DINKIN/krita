/*
    Copyright (C) 2011 Silvio Heinrich <plassy@web.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <KoColorSpaceMaths.h>
#include "kis_color.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// --------- CoreImpl ------------------------------------------------------------------------ //

template<class HSXType>
struct CoreImpl: public KisColor::Core
{
    virtual void setRGB(float r, float g, float b, float a) {
        rgb(0) = r;
        rgb(1) = g;
        rgb(2) = b;
        hsx(3) = a;
        updateHSX();
    }
    
    virtual void setHSX(float h, float s, float x, float a) {
        hsx(0) = h;
        hsx(1) = s;
        hsx(2) = x;
        hsx(3) = a;
        updateRGB();
    }
    
    virtual void updateRGB() {
        float h = qBound(0.0f, hsx(0), 1.0f);
        float s = qBound(0.0f, hsx(1), 1.0f);
        float x = qBound(0.0f, hsx(2), 1.0f);
        
        KisColor::VecRGB gray(x, x, x);
        ::getRGB(rgb(0), rgb(1), rgb(2), h);
        ::setLightness<HSXType>(rgb(0), rgb(1), rgb(2), x);
        rgb = gray + (rgb - gray) * s;
    }
    
    virtual void updateHSX() {
        float r = qBound(0.0f, rgb(0), 1.0f);
        float g = qBound(0.0f, rgb(1), 1.0f);
        float b = qBound(0.0f, rgb(2), 1.0f);
        
        hsx(0) = ::getHue(r, g, b);
        hsx(1) = ::getSaturation<HSLType>(r, g, b);
        hsx(2) = ::getLightness <HSXType>(r, g, b);
    }
};


/////////////////////////////////////////////////////////////////////////////////////////////////
// --------- CoreHSVImpl --------------------------------------------------------------------- //

struct CoreHSVImpl: public KisColor::Core
{
    virtual void setRGB(float r, float g, float b, float a) {
        rgb(0) = r;
        rgb(1) = g;
        rgb(2) = b;
        hsx(3) = a;
        updateHSX();
    }
    
    virtual void setHSX(float h, float s, float x, float a) {
        hsx(0) = h;
        hsx(1) = s;
        hsx(2) = x;
        hsx(3) = a;
        updateRGB();
    }
    
    virtual void updateRGB() {
        float h = qBound(0.0f, hsx(0), 1.0f);
        float s = qBound(0.0f, hsx(1), 1.0f);
        float v = qBound(0.0f, hsx(2), 1.0f);
        
        ::getRGB(rgb(0), rgb(1), rgb(2), h);
        ::setSaturation<HSVType>(rgb(0), rgb(1), rgb(2), s);
        ::setLightness <HSVType>(rgb(0), rgb(1), rgb(2), v);
    }
    
    virtual void updateHSX() {
        float r = qBound(0.0f, rgb(0), 1.0f);
        float g = qBound(0.0f, rgb(1), 1.0f);
        float b = qBound(0.0f, rgb(2), 1.0f);
        
        hsx(0) = ::getHue(r, g, b);
        hsx(1) = ::getSaturation<HSVType>(r, g, b);
        hsx(2) = ::getLightness <HSVType>(r, g, b);
    }
};


/////////////////////////////////////////////////////////////////////////////////////////////////
// --------- KisColor ------------------------------------------------------------------------ //

KisColor::KisColor(Type type)
{
    initRGB(type, 0.0f, 0.0f, 0.0f, 0.0f);
}

KisColor::KisColor(float hue, float a, Type type)
{
    float r, g, b;
    ::getRGB(r, g, b, hue);
    initRGB(type, r, g, b, a);
}

KisColor::KisColor(float r, float g, float b, float a, Type type)
{
    initRGB(type, r, g, b, a);
}

KisColor::KisColor(const QColor& color, Type type)
{
    initRGB(type, color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

KisColor::KisColor(Qt::GlobalColor color, Type type)
{
    QColor c(color);
    initRGB(type, c.redF(), c.greenF(), c.blueF(), c.alphaF());
}

KisColor::KisColor(const KisColor& color)
{
    initHSX(color.getType(), color.getH(), color.getS(), color.getX(), color.getA());
}

KisColor::KisColor(const KisColor& color, KisColor::Type type)
{
    if(color.getType() == type)
        initHSX(type, color.getH(), color.getS(), color.getX(), color.getA());
    else
        initRGB(type, color.getR(), color.getG(), color.getB(), color.getA());
}

void KisColor::initRGB(Type type, float r, float g, float b, float a)
{
    switch(type)
    {
        case HSY: { new (m_coreData) CoreImpl<HSYType>; } break;
        case HSV: { new (m_coreData) CoreHSVImpl;       } break;
        case HSL: { new (m_coreData) CoreImpl<HSLType>; } break;
        case HSI: { new (m_coreData) CoreImpl<HSIType>; } break;
    }
    
    core()->type = type;
    core()->setRGB(r, g, b, a);
}

void KisColor::initHSX(Type type, float h, float s, float x, float a)
{
    switch(type)
    {
        case HSY: { new (m_coreData) CoreImpl<HSYType>; } break;
        case HSV: { new (m_coreData) CoreHSVImpl;       } break;
        case HSL: { new (m_coreData) CoreImpl<HSLType>; } break;
        case HSI: { new (m_coreData) CoreImpl<HSIType>; } break;
    }
    
    core()->type = type;
    core()->setHSX(h, s, x, a);
}

void KisColor::setRGBfromHue(float hue, float alpha)
{
    float r, g, b;
    ::getRGB(r, g, b, hue);
    core()->setRGB(r, g, b, alpha);
}

KisColor& KisColor::operator=(const KisColor& color)
{
    initHSX(color.getType(), color.getH(), color.getS(), color.getX(), color.getA());
    return *this;
}

