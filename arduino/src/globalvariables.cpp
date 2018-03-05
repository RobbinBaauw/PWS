#include <globalvariables.h>
#include <Arduino.h>

//Hier staan een aantal constantes gedefineerd, eigenlijk de instellingen van het programma
const double STEPPERINTERVAL = 0.1;
const double DRIVEINTERVAL = 0.5;

const double RADIANDEGREES = (180 / PI);
const double DEGREERADIAN = (PI / 180);

const double OMTREKWIEL = 0.2;
const double DRIVEGEARFACTOR = 1.5;

const int TILTMAXDEGREES = 40;
const int TILTMINDEGREES = -40;
const double TILTGEARFACTOR = 3.75;

const double PANDMXMAX = 255;
const double PANSPEEDFACTOR = 5;

const int MAXSPEEDOUTPUT = 240;
const int MINSPEEDOUTPUT = 15;

const double DRIVEOUTPUTCORRECTION = 1.5;

const int MAXTIMELAPSERPM = 50;

bool bGlobalLogs = false;