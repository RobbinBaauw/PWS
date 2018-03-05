#ifndef utils_h
#define utils_h

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
    #include "pins_arduino.h"
    #include "WConstants.h"
#endif

#include <element.h>

double maptonumber(double x, double out_start, double out_finish);
double absolute(double x);
void calculateDollyPosition(
    const double dCurrentRadian, 
    const double dCurrentMeters, 
    const int iCurrentElement, 
    const Element eElementList[10], 
    const int iDirection,
    double& dXDolly,
    double& dYDolly,
    const bool bDoLogCoords
);
void calculateDollyPosition(
    const double dCurrentRadian, 
    const double dCurrentMeters, 
    const int iCurrentElement, 
    const Element eElementList[10], 
    const int iDirection,
    double& dXDolly,
    double& dYDolly,
    double& dSlopeDolly,
    const bool bDoLogCoords
);

bool isBetweenTwoPoints(
    double dPointOne,
    double dPointTwo,
    double dCurrentPoint
);
#endif