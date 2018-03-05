#include <utils.h>
#include <element.h>

//x is getal dat moet worden gemapt, huidig is het tussen 0 en out_start gemapt, en out_finish wordt het uiteindelijke doel. Eerst bereken je hoe groot deel was van de huidige map, dan reken je door om het nu goed te mappen
double maptonumber(double x, double out_start, double out_finish) {
    return (out_finish / (out_start / x));
}

//De huidige abs functie werkte niet goed, dus daarom een eigen geschreven, heel erg simpel
double absolute(double x) {
    if(x < 0)
    {
        return -x;
    }
    else
    {
        return x;
    }
}

//Niet altijd hoeft een slopeAngle uitgerekend te worden tijdens het uitrekenen van de dolly positie. Het is dus ook mogelijk om geen slopeAngle mee te geven, dat wordt hierdoor geregeld
void calculateDollyPosition(
    const double dCurrentRadian, 
    const double dCurrentMeters, 
    const int iCurrentElement, 
    const Element eElementList[10], 
    const int iDirection,
    double& dXDolly,
    double& dYDolly,
    const bool bDoLogCoords
)
{
    double dSlopeDolly;

    calculateDollyPosition(
        dCurrentRadian,
        dCurrentMeters,
        iCurrentElement,
        eElementList,
        iDirection,
        dXDolly,
        dYDolly,
        dSlopeDolly,
        bDoLogCoords
    );
}

//Deze functie rekent de positie van de dolly uit
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
) 
{
    double dPrevXEnd;
    double dPrevYEnd;

    //De coordinaten waar vandaan we gaan rekenen, het beginpunt in weze
    dPrevXEnd = eElementList[iCurrentElement].getXBegin();
    dPrevYEnd = eElementList[iCurrentElement].getYBegin();

    Element eCurrEl = eElementList[iCurrentElement];

    double dXDifference;
    double dYDifference;

    //Als het huidige element een cirkel is
    if(eCurrEl.getType() == 2 || eCurrEl.getType() == 3)
    {
        if(dYDolly == eCurrEl.getYMiddle())
        {
            dSlopeDolly = 99999;
        }
        else
        {
            //Dit is de inverse van de lijn tussen de dolly en het midden van de cirkel waar de dolly omheen draait, de normale is (dYDolly - eCurrEl.getYMiddle()) / (dXDolly - eCurrEl.getXMiddle()), en de lijn recht daarop is deze:
            dSlopeDolly = -(dXDolly - eCurrEl.getXMiddle()) / (dYDolly - eCurrEl.getYMiddle());    
        }
        
        //Bereken het verschil van de dolly tov (0,0)
        dXDifference = cos(dCurrentRadian) * eCurrEl.getRadius();
        dYDifference = sin(dCurrentRadian) * eCurrEl.getRadius();

        //Je wilt het verschil hebben ten opzichte van het beginpunt, dus je haalt de coordinaten daarvan tov (0,0) eraf
        dXDifference -= (cos(eCurrEl.getBeginRadian()) * eCurrEl.getRadius());
        dYDifference -= (sin(eCurrEl.getBeginRadian()) * eCurrEl.getRadius());
    }
    //Als het huidige element een rechte lijn is
    else if(eCurrEl.getType() == 1)
    {
        //Bereken de hoek die de dolly maakt, eigenlijk gewoon de hoek die al wordt meegegeven naar een slope toe
        dSlopeDolly = tan(eCurrEl.getAngleToRadian());

        //Bereken het verschil in coordinaten vanaf (0,0)
        dXDifference = cos(eCurrEl.getAngleToRadian()) * dCurrentMeters;
        dYDifference = sin(eCurrEl.getAngleToRadian()) * dCurrentMeters;
    }

    // Bereken het nieuwe coordinaat van de dolly adhv de vorige xy en het verschil daarin
    dXDolly = dPrevXEnd + dXDifference;
    dYDolly = dPrevYEnd + dYDifference;

    if(bDoLogCoords)
    {
        Serial.print("X,Y: ");
        Serial.print(dXDolly);
        Serial.print(" , ");
        Serial.println(dYDolly);
    }
}

//Soms is het nodig om te kijken of een punt tussen twee punten ligt, deze functie doet dat
bool isBetweenTwoPoints(
    double dPointOne,
    double dPointTwo,
    double dCurrentPoint
) 
{
    if(dPointOne > dPointTwo)
    {
        if(dCurrentPoint <= dPointOne && dCurrentPoint >= dPointTwo)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if(dCurrentPoint >= dPointOne && dCurrentPoint <= dPointTwo)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}