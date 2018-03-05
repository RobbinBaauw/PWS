#include <pantilt.h>
#include <Arduino.h>
#include <utils.h>
#include <globalvariables.h>
#include <follow.h>

#include <A4988.h>

A4988 aTilt(200, 7, 8);

//Deze functie wordt aangeroepen wanneer de joystick wordt gebruikt om te tilten, en zal de informatie naar de stepper driver sturen
void changeTilt(
    const int iTiltSpeed,
    double& dTiltPosition,
    const int iTiltSpeedFactor)
{
    //Je hebt een tiltspeed ingesteld door de joystick te gebruiken. Deze wordt vermenigvuldigd met een bepaalde factor gedeeld door 10, deze factor is in te stellen in de app.
    //Je ziet ook een tiltgearfactor, dit is een factor die de tilt vertraagt, en om een bepaalde snelheid te halen moet de motor dus de gearfactor keer zo snel gaan draaien
    double dTiltRPM = iTiltSpeed * (iTiltSpeedFactor / 10) * TILTGEARFACTOR;

    //Wanneer de motor wordt bewogen wordt de rest van de executie van het programma tegengehouden, wat niet de bedoeling is. Daarom zorgen we ervoor dat het uitvoeren van de motor maar STEPPERINTERVAL duurt
    //Dit wordt verwezenlijkt door de RPM te delen door 60 (RPS), te vermenigvuldigen met 360 (Graden per seconde) en dat vermenigvuldigen met de stepper interval
    double dAnglePerInterval = dTiltRPM * 6.0 * STEPPERINTERVAL;

    //Dit is de anglerperinterval maar dan zonder de tiltgearfactor, dus hoeveel graden per seconde wordt gedraaid
    double dAnglePerIntervalDivided = dAnglePerInterval / TILTGEARFACTOR;

    //Wanneer de nieuwe tiltpositie binnen de mogelijke tiltposities valt, roteer het
    if(dTiltPosition + dAnglePerIntervalDivided < TILTMAXDEGREES && dTiltPosition + dAnglePerIntervalDivided > TILTMINDEGREES)
    {
        dTiltPosition += dAnglePerIntervalDivided;
        aTilt.setRPM(absolute((int) dTiltRPM));
        aTilt.rotate((int) dAnglePerInterval);
    }
}

//Deze functie wordt aangeroepen in de loop om te tilten door middel van een doel
void changeTiltLoop(
    int& iTiltTarget,
    double& dTiltPosition,
    const int iTiltSpeedFactor
)
{
    //Bereken de tiltRPM door middel van een speedfactor
    //Je ziet ook een tiltgearfactor, dit is een factor die de tilt vertraagt, en om een bepaalde snelheid te halen moet de motor dus de gearfactor keer zo snel gaan draaien
    double dTiltRPM = 5.0 * (iTiltSpeedFactor / 10.0) * TILTGEARFACTOR;

    //Wanneer de motor wordt bewogen wordt de rest van de executie van het programma tegengehouden, wat niet de bedoeling is. Daarom zorgen we ervoor dat het uitvoeren van de motor maar STEPPERINTERVAL duurt
    //Dit wordt verwezenlijkt door de RPM te delen door 60 (RPS), te vermenigvuldigen met 360 (Graden per seconde) en dat vermenigvuldigen met de stepper interval
    double dAnglePerInterval = dTiltRPM * 6.0 * STEPPERINTERVAL;

    //Dit is de anglerperinterval maar dan zonder de tiltgearfactor, dus hoeveel graden per seconde wordt gedraaid
    double dAnglePerIntervalDivided = dAnglePerInterval / TILTGEARFACTOR;

    //Als het target groter is dan de max of kleiner dan de min, zet het als de max of de min
    if(iTiltTarget > TILTMAXDEGREES)
    {
        iTiltTarget = TILTMAXDEGREES;
    }
    else if(iTiltTarget < TILTMINDEGREES)
    {
        iTiltTarget = -TILTMINDEGREES;
    }

    //Er wordt gekeken of het verschil in het doel en de huidige positie is groter is dan wat geroteerd kan worden
    if(absolute(iTiltTarget - dTiltPosition) > dAnglePerIntervalDivided)
    {
        aTilt.setRPM(dTiltRPM);

        //Zoja, als het doel groter is dan de positie, roteer dAnglePerIntervalDivided
        if(iTiltTarget - dTiltPosition > 0)
        {
            dTiltPosition += dAnglePerIntervalDivided;
            aTilt.rotate((int) (dAnglePerInterval + 1));

        }
        //Is het doel kleiner dan de positie, roteer -dAnglePerIntervalDivided
        else
        {
            dTiltPosition -= dAnglePerIntervalDivided;
            aTilt.rotate(-(int) (dAnglePerInterval + 1));
        }

    }
    //Als de afstand behoorlijk klein is tussen het doel en de huidige positie, roteer het verschil
    else if(absolute(iTiltTarget - dTiltPosition) > 0.1)
    {
        aTilt.setRPM(dTiltRPM);
        
        dTiltPosition += (iTiltTarget - dTiltPosition);
        aTilt.rotate((int) (iTiltTarget - dTiltPosition + 0.5));

    }
}

//Deze functie wordt elke 100ms aangeroepen wanneer er getrackt wordt
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
    const int iDoAlter)
{
    //Call de dollyVolg functie om de panangle uit te rekenen
    dPanAngle = dollyVolg(
        dCurrentRadian,
        dCurrentMeters,
        dXFocus,
        dYFocus,
        dXDolly,
        dYDolly,
        iCurrentElement,
        eElementList,
        iDirection,
        iDistanceFocusPoint,
        iDoAlter);

    //Map de net berekende panangle (kan tussen de -270 en 270 liggen) naar een waarde tussen 0 en 255
    dPanOutput = maptonumber(dPanAngle + 270, 540, 255);

    //Bereken de fine daarvan, de decimaal ook naar 0 en 255 gemapt
    dPanOutputFine = maptonumber(dPanOutput - (int) dPanOutput, 1, 255);
}

//Deze functie wordt aangeroepen wanneer de joystick wordt gebruikt om te pannen
void changePan(
    const int iPanSpeed, 
    double& dPanAngle,
    double& dPanOutput, 
    double& dPanOutputFine)
{ 
    //Je hebt een panspeed ingesteld door de joystick te gebruiken. Deze wordt vermenigvuldigd met een bepaalde factor gedeeld door 10, deze factor is in te stellen in de app.
    double dAngleDifference = absolute(iPanSpeed) * (PANSPEEDFACTOR / 10);

    //Wanneer de output al bijna bij 0 zit of juist bijna bij de max aantal DMX, verander niet meer
    if(dPanOutput > 1 || dPanOutput < PANDMXMAX - 1)
    {
        //Als de snelheid negatief is, trek het verschil eraf, bij positief doe het erbij
        if(iPanSpeed < 0)
        {
            dPanOutput -= dAngleDifference;
        }
        else
        {
            dPanOutput += dAngleDifference;
        }
    }
    
    //Bereken de output voor de 2e pan waarde, dit is het decimaal van de panoutput gemapt tot 255
    dPanOutputFine = maptonumber(dPanOutput - (int) dPanOutput, 1, 255);

    //Bekijk de output en map deze naar 0 tot 540, trek dan 270 eraf om hem tussen -270 en 270 te maken
    dPanAngle = maptonumber(dPanOutput, 255, 540) -  270;
}