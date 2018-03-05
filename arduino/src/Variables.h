#ifndef test_h
#define test_h

#include <element.h>

double dCurrentMeters;
double dCurrentRadian;

double dXFocus;
double dYFocus;

double dXDolly;
double dYDolly;

int iDriveTargetPercent;
int iDriveTargetPrevPercent;

double dDriveTargetKmH;
double dDriveTargetKmHJoystick;

double dDriveRPMIncoming;
double dDriveRPMTarget;
double dTargetDistance;
double dDriveOutput;

int iPanSpeed;
double dPanAngle;
double dPanOutput;
double dPanOutputFine;
int iPanSpeedFactor;

int iTiltSpeed;
double dTiltPosition;
int iTiltTarget;
int iTiltSpeedFactor;

int iDistanceFocusPoint;

int isTracking;
int isTimelapse;
int isDriving;

int iTimelapseTime;
int iTimelapseEndPercent;
double dCurrentPercentDone;

int iCurrentElement;
int iDirection;
int iDoAlter;

double dTotalLength;
Element eElementList[10];

unsigned long lPreviousPositionUpdate;

String sReceivedData;

char cRPMAnalogInput[6];
String sRPMAnalogNumber;
int iRPMAnalogIndex;

bool bFreeRoam;

#endif