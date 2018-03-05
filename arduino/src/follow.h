#ifndef follow_h
#define follow_h

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
    #include "pins_arduino.h"
    #include "WConstants.h"
#endif

#include <element.h>

double dollyVolg(
    const double dCurrentRadian, 
    const double dCurrentMeters,
    const double dXFocus,
    const double dYFocus, 
    double& dXDolly,
    double& dYDolly,
    const int iCurrentElement, 
    const Element eElementList[10], 
    const int iDirection,
    const int iDistanceFocusPoint,
    const int iDoAlter);

void getFurtherDollyPosition(
    double& dFurtherXDolly,
    double& dFurtherYDolly,
    const int iCurrentElement,
    const Element eElementList[10],
    const int iDirection,
    const double dCurrentRadian,
    const double dCurrentMeters,
    const bool dDoLog
);

int getPointFocusCoordinates(
    const double dPanAngle,
    const int iDistanceFocusPoint,
    const double dCurrentRadian,
    const double dCurrentMeters, 
    double& dXFocus,
    double& dYFocus, 
    const int iCurrentElement, 
    const Element eElementList[10], 
    const int iDirection,
    double& dXDolly,
    double& dYDolly,
    const double dTotalLength);

void driveOverRailsTest(    
    double& dCurrentTestRadian,
    double& dCurrentTestMeters, 
    int& iCurrentTestElement, 
    const Element eElementList[10], 
    const int iTestDirection,
    const double dTotalLength);

void CorrectDollySlope(
    const double dXDolly,
    const double dYDolly,
    const double dFurtherXDolly,
    const double dFurtherYDolly,
    const Element eElementList[10], 
    const int iCurrentElement,
    double& dSlopeDolly 
);
#endif