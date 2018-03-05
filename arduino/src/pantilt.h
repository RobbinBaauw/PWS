#ifndef pantilt_h
#define pantilt_h

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
    #include "pins_arduino.h"
    #include "WConstants.h"
#endif

#include <element.h>

void changeTiltLoop(
    int& iTiltTarget,
    double& dTiltPosition,
    const int iTiltSpeedFactor
);

void changeTilt(
    const int iTiltSpeed,
    double& dTiltPosition,
    const int iTiltSpeedFactor);

void changePanAuto(
    const double dCurrentMeters,
    const double dCurrentRadian, 
    const double dXFocus, 
    const double dYFocus, 
    double& dXDolly,
    double& dYDolly,
    const int iCurrentElement,
    const Element eElementList[10],
    const int iDirection,
    double& dPanAngle, 
    double& dPanOutput,
    double& dPanOutputFine,
    const int iDistanceFocusPoint,
    const int iDoAlter);

void changePan(
    const int iPanSpeed, 
    double& dPanAngle,
    double& dPanOutput, 
    double& dPanOutputFine);

#endif