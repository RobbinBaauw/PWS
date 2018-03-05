#include <doDrive.h>
#include <globalvariables.h>
#include <utils.h>
#include <element.h>

//Functie die elke DRIVEINTERVAL wordt aangeroepen wanneer de joystick niet wordt gebruikt.
//Checkt hoever de dolly van zijn doel af zit en zodra het buiten een bepaalde afstand is, laat hem rijden op een berekende snelheid
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
    unsigned long& lPreviousPositionUpdate
    )
{
    double dCurrentMetersDone = calculateMetersDone(
        dCurrentMeters,
        dCurrentRadian,
        eElementList,
        iCurrentElement
    );
    
    //Bereken in procent hoeveel van de totale rails is afgelegd door de dolly --> afgelegde lengte / totale lengte * 100
    double dCurrentPercent = dCurrentMetersDone / dTotalLength * 100;

    //Check of het verschil tussen de huidige procent en het doel procent groter is dan 1, dan kan er een direction worden veranderd
    if(absolute(dCurrentPercent - iDriveTargetPercent) > 2)
    {
        //Als het doel terug op de rails ligt, wordt direction 2, anders 1
        if(iDriveTargetPercent < dCurrentPercent)
        {
            iDirection = 2;
        }
        else
        {
            iDirection = 1;
        }
    }
    if(bGlobalLogs)
    {
        Serial.print("TotalLength: ");
        Serial.print(dTotalLength);
        Serial.print(" ");
        Serial.println(iDriveTargetPercent);
    }

	//Bereken hoever op de rails het ingestelde doel ligt
    double dTargetMeters = dTotalLength * (iDriveTargetPercent > 100 ? 100 : iDriveTargetPercent) / 100;

	//Als de richting 2 is, rijdt de dolly dus terug, en ligt het doel juist op de totale lengte - net berekende lengte
    if(iDirection == 2)
    {
        dTargetMeters = dTotalLength - dTargetMeters;
    }

	//Als de richting 2 is, rijdt de dolly dus terug, en ligt de dolly dus op de totale afstand - afgelegde afstand
    if(iDirection == 2)
    {
        dCurrentMetersDone = dTotalLength - dCurrentMetersDone;
    }
    
	//Als logs aan staan, log een aantal waardes voor debugging
    if(bGlobalLogs)
    {
        Serial.print("Targetmeters: ");
        Serial.print(dTargetMeters);
        Serial.print(" van ");
        Serial.print(dTotalLength);
        Serial.println(" meter");
    }

	//Bereken het verschil in afstand tussen het doel en de huidige positie van de dolly
    double dDistanceDelta = dTargetMeters - dCurrentMetersDone;

    if(bGlobalLogs)
    {
        Serial.print("Distance delta: ");
        Serial.println(dDistanceDelta);
    }

	//Als de net berekende afstand groter is dan een halve meter, run een stuk code dat het rijden aanzet
    if(dDistanceDelta > 0.5)
    {		
		//Zet het doel op het verschil in afstand
        dTargetDistance = dDistanceDelta;

        double dTargetRPM;

		//Als timelapse aanstaat, bereken de meter per seconde en daarna de doelRPM adhv de tijd die is ingesteld door de gebruiker
		//Doe het gedeeld door de DriveGearFactor, de dolly wordt namelijk versneld door middel van tandwielen en hoeft dus minder snel te draaien om een bepaalde snelheid te bereiken
        if(isTimelapse == 1)
        {
            double dMS = dDistanceDelta / iTimelapseTime;
            dTargetRPM = dMS / OMTREKWIEL * 60 / DRIVEGEARFACTOR;   
        }
		//Als timelapse niet aanstaat, bereken de RPM door middel van de gegeven KmH
        else
        {
            dTargetRPM = (dDriveTargetKmH / 3.6) / OMTREKWIEL * 60 / DRIVEGEARFACTOR;            
        }

        if(bGlobalLogs)
        {
            Serial.println("Turning driving on!");
            Serial.print("RPM: ");
            Serial.print(dTargetRPM);
            Serial.print(" ");
            Serial.println(dDriveTargetKmH);
        }

		//Zet driven aan
        isDriving = 1;

		//Zet de previous target procent op de huidige target percent, zodat later vegeleken kan worden of dit doel anders is
        iDriveTargetPrevPercent = iDriveTargetPercent;
        
		//Zet de laatste keer dat de dollypositie is berekend op dit moment (millis() vraagt aantal milliseconden sinds start programma)
        lPreviousPositionUpdate = millis();

		//Return de net berekende RPM, zodat die omgewerkt weg kan worden geschreven naar het motor shield
        return dTargetRPM;

    }

}

//Functie die elke DRIVEINTERVAL wordt aangeroepen wanneer de joystick wel wordt gebruikt.
double doDriveMotorJoystick(
    const double dDriveTargetKmHJoystick,
    int& iDirection,
    int& isDriving,
    unsigned long& lPreviousPositionUpdate,
    const double dCurrentMeters,
    const double dCurrentRadian,
    const Element eElementList[10],
    const int iCurrentElement
    )
{
	//Kijken of het een positieve of negatieve waarde is, en bepaal aan de hand daarvan de direction
    if(dDriveTargetKmHJoystick > 0)
    {
        iDirection = 1;
    }
    else
    {
        iDirection = 2;
    }

	//Check of het volgende element een lengte heeft van 0 (dus niet bestaat, zoja, dan weet je dat het bijna bij het einde van de rails zit)
    bool bIsNearEnd = iDirection == 1 ? (eElementList[iCurrentElement + 1].getLength() == 0 ? true : false) : (eElementList[iCurrentElement - 1].getLength() == 0 ? true : false);

    Element eCurrEl = eElementList[iCurrentElement];

    if(eCurrEl.getType() == 2 || eCurrEl.getType() == 3)
    {
		//Als het huidige element een cirkel is, de richting 1 is, op het laatste element zit (bIsNearEnd) en de afstand tussen de huidige radian op die cirkel en de eindradian kleiner dan 0.3 is, stop met rijden
        if(iDirection == 1 && bIsNearEnd && absolute(dCurrentRadian - eElementList[iCurrentElement].getEndRadian()) < 0.2)
        {
            Serial.println("Te kleine afstand");
            isDriving = 0;
            return 0;
        }
		//Als het huidige element een cirkel is, de richting 2 is, op het laatste element zit (bIsNearEnd) en de afstand tussen de huidige radian op die cirkel en de beginradian kleiner dan 0.3 is, stop met rijden
        else if(iDirection == 2 && bIsNearEnd && absolute(dCurrentRadian - eElementList[iCurrentElement].getBeginRadian()) < 0.2)
        {
            Serial.println("Te kleine afstand");
            isDriving = 0;
            return 0;
        }
    }
    else
    {
		//Als het huidige element een rechte lijn is, de richting 1 is, het op het laatste element is en de afstand tussen de huidige meters afgelegd op die lijn en de lengte van die lijn kleiner is dan 0.3, stop met rijden
        if(iDirection == 1 && bIsNearEnd && absolute(dCurrentMeters - eElementList[iCurrentElement].getLength()) < 0.3)
        {
            Serial.println("Te kleine afstand");
            isDriving = 0;
            return 0;
        }
		//Als het huidige element een rechte lijn is, de richting 2 is, het op het laatste element is en de afstand tussen de huidige meters afgelegd op die lijn en het nulpun kleiner is dan 0.3, stop met rijden
        else if(iDirection == 2 && bIsNearEnd && dCurrentMeters < 0.3)
        {
            Serial.println("Te kleine afstand");
            isDriving = 0;
            return 0;
        }
    }

	//Bereken het doel van de RPM, aan de hand van wat de joystick aangeeft in kilometer per uur
    double dTargetRPM = (absolute(dDriveTargetKmHJoystick) / 3.6) / OMTREKWIEL * 60 / DRIVEGEARFACTOR;

	//Zet driven aan
	isDriving = 1;
	
	//Zet de laatste keer dat de dollypositie is berekend op dit moment (millis() vraagt aantal milliseconden sinds start programma)
	lPreviousPositionUpdate = millis();

	//Return de RPM, zodat deze later gebruikt kan worden om naar het motorshield te sturen
    return dTargetRPM;
}

double calculateMetersDone(
    const double dCurrentMeters,
    const double dCurrentRadian,
    const Element eElementList[10],
    const int iCurrentElement
)
{
    double dCurrentMetersDone;

    //Loopt door de hele lijst met alle elementen tot het element waar je nu bent.
    for(int i = 0; i < iCurrentElement + 1; i++)
    {
        //Als het huidige element in de loop niet gelijk is aan het element waar de dolly zich bevindt, dus de dolly is verder dan het element in de loop, voeg de lengte van dat element toe aan de afstand die tot nu toe is afgelegd
        if(i != iCurrentElement)
        {
            dCurrentMetersDone += eElementList[i].getLength();
        }
        else
        {
            Element eCurrentElement = eElementList[iCurrentElement];
            if(eCurrentElement.getType() == 2 || eCurrentElement.getType() == 3)
            {
                //Als het huidige element gelijk is aan het element van de dolly en de dolly rijdt op een cirkel, bereken de radialen die de dolly op dat element heeft afgelegd
                double dNettoRadian = dCurrentRadian - eCurrentElement.getBeginRadian();

                //Er moet berekend worden hoever de dolly heeft gereden om tot die radian te komen, dat is het verschil is radialen keer de radius van de cirkel
                //Zo kom je daar op: 2 * pi * r = omtrek, in dit geval heb je dNettoRadian / 2pi procent van de cirkel gereden, dus dNettoRadian * r = gereden omtrek
                double dLengthToCurrentRadian = absolute(dNettoRadian * eCurrentElement.getRadius());
                
                //Voeg de net berekende lengte aan de gereden meters toe
                dCurrentMetersDone += dLengthToCurrentRadian;
            }
            else if(eCurrentElement.getType() == 1)
            {
                //Als het huidige element gelijk is aan het element van de dolly en de dolly rijdt op een rechte lijn, voeg gewoon de currentmeters eraan toe
                dCurrentMetersDone += dCurrentMeters;
            }
        }
    } 

    return dCurrentMetersDone;
}

//Deze functie wordt elke DRIVEINTERVAL aangeroepen wanneer de dolly aan het rijden is en de joystick niet wordt gebruikt
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
    const int iDriveTargetPrevPercent
)
{    
	//Bereken de nieuwe RPM (mocht de input veranderd zijn)
    dDriveRPMTarget = (dDriveTargetKmH / 3.6) / OMTREKWIEL * 60 / DRIVEGEARFACTOR;

	//Bekijk hoe lang het geleden is dat deze code is uitgevoerd, voor precieze berekeningen
    unsigned long lMillisDiff = millis() - lPreviousPositionUpdate;
	
	//Kijk hoever er in die tijd gereden is aan de hand van de RPM die binnenkomt vanaf de andere Arduino
	//Dit proces is redelijk precies omdat deze code behoorlijk vaak wordt uitgevoerd
	//Het werkt zo: je kijkt hoeveel seconden het geleden is, je kijkt hoeveel rondes per seconde het wiel draait, vermenigvuldigt die met elkaar (dan heb je het aantal rondjes dat is rondgedraaid), en vermenigvuligt dat met de omtrek van het wiel
    float dMetersDriven = (lMillisDiff / 1000.0) * (dDriveRPMIncoming / 60) * OMTREKWIEL;

    if(bGlobalLogs)
    {
        Serial.print("dMetersDriven: ");
        Serial.print(dMetersDriven);
        Serial.print(" ");
        Serial.println(dTargetDistance);
    }

	//Haal de gereden afstand van het doel af
    dTargetDistance -= dMetersDriven;

	//Stel de tijd in dat voor het laatst een positieupdate is gegeven
    lPreviousPositionUpdate = millis();

	//Bereken de nieuwe dollypositie
    calculateDollyPosition(
        dCurrentRadian,
        dCurrentMeters,
        iCurrentElement,
        eElementList,
        iDirection,
        dXDolly,
        dYDolly,
        true
    );

    //Bekijk of de dolly zich nu op een nieuw element bevindt
	checkNewElement(
		dCurrentRadian,
        dCurrentMeters,
        iCurrentElement,
        eElementList,
        iDirection,
		dMetersDriven
	);
	

	//Als de afstand tot het doel kleiner is dan 20cm, stop met rijden
    if(dTargetDistance < 0.2)
    {
        Serial.println("Te kleine afstand");
        isTimelapse = 0;
        isDriving = 0;
        return 0;
    }

	//Bereken het verschil in RPM tussen de inkomenden en het doel
    double dDiffRPM = absolute(dDriveRPMIncoming - dDriveRPMTarget);

    if(bGlobalLogs)
    {       
        Serial.print("CurrentElement: ");
        Serial.println(iCurrentElement);
    }

	//Als het verschil in RPM groter dan 2 is, corrigeer het en return het, is het niet groter dan twee, return het huidige DriveRPM target
    if(dDiffRPM > 10)
    {
        if(bGlobalLogs)
        { 
            Serial.println("Wrong RPM!");
        }

        if(dDriveRPMIncoming > dDriveRPMTarget)
        {
            return dDriveRPMTarget * DRIVEOUTPUTCORRECTION - 5;
        }
        else
        {
            if(dDriveRPMTarget * DRIVEOUTPUTCORRECTION + 5 < 250)
            {
                return dDriveRPMTarget * DRIVEOUTPUTCORRECTION + 5;
            }
        }
    }
    else
    {
        return dDriveRPMTarget * DRIVEOUTPUTCORRECTION;
    }

}

//Deze functie wordt elke DRIVEINTERVAL aangeroepen wanneer de dolly aan het rijden is en de joystick wel wordt gebruikt
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
)
{    
	//Bereken de doelRPM, mocht dit zijn veranderd sinds de laatste run
    dDriveRPMTarget = (absolute(dDriveTargetKmHJoystick) / 3.6) / OMTREKWIEL * 60 / DRIVEGEARFACTOR;

	//Verander de positie afhankelijk van de joystick
    if(dDriveTargetKmHJoystick > 0)
    {
        iDirection = 1;
    }
    else
    {
        iDirection = 2;
    }

	//Bekijk hoe lang het geleden is dat deze code is uitgevoerd, voor precieze berekeningen
    unsigned long lMillisDiff = millis() - lPreviousPositionUpdate;
	
	//Kijk hoever er in die tijd gereden is aan de hand van de RPM die binnenkomt vanaf de andere Arduino
	//Dit proces is redelijk precies omdat deze code behoorlijk vaak wordt uitgevoerd
	//Het werkt zo: je kijkt hoeveel seconden het geleden is, je kijkt hoeveel rondes per seconde het wiel draait, vermenigvuldigt die met elkaar (dan heb je het aantal rondjes dat is rondgedraaid), en vermenigvuligt dat met de omtrek van het wiel
    float dMetersDriven = (lMillisDiff / 1000.0) * (dDriveRPMIncoming / 60) * OMTREKWIEL;

	//Bekijk of de dolly zich nu op een nieuw element bevindt
	checkNewElement(
		dCurrentRadian,
        dCurrentMeters,
        iCurrentElement,
        eElementList,
        iDirection,
		dMetersDriven
	);

    lPreviousPositionUpdate = millis();

    //Check of het volgende element een lengte heeft van 0 (dus niet bestaat, zoja, dan weet je dat het bijna bij het einde van de rails zit)
    bool bIsNearEnd = iDirection == 1 ? (eElementList[iCurrentElement + 1].getLength() == 0 ? true : false) : (eElementList[iCurrentElement - 1].getLength() == 0 ? true : false);

    Element eCurrEl = eElementList[iCurrentElement];

    if(eCurrEl.getType() == 2 || eCurrEl.getType() == 3)
    {
		//Als het huidige element een cirkel is, de richting 1 is, op het laatste element zit (bIsNearEnd) en de afstand tussen de huidige radian op die cirkel en de eindradian kleiner dan 0.3 is, stop met rijden
        if(iDirection == 1 && bIsNearEnd && absolute(dCurrentRadian - eElementList[iCurrentElement].getEndRadian()) < 0.3)
        {
            Serial.println("Te kleine afstand");
            isDriving = 0;
            return 0;
        }
		//Als het huidige element een cirkel is, de richting 2 is, op het laatste element zit (bIsNearEnd) en de afstand tussen de huidige radian op die cirkel en de beginradian kleiner dan 0.3 is, stop met rijden
        else if(iDirection == 2 && bIsNearEnd && absolute(dCurrentRadian - eElementList[iCurrentElement].getBeginRadian()) < 0.3)
        {
            Serial.println("Te kleine afstand");
            isDriving = 0;
            return 0;
        }
    }
    else
    {
		//Als het huidige element een rechte lijn is, de richting 1 is, het op het laatste element is en de afstand tussen de huidige meters afgelegd op die lijn en de lengte van die lijn kleiner is dan 0.3, stop met rijden
        if(iDirection == 1 && bIsNearEnd && absolute(dCurrentMeters - eElementList[iCurrentElement].getLength()) < 0.3)
        {
            Serial.println("Te kleine afstand");
            isDriving = 0;
            return 0;
        }
		//Als het huidige element een rechte lijn is, de richting 2 is, het op het laatste element is en de afstand tussen de huidige meters afgelegd op die lijn en het nulpun kleiner is dan 0.3, stop met rijden
        else if(iDirection == 2 && bIsNearEnd && dCurrentMeters < 0.3)
        {
            Serial.println("Te kleine afstand");
            isDriving = 0;
            return 0;
        }
    }
	
	//Bereken de nieuwe dollypositie
    calculateDollyPosition(
        dCurrentRadian,
        dCurrentMeters,
        iCurrentElement,
        eElementList,
        iDirection,
        dXDolly,
        dYDolly,
        true
    );

    double dCurrentMetersDone = calculateMetersDone(
        dCurrentMeters,
        dCurrentRadian,
        eElementList,
        iCurrentElement
    );

    //Bereken in procent hoeveel van de totale rails is afgelegd door de dolly --> afgelegde lengte / totale lengte * 100
    iDriveTargetPercent = (int) (dCurrentMetersDone / dTotalLength * 100);

	//Bereken het verschil in RPM tussen de inkomenden en het doel
    double dDiffRPM = absolute(dDriveRPMIncoming - dDriveRPMTarget);

    if(bGlobalLogs)
    {       
        Serial.print("CurrentElement: ");
        Serial.println(iCurrentElement);
    }

	//Als het verschil in RPM groter dan 2 is, corrigeer het en return het, is het niet groter dan twee, return het huidige DriveRPM target
    if(dDiffRPM > 10)
    {
        if(bGlobalLogs)
        { 
            Serial.println("Wrong RPM!");
        }

        if(dDriveRPMIncoming > dDriveRPMTarget)
        {
            return dDriveRPMTarget * DRIVEOUTPUTCORRECTION - 5;
        }
        else
        {
            if(dDriveRPMTarget * DRIVEOUTPUTCORRECTION + 5 < 250)
            {
                return dDriveRPMTarget * DRIVEOUTPUTCORRECTION + 5;
            }
        }
    }
    else
    {
        return dDriveRPMTarget * DRIVEOUTPUTCORRECTION;
    }

}

void checkNewElement(
    double& dCurrentRadian, 
    double& dCurrentMeters, 
    int& iCurrentElement, 
    const Element eElementList[10], 
    const int iDirection,
	const double dMetersDriven
)
{
	Element eCurrEl = eElementList[iCurrentElement];

	//Dit deel code kijkt of de dolly aan een nieuw element is begonnen en of de dolly dus een nieuwe iCurrentElement moet krijgen
    if(eCurrEl.getType() == 1)
    {
        if(iDirection == 1)
        {
			//Bereken de afstand tot het einde van het huidige element, na het gereden stukje
			double dDistanceToEnd = eCurrEl.getLength() - dCurrentMeters - dMetersDriven;
			
			//Als er gebruik wordt gemaakt van een rechte lijn én de riching 1 is:
			//Kijk of de afstand tot het einde kleiner is dan 0, oftewel het is in een nieuw element gegaan (element verder)
            if(dDistanceToEnd < 0)
            {
				//Zoja, voeg 1 toe aan het huidige element
                Serial.println("Upgrading CurrentElement!");
                iCurrentElement += 1;

                if(eElementList[iCurrentElement].getType() == 1)
                {
					//Als het nieuwe element een rechte lijn is, maak de currentmeters het aantal dat de afstand tot het einde onder 0 kwam, dus de afstand die is afgelegd op het nieuwe element
                    dCurrentMeters = absolute(dDistanceToEnd);
                    dCurrentRadian = 0;
                }
                else
                {
					//2 * pi * r = omtrek, omgebouwd naar hoe het nu nodig is is dat afgelegde radian / 2pi * 2pi * r = afgelegde afstand
					//Dit geeft afgelegde radian * r = afgelegde afstand, afgelegde radian is afgelegde afstand / r
					double dDrivenRadian = absolute(dDistanceToEnd) / eElementList[iCurrentElement].getRadius();
					
					//Als de beginradian groter is dan de eindradian, haal de net berekende gereden radian van de beginradian af, voeg het anders toe
                    dCurrentRadian = eElementList[iCurrentElement].getBeginRadian();
                    dCurrentRadian += (eElementList[iCurrentElement].getBeginRadian() > eElementList[iCurrentElement].getEndRadian() ? -absolute(dDrivenRadian) : absolute(dDrivenRadian));
                    dCurrentMeters = 0;
                }
            }
            else
            {
				//Als de afstand tot het einde van het huidige element groter is dan 0, voeg de gereden afstand toe aan dCurrentMeters
                dCurrentMeters += dMetersDriven;
            }  
        }
        else
        {
			//Bereken de afstand tot het einde van het huidige element, na het gereden stukje
			double dDistanceToEnd = dCurrentMeters - dMetersDriven;
			
			//Als er gebruik wordt gemaakt van een rechte lijn én de riching 2 is:
			//Kijk of de afstand tot het einde kleiner is dan 0, oftewel het is in een nieuw element gegaan (element terug)
            if(dDistanceToEnd < 0)
            {
				//Zoja, haal 1 af aan het huidige element
                Serial.println("Downgrading CurrentElement!");
                iCurrentElement -= 1;

                if(eElementList[iCurrentElement].getType() == 1)
                {
					//Als het nieuwe element een rechte lijn is, maak de currentmeters de lengte van het nieuwe element - de afstand dat in het nieuwe element is afgelegd
                    dCurrentMeters = eElementList[iCurrentElement].getLength() - absolute(dDistanceToEnd);
                    dCurrentRadian = 0;
                }
                else
                {
					//2 * pi * r = omtrek, omgebouwd naar hoe het nu nodig is is dat afgelegde radian / 2pi * 2pi * r = afgelegde afstand
					//Dit geeft afgelegde radian * r = afgelegde afstand, afgelegde radian is afgelegde afstand / r
					double dDrivenRadian = absolute(dDistanceToEnd) / eElementList[iCurrentElement].getRadius();
					
					//Als de eindradian groter is dan de beginradian, haal de net berekende gereden radian van de eindradian af, voeg het anders toe
					dCurrentRadian = eElementList[iCurrentElement].getEndRadian();
                    dCurrentRadian += (eElementList[iCurrentElement].getEndRadian() > eElementList[iCurrentElement].getBeginRadian() ? -absolute(dDrivenRadian) : absolute(dDrivenRadian));
                    dCurrentMeters = 0;
                }
            }  
            else
            {
				//Als de afstand tot het einde van het huidige element groter is dan 0, haal dan de gereden afstand af van dCurrentMeters
                dCurrentMeters -= dMetersDriven;
            }  
        }
    }
    else
    {
        if(iDirection == 1)
        {
			//Bereken de afstand tot het einde van het huidige element, na het gereden stukje
            double dDistanceToEnd = eCurrEl.getLength() - (absolute(eElementList[iCurrentElement].getBeginRadian() - dCurrentRadian) * eElementList[iCurrentElement].getRadius()) - dMetersDriven;
			
            if(eCurrEl.getEndRadian() > eCurrEl.getBeginRadian())
            {
				//2 * pi * r = omtrek, omgebouwd naar hoe het nu nodig is is dat afgelegde radian / 2pi * 2pi * r = afgelegde afstand
				//Dit geeft afgelegde radian * r = afgelegde afstand, afgelegde radian is afgelegde afstand / r
				//Voeg deze afstand aan de currentRadian toe wanneer de endradian groter is dan de beginradian, en haal het ervanaf wanneer andersom het geval is
                dCurrentRadian += dMetersDriven / eCurrEl.getRadius();
            }
            else
            {
                dCurrentRadian -= dMetersDriven / eCurrEl.getRadius();
            }

			//Als er gebruik wordt gemaakt van een cikel én de riching 1 is:
			//Kijk of de huidige radian tussen de beginradian en de endradian in ligt, zonee, dan is het dus naar een volgend element
            if(!isBetweenTwoPoints(eCurrEl.getBeginRadian(), eCurrEl.getEndRadian(), dCurrentRadian))
            {
                Serial.println("Upgrading CurrentElement!");
                iCurrentElement += 1;

                if(eElementList[iCurrentElement].getType() == 1)
                {
					//Als het nieuwe element een rechte lijn is, maak de currentmeters de lengte van het nieuwe element - de afstand dat in het nieuwe element is afgelegd
                    dCurrentMeters = eElementList[iCurrentElement].getLength() - absolute(dDistanceToEnd);
                    dCurrentRadian = 0;
                }
                else
                {
					//2 * pi * r = omtrek, omgebouwd naar hoe het nu nodig is is dat afgelegde radian / 2pi * 2pi * r = afgelegde afstand
					//Dit geeft afgelegde radian * r = afgelegde afstand, afgelegde radian is afgelegde afstand / r
					double dDrivenRadian = absolute(dDistanceToEnd) / eElementList[iCurrentElement].getRadius();
					Serial.println(dDistanceToEnd);
                    Serial.println(dDrivenRadian);
                    Serial.println(eElementList[iCurrentElement].getBeginRadian());
					//Als de eindradian groter is dan de beginradian, haal de net berekende gereden radian van de eindradian af, voeg het anders toe
					dCurrentRadian = eElementList[iCurrentElement].getBeginRadian();
                    dCurrentRadian += (eElementList[iCurrentElement].getEndRadian() > eElementList[iCurrentElement].getBeginRadian() ? -absolute(dDrivenRadian) : absolute(dDrivenRadian));
                    dCurrentMeters = 0;
                }
            }
        }
        else
        {
			//Bereken de afstand tot het einde van het huidige element, na het gereden stukje
			double dDistanceToEnd = eCurrEl.getLength() - (absolute(eElementList[iCurrentElement].getEndRadian() - dCurrentRadian) * eElementList[iCurrentElement].getRadius()) - dMetersDriven;
			
			if(eCurrEl.getEndRadian() > eCurrEl.getBeginRadian())
            {
				//2 * pi * r = omtrek, omgebouwd naar hoe het nu nodig is is dat afgelegde radian / 2pi * 2pi * r = afgelegde afstand
				//Dit geeft afgelegde radian * r = afgelegde afstand, afgelegde radian is afgelegde afstand / r
				//Haa; deze afstand van de currentRadian af wanneer de endradian groter is dan de beginradian, en voeg het toe wanneer andersom het geval is
                dCurrentRadian -= dMetersDriven / eCurrEl.getRadius();
            }
            else
            {
                dCurrentRadian += dMetersDriven / eCurrEl.getRadius();
            }
			
			//Als er gebruik wordt gemaakt van een cikel én de riching 2 is:
			//Kijk of de huidige radian tussen de beginradian en de endradian in ligt, zonee, dan is het dus naar een volgend element
            if(!isBetweenTwoPoints(eCurrEl.getBeginRadian(), eCurrEl.getEndRadian(), dCurrentRadian))
            {
                Serial.println("Downgrading CurrentElement!");
                iCurrentElement -= 1;

                if(eElementList[iCurrentElement].getType() == 1)
                {
					//Als het nieuwe element een rechte lijn is, maak de currentmeters de lengte van het nieuwe element - de afstand dat in het nieuwe element is afgelegd
                    dCurrentMeters = eElementList[iCurrentElement].getLength() - absolute(dDistanceToEnd);
                    dCurrentRadian = 0;
                }
                else
                {
					//2 * pi * r = omtrek, omgebouwd naar hoe het nu nodig is is dat afgelegde radian / 2pi * 2pi * r = afgelegde afstand
					//Dit geeft afgelegde radian * r = afgelegde afstand, afgelegde radian is afgelegde afstand / r
					double dDrivenRadian = absolute(dDistanceToEnd) / eElementList[iCurrentElement].getRadius();
					
					//Als de eindradian groter is dan de beginradian, haal de net berekende gereden radian van de eindradian af, voeg het anders toe
					dCurrentRadian = eElementList[iCurrentElement].getEndRadian();
                    dCurrentRadian += (eElementList[iCurrentElement].getEndRadian() > eElementList[iCurrentElement].getBeginRadian() ? -absolute(dDrivenRadian) : absolute(dDrivenRadian));
                    dCurrentMeters = 0;
                }
            }
        }
    }
}