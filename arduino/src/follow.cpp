#include <follow.h>
#include <element.h>
#include <globalvariables.h>
#include <utils.h>
#include <Arduino.h>
#include <Math.h>
#include <doDrive.h>

//Returnt de hoek die de movinghead moet maken
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
    const int iDoAlter)
{

    double dSlopeDolly;
     
    //Berekent de nieuwe posities van de dolly, krijgt ook meteen de slope door welke de dolly heeft (richting waar het heen kijkt in de wereld)
    calculateDollyPosition(
        dCurrentRadian,
        dCurrentMeters,
        iCurrentElement,
        eElementList,
        iDirection,
        dXDolly,
        dYDolly,
        dSlopeDolly,
        false
    );

    double dFurtherXDolly;
    double dFurtherYDolly;

    //Berekent de positie van de dolly een klein stukje verder, om te kijken welke richting de dolly heen kijkt (de SlopeDolly moet worden gecorrigeerd, het geeft bv 30 graden maar de echte hoek is 30-180 = -150 graden)
    getFurtherDollyPosition(
        dFurtherXDolly,
        dFurtherYDolly,
        iCurrentElement,
        eElementList,
        iDirection,
        dCurrentRadian,
        dCurrentMeters,
        false
    );

    //Corrigeert de DollySlope
    CorrectDollySlope(
        dXDolly,
        dYDolly,
        dFurtherXDolly,
        dFurtherYDolly,
        eElementList,
        iCurrentElement,
        dSlopeDolly
    );

    //Berekent het verschil tussen de X en Y coordinaten van de dolly en het focuspunt
    double dXFocusDifference = dXFocus - dXDolly;
    double dYFocusDifference = dYFocus - dYDolly;

    //Berkent de afstand tussen de dolly en het focuspunt
    double dDistanceFocusDolly = sqrt(dXFocusDifference * dXFocusDifference + dYFocusDifference * dYFocusDifference);

    //Berekent de hoek tussen de dolly en het focuspunt (zie tekeningen)
    double dAngleToFocus = asin(dYFocusDifference / dDistanceFocusDolly) * RADIANDEGREES;

    //asin(1/6pi) == asin(5/6pi), maar asin zal alleen de 1/6pi returnen, hiervoor moet gecorrigeerd worden, wanneer het focuspunt links ligt van de dolly doe je daarom 180 - dAngleToFocus
    if(dXFocusDifference < 0)
    {
        dAngleToFocus = 180 - dAngleToFocus;
    }

    //Berekent de hoek die gedraaid moet worden, dat is hoek waar de dolly heen kijkt, - (slopeDolly - 90), zie tekening
    double dPanAngle = dAngleToFocus - (dSlopeDolly - 90);

    if(iDirection == 2)
    {
        dPanAngle -= 180;
    }

    //Soms moet de panangle 360 erbij of eraf krijgen omdat ergens op de gehele route de hoek die gedraaid moet worden over de 270 heengaat of onder de -270 zit. Dit wordt eerder bepaald en deze correctie wordt hier toegepast
    if(iDoAlter == -1)
    {
        //DoAlter == -1 betekent -360, maar als de hoek -360 ook een illegale waarde oplevert, toch maar niet doen
        if(dPanAngle - 360 < -270)
        {
            return dPanAngle;
        }
        else
        {
            return dPanAngle - 360;
        } 
    }
    else if(iDoAlter == 1)
    {
        //DoAlter == 1 betekent 360, maar als de hoek +360 ook een illegale waarde oplevert, toch maar niet doen
        if(dPanAngle + 360 > 270)
        {
            return dPanAngle;
        }
        else
        {
            return dPanAngle + 360;
        }
    }
    else
    {
        //Doe niets, return het gewoon
        return dPanAngle;
    }
}

//Functie die de coordinaten berekent die de dolly over een hele kleine afstand zal hebben, om zo een goede slopedolly te kunnen berekenen
void getFurtherDollyPosition(
    double& dFurtherXDolly,
    double& dFurtherYDolly,
    const int iCurrentElement,
    const Element eElementList[10],
    const int iDirection,
    const double dCurrentRadian,
    const double dCurrentMeters,
    const bool dDoLog
)
{
    double dFurtherCurrentMeters;
    double dFurtherCurrentRadian;

    Element eCurrEl = eElementList[iCurrentElement];

    if(eCurrEl.getType() == 1)
    {
        //Als het huidige element een rechte lijn is en de richting 1 is, voeg een millimeter toe aan de currentmeters, bij richting 2 andersom
        if(iDirection == 1)
        {
            dFurtherCurrentMeters = dCurrentMeters + 0.001;
        }
        else
        {
            dFurtherCurrentMeters = dCurrentMeters - 0.001;
        }
    }
    else if(eCurrEl.getType() == 2 || eCurrEl.getType() == 3)
    {
        //Kijk ook hier of een klein beetje aan de currentradian moet worden toegevoegd of verwijderd, afhankelijk van de richting en of endradian groter of kleiner dan beginradian is
        if(iDirection == 1)
        {
            if(eCurrEl.getEndRadian() > eCurrEl.getBeginRadian())
            {
                dFurtherCurrentRadian = dCurrentRadian + 0.001;
            }
            else
            {
                dFurtherCurrentRadian = dCurrentRadian - 0.001;
            }
        }
        else
        {
            if(eCurrEl.getEndRadian() > eCurrEl.getBeginRadian())
            {
                dFurtherCurrentRadian = dCurrentRadian - 0.001;
            }
            else
            {
                dFurtherCurrentRadian = dCurrentRadian + 0.001;
            }
        }
    }

    //Bereken adhv deze net berekende gegevens de dollypositie van een fractie van een seconde verder
    calculateDollyPosition(
        dFurtherCurrentRadian,
        dFurtherCurrentMeters,
        iCurrentElement,
        eElementList,
        iDirection,
        dFurtherXDolly,
        dFurtherYDolly,
        dDoLog
    );
}

//Verkrijg de coordinaten van het punt waarop zal worden gefocust
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
    const double dTotalLength)
{
    double dSlopeDolly;
    
    //Bereken eerst de dolly positie, ook om de richting te verkrijgen waar de dolly naar kijkt
    calculateDollyPosition(
        dCurrentRadian,
        dCurrentMeters,
        iCurrentElement,
        eElementList,
        iDirection,
        dXDolly,
        dYDolly,
        dSlopeDolly,
        true
    );

    //Bereken ook hier weer de volgende dolly positie en corrigeer de slope
    double dFurtherXDolly;
    double dFurtherYDolly;

    getFurtherDollyPosition(
        dFurtherXDolly,
        dFurtherYDolly,
        iCurrentElement,
        eElementList,
        iDirection,
        dCurrentRadian,
        dCurrentMeters,
        true
    );

    CorrectDollySlope(
        dXDolly,
        dYDolly,
        dFurtherXDolly,
        dFurtherYDolly,
        eElementList,
        iCurrentElement,
        dSlopeDolly
    );

    //Bereken de graden naar de focus door de slope van de dolly op te tellen bij de hoek die is gedraaid
    //Haal daar 90 graden van af omdat naar rechts kijkend 0 graden is
    double dAngleToFocus = dSlopeDolly + dPanAngle - 90;

    //Bereken het verschil in X en Y door middel van de net berekende hoek en de ingevoerde afstand tot het focus punt
    double dXFocusDifference = cos(dAngleToFocus * DEGREERADIAN) * iDistanceFocusPoint;
    double dYFocusDifference = sin(dAngleToFocus * DEGREERADIAN) * iDistanceFocusPoint;

    //Tel de dolly positie en het verschi in X en Y bij elkaar op voor de uiteindelijke focuspositie
    dXFocus = dXDolly + dXFocusDifference;
    dYFocus = dYDolly + dYFocusDifference;

    Serial.print("X, Y focus: ");
    Serial.print(dXFocus);
    Serial.print(" ");
    Serial.println(dYFocus);

    Element eCurrEl = eElementList[iCurrentElement];    

    //In dit stuk zal de hele route over de gehele rails worden gesimuleerd en worden gekeken hoe er per coordinaat gedraaid moet worden. Aangezien de movinghead een range heeft van -270 tot 270 graden, mag de berekende graad daar niet buiten zitten.
    //Wanneer dit getal daar buiten valt wordt -1 (doe telkens -360 graden) of 1 (doe telkens +360 graden) gereturned
    int iDoAlter = 0;
    int iTestDirection = 1;
    int iCurrentTestElement = 0;
    double dMetersDriven = 5.0 / 100 * dTotalLength;

    double dCurrentTestMeters = 0;
    double dCurrentTestRadian;

    //Als je begint met een cirkel, zet de testradian als de beginradian van het eerste element
    if(eElementList[0].getType() == 2 || eElementList[0].getType() == 3)
    {
        dCurrentTestRadian = eElementList[0].getBeginRadian();
    }

    //Ga de 100% in stukken van 5 procent af, het moet niet teveel performance kosten natuurlijk
    for(int z = 0; z < 100; z = z + 5)
    {   
        //Call de functie die de dolly een klein stuk laat rijden om zo de dCurrentTestRadian en de dCurrentTestMeters te berekenen
        checkNewElement(
            dCurrentTestRadian,
            dCurrentTestMeters,
            iCurrentTestElement,
            eElementList,
            iTestDirection,
            dMetersDriven
        );

        double dCurrentDollyX;
        double dCurrentDollyY;

        //Verkrijg de panangle met de net berekende currentradian of currentmeters
        double dPanAngle = dollyVolg(
            dCurrentTestRadian,
            dCurrentTestMeters,
            dXFocus,
            dYFocus,
            dCurrentDollyX,
            dCurrentDollyY,
            iCurrentTestElement,
            eElementList,
            iTestDirection,
            iDistanceFocusPoint,
            0);

        //Wanneer de panangle boven de 270 of onder de -270 komt, zet de DoAlter en ga uit de loop
        if(dPanAngle > 270)
        {
            iDoAlter = -1;
            break;
        }
        else if(dPanAngle < -270)
        {
            iDoAlter = 1;
            break;
        }
    }

    iTestDirection = 2;
    Serial.println("Swapping direction test");

    //Draai de richting om (rij dus terug), en voer exact hetzelfde uit
    for(int z = 100; z > 0; z = z - 5)
    {
        checkNewElement(
            dCurrentTestRadian,
            dCurrentTestMeters,
            iCurrentTestElement,
            eElementList,
            iTestDirection,
            dMetersDriven
        );

        double dCurrentDollyX;
        double dCurrentDollyY;

        double dPanAngle = dollyVolg(
            dCurrentTestRadian,
            dCurrentTestMeters,
            dXFocus,
            dYFocus,
            dCurrentDollyX,
            dCurrentDollyY,
            iCurrentTestElement,
            eElementList,
            iTestDirection,
            iDistanceFocusPoint,
            0);

        if(dPanAngle > 270)
        {
            iDoAlter = -1;
            break;
        }
        else if(dPanAngle < -270)
        {
            iDoAlter = 1;
            break;
        }
    }
    
    Serial.print("Do alter: ");
    Serial.println(iDoAlter);

    return iDoAlter;
}

//Hier wordt gekeken of de dollyslope gecorrigeerd moet worden, er wordt namelijk een slope meegegeven waar een hoek uitgehaald wordt
//Deze hoek klopt soms niet, omdat een hoek van links naar beneden aan wordt gegeven als een positieve hoek (rechts omhoog), omdat dat op een lijn ligt
void CorrectDollySlope(
    const double dXDolly,
    const double dYDolly,
    const double dFurtherXDolly,
    const double dFurtherYDolly,
    const Element eElementList[10], 
    const int iCurrentElement,
    double& dSlopeDolly 
)
{
    Element eCurrEl = eElementList[iCurrentElement];    

    //Wanneer de X < Xmiddle en de Y > Ymiddle, is het dus het kwadrant linksbovenin de cirkel
    if(dXDolly < eCurrEl.getXMiddle() && dYDolly > eCurrEl.getYMiddle())
    {
        //De dolly rijdt rechtsom over de cirkel
        if(dYDolly < dFurtherYDolly && dXDolly < dFurtherXDolly)
        {
            //Geen correctie nodig, pak gewoon de tangens
            dSlopeDolly = atan(dSlopeDolly) * RADIANDEGREES;
        }
        //De dolly rijdt linksom over de cirkel
        else
        {
            //Naar beneden moet worden gecorrigeerd, dus voeg 180 graden en zorg dat het binnen de 360 blijft
            //Van het berekende getal wordt de modulo gepakt, oftewel het berekende getal / 360, de 'rest' wordt berekend
            dSlopeDolly = fmod(atan(dSlopeDolly) * RADIANDEGREES + 180, 360);
        }
    }
    //Wanneer de X > Xmiddle en de Y > Ymiddle, is het dus het kwadrant rechtsbovenin de cirkel
    else if(dXDolly > eCurrEl.getXMiddle() && dYDolly > eCurrEl.getYMiddle())
    {
        //De dolly rijdt rechtsom over de cirkel
        if(dYDolly > dFurtherYDolly && dXDolly < dFurtherXDolly)
        {
            //Geen correctie nodig
            dSlopeDolly = atan(dSlopeDolly) * RADIANDEGREES;
        }
        //De dolly rijdt linksom over de cirkel
        else
        {
            //Wel correctie nodig
            dSlopeDolly = fmod(atan(dSlopeDolly) * RADIANDEGREES + 180, 360);
        }
    }
    //Wanneer de X < Xmiddle en de Y < Ymiddle, is het dus het kwadrant linksonderin de cirkel
    else if(dXDolly < eCurrEl.getXMiddle() && dYDolly < eCurrEl.getYMiddle())
    {
        //De dolly rijdt rechtsom over de cirkel
        if(dYDolly < dFurtherYDolly && dXDolly > dFurtherXDolly)
        {
            //Wel correctie nodig
            dSlopeDolly = fmod(atan(dSlopeDolly) * RADIANDEGREES + 180, 360);
        }
        //De dolly rijdt linksom over de cirkel
        else
        {
            //Geen correctie nodig
            dSlopeDolly = atan(dSlopeDolly) * RADIANDEGREES;
        }
    }
    //Wanneer de X > Xmiddle en de Y < Ymiddle, is het dus het kwadrant rechtsonderin de cirkel
    else if(dXDolly > eCurrEl.getXMiddle() && dYDolly < eCurrEl.getYMiddle())
    {
        //De dolly rijdt rechtsom over de cirkel
        if(dYDolly > dFurtherYDolly && dXDolly > dFurtherXDolly)            
        {
            //Wel correctie nodig
            dSlopeDolly = fmod(atan(dSlopeDolly) * RADIANDEGREES + 180, 360);
        }
        //De dolly rijdt linksom over de cirkel
        else
        {
            //Geen correctie nodig
            dSlopeDolly = atan(dSlopeDolly) * RADIANDEGREES;
        }
    }
}