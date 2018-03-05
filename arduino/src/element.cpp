#include <element.h>
#include <math.h>

//De klasse van Element, welke heel actief wordt gebruikt. Hier staan gewoon wat getters, meer dan dat is het eigenlijk niet
Element::Element(
    const double xbegin,
    const double ybegin,
    const double xend,
    const double yend, 
    const double xmiddle,
    const double ymiddle,
    const int type,
    const double length,
    const double circleradius,
    const double angle,
    const double beginradian,
    const double endradian)
  : xbegin(xbegin), 
  ybegin(ybegin), 
  xend(xend), 
  yend(yend),
  xmiddle(xmiddle),
  ymiddle(ymiddle),
  type(type),
  length(length),
  circleradius(circleradius),
  angle(angle),
  beginradian(beginradian),
  endradian(endradian)
{}

Element::Element() {}
 
double Element::getLength() const
{
    return this->length;
}

int Element::getType() const
{
    return this->type;
}

double Element::getXBegin() const
{
    return this->xbegin;
}

double Element::getYBegin() const
{
    return this->ybegin;
}

double Element::getXMiddle() const
{
    return this->xmiddle;
}

double Element::getYMiddle() const
{
    return this->ymiddle;
}

double Element::getXEnd() const
{
    return this->xend;
}

double Element::getYEnd() const
{
    return this->yend;
}

double Element::getRadius() const
{
    return this->circleradius;
}

double Element::getAngle() const
{
    return this->angle;
}

double Element::getAngleToRadian() const
{
    return this->angle * (M_PI / 180);
}

double Element::getBeginRadian() const
{
    return this->beginradian;
}

double Element::getEndRadian() const
{
    return this->endradian;
}