#ifndef Element_h
#define Element_h

class Element
{
    public :
        Element();
        Element(
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
            const double endradian);
        double getLength() const;
        int getType() const;   
        double getXBegin() const;
        double getYBegin() const;
        double getXMiddle() const;
        double getYMiddle() const;
        double getXEnd() const;
        double getYEnd() const;
        double getRadius() const;
        double getAngle() const;
        double getAngleToRadian() const;
        double getBeginRadian() const;
        double getEndRadian() const;

    private :
        int type;
        double length;
        double xbegin;
        double xmiddle;
        double xend;
        double ybegin;
        double ymiddle;
        double yend;
        double circleradius;
        double angle;
        double beginradian;
        double endradian;
};

#endif