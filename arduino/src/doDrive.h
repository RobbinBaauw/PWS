#ifndef doDrive_h
#define doDrive_h

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
    #include "pins_arduino.h"
    #include "WConstants.h"
#endif

#include <element.h>

double doDriveMotor(
    const double dDriveTargetKmH,
    const int iDriveTargetPercent, 
    int& iDriveTargetPrevPercent,
    const double dTotalLength,
    const Element eElementList[10],
    const int iCurrentElement,
    double& dXDolly,
    double& dYDolly,
    double& dCurrentMeters,
    double& dCurrentRadian,
    double& dTargetDistance,
    int& iDirection,
    int& isDriving,
    const int isTimelapse,
    const int iTimelapseTime,
    unsigned long& lPreviousPositionUpdate);

double doDriveMotorJoystick(
    const double dDriveTargetKmHJoystick,
    int& iDirection,
    int& isDriving,
    unsigned long& lPreviousPositionUpdate,
    const double dCurrentMeters,
    const double dCurrentRadian,
    const Element eElementList[10],
    const int iCurrentElement
    );

double calculateMetersDone(
    const double dCurrentMeters,
    const double dCurrentRadian,
    const Element eElementList[10],
    const int iCurrentElement
);

double updateCurrentPosition(
    const double dDriveRPMIncoming,
    const double dDriveTargetKmH,
    unsigned long& lPreviousPositionUpdate,
    double& dTargetDistance,
    double& dXDolly,
    double& dYDolly,
    double& dCurrentMeters,
    double& dCurrentRadian,
    const Element eElementList[10],
    int& iCurrentElement,
    const int iDirection,
    int& isDriving,
    int& isTimelapse,
    double& dDriveRPMTarget,
    const int iDriveTargetPercent,
    const int iDriveTargetPrevPercent);

double updateCurrentPositionJoystick(
    const double dDriveRPMIncoming,
    const double dDriveTargetKmHJoystick,
    unsigned long& lPreviousPositionUpdate,
    double& dXDolly,
    double& dYDolly,
    double& dCurrentMeters,
    double& dCurrentRadian,
    const Element eElementList[10],
    int& iCurrentElement,
    int& iDirection,
    int& isDriving,
    double& dDriveRPMTarget,
    int& iDriveTargetPercent,
    const double dTotalLength
);

void checkNewElement(
    double& dCurrentRadian, 
    double& dCurrentMeters, 
    int& iCurrentElement, 
    const Element eElementList[10], 
    const int iDirection,
	const double dMetersDriven
);
#endif