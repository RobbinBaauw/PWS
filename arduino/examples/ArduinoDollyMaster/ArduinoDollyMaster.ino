#define BLYNK_PRINT Serial

// Importeer een aantal Arduino Libraries
#include <SPI.h>
#include <Ethernet2.h>
#include <BlynkSimpleEthernet2.h>
#include <SimpleTimer.h>
#include <DmxMaster.h>
#include <Wire.h>

// Importeer een aantal C++ Libraries
#include <math.h>
#include <stdlib.h>

// Importeer een aantal eigen Libraries
#include <doDrive.h>
#include <follow.h>
#include <globalvariables.h>
#include <utils.h>
#include <pantilt.h>
#include <Variables.h>

// Blynk auth code
char auth[] = "ba9b4890412e43139bbef0a6f571c847";

// Stel de netwerkinstellingen voor school in
// byte mac[] = {
//   0x00, 0x00, 0x00, 0x00
// };
// IPAddress dnServer(0, 0, 0,0 );
// IPAddress gateway(0, 0, 0, 0);
// IPAddress subnet(0, 0, 0, 0);
// IPAddress ip(0, 0, 0, 0);

// Maak een aantal timers die op een bepaalde interval zullen runnen
SimpleTimer timer;
SimpleTimer timer2;
SimpleTimer receivingWireTimer;
SimpleTimer receivingSerialTimer;
SimpleTimer sendingNodeDataTimer;
SimpleTimer sendingTerminalDataTimer;

// Defineer een aantal leds die in Blynk te zien zijn
WidgetLED trackled(V5);
WidgetLED trackled2(V15);

WidgetLED timelapseled(V6);
WidgetLED timelapseled2(V16);

WidgetTerminal terminal(V21);

// Pins voor de motor driver
int pwm_a = 9;
int dir_a = 8;

// Setup
void setup()
{
    // Start de communicatie via analoge poorten
    Wire.begin();

    // Reset de DMX
    DmxMaster.write(1, 0);
    DmxMaster.write(2, 0);
    DmxMaster.write(5, iPanSpeedFactor);

    // Zet de 2 PWM pins als output pins
    pinMode(pwm_a, OUTPUT);
    pinMode(dir_a, OUTPUT);

    // Schfijf 0 naar het motorshield (motor staat stil)
    analogWrite(pwm_a, 0);

    // Stel serial in om Blynk te gebruiken
    Serial.begin(9600);

    // Start Blynk op met het volgende IP adres
     Blynk.begin(auth, IPAddress(192,168,1,34), 8442);

    // Stel een aantal timers in die op een bepaalde interval gaan lopen
    timer.setInterval(STEPPERINTERVAL * 1000 - 100, intervalFunction);
    timer2.setInterval(DRIVEINTERVAL * 1000, intervalFunctionHalfSecond);
    receivingSerialTimer.setInterval(50, receiveSerial);
    receivingWireTimer.setInterval(DRIVEINTERVAL * 500, receiveWire);
    sendingNodeDataTimer.setInterval(10000, sendNodeData);
    sendingTerminalDataTimer.setInterval(5000, sendTerminalData);

    //Stel een aantal standaard waardes in 
    sReceivedData = "2_0,0,9.397,3.42,,,1,10,,20,,|9.397,3.42,5.042,15.385,7.22,9.402,2,20,6."
                    "366,-160,-1.222,1.92";

    iCurrentElement = 0;
    iDirection = 1;
    iTiltSpeedFactor = 10;
    iPanSpeedFactor = 20;
    dDriveTargetKmH = 2;
    bGlobalLogs = false;
    bFreeRoam = false;

    parseSerialData();
}

// Loop van Arduino zelf
void loop()
{
    // Call een functie van Blynk om connectie te behouden
    Blynk.run();

    // Run de timers
    timer.run();
    timer2.run();
    receivingWireTimer.run();
    sendingNodeDataTimer.run();
    sendingTerminalDataTimer.run();
    receivingSerialTimer.run();
}

//Stuur wat data naar de Node server wanneer logging aanstaat (niet getest / af)
void sendNodeData()
{
    if (bGlobalLogs)
    {
        String sAllDataCombined;
        sAllDataCombined += dXDolly;
        sAllDataCombined += ",";
        sAllDataCombined += dYDolly;
        sAllDataCombined += ",";
        sAllDataCombined += dPanAngle;
        sAllDataCombined += ",";
        sAllDataCombined += dDriveRPMIncoming;

        writeString(sAllDataCombined);
    }
}

//Print data naar de Blynk terminal wanneer logging aanstaat
void sendTerminalData()
{
    if (bGlobalLogs)
    {
        terminal.print("XY: ");
        terminal.print(dXDolly);
        terminal.print(" ");
        terminal.println(dYDolly);

        terminal.print("PanAngle: ");
        terminal.println(dPanAngle);

        terminal.print("CurrentElement: ");
        terminal.println(iCurrentElement);

        if (dCurrentMeters != 0)
        {
            terminal.print("CurrentMeters: ");
            terminal.println(dCurrentMeters);
        }
        else
        {
            terminal.print("CurrentRadian: ");
            terminal.println(dCurrentRadian);
        }
    }
}

//Schrijf data weg naar de serial wanneer deze request wordt gestuurd
void writeString(String stringData)
{
    Serial.print("Serial writing: ");
    for (int i = 0; i < stringData.length(); i++)
    {
        Serial.write(stringData[i]);
    }
    Serial.println("!");
}

//Stuur om een RPM input request van de andere Arduino en verwerk dit naar een double
void receiveWire()
{
    Wire.requestFrom(4, 6);

    while (Wire.available())
    {
        cRPMAnalogInput[iRPMAnalogIndex] = Wire.read();
        iRPMAnalogIndex++;
    }
    if (iRPMAnalogIndex > 0)
    {
        String sRPMAnalogNumber = String(cRPMAnalogInput);
        dDriveRPMIncoming = sRPMAnalogNumber.toDouble();
        iRPMAnalogIndex = 0;
    }
}

//Vervorm de rails string naar een Element
void parseSerialData()
{
    stopDolly();

    //Maak een array van 1 langer dan de lengte van de gehele string
    char chararray[sReceivedData.length() + 1];

    //Maak van de string een chararray
    sReceivedData.toCharArray(chararray, sReceivedData.length() + 1);

    //Bekijk het eerste deel van de string tot de _ bekijk zo hoeveel elementen het zijn en maak er een integer van
    String amountElements = "";
    amountElements += chararray[0];
    if (chararray[1] != '_')
    {
        amountElements += chararray[1];
    }

    int elementLength = amountElements.toInt();

    //Je hebt een 2d array, de eerste is even lang als het aantal elementen, en de tweede daarin heeft 12 properties, alle properties van de Element klasse
    String properties[elementLength][12];

    int currentproperty = 0;
    int currentelement = 0;

    //Ga van de string naar de properties 2d array
    for (int i = amountElements.length() + 1; i < sReceivedData.length(); i++)
    {
        if (chararray[i] == ',')
        {
            currentproperty += 1;
        }
        else if (chararray[i] == '|')
        {
            currentelement += 1;
            currentproperty = 0;
        }
        else
        {
            properties[currentelement][currentproperty] += chararray[i];
        }
    }

    //Vul de ElementList array
    for (int z = 0; z < elementLength; z++)
    {
        eElementList[z] = Element(properties[z][0].toFloat(), properties[z][1].toFloat(),
            properties[z][2].toFloat(), properties[z][3].toFloat(), properties[z][4].toFloat(),
            properties[z][5].toFloat(), properties[z][6].toInt(), properties[z][7].toFloat(),
            properties[z][8].toFloat(), properties[z][9].toFloat(), properties[z][10].toFloat(),
            properties[z][11].toFloat());
    }

    dTotalLength = 0;

    //Loop door de hele array en bepaal zo de lengte
    for (int i = 0; i < elementLength; i++)
    {
        if (eElementList[i].getLength() != 0)
        {
            dTotalLength += eElementList[i].getLength();
        }
    }

    Blynk.virtualWrite(V63, (int) dTotalLength);

    //Reset de string
    sReceivedData = "";

    //Reset alles tot positie 0 0
    iCurrentElement = 0;
    iDirection = 1;

    dCurrentRadian = eElementList[iCurrentElement].getType() == 1 ? 0 : eElementList[iCurrentElement].getBeginRadian();
    dCurrentMeters = 0;
    
    dXFocus = 0;
    dYFocus = 0;
}

//Lees de serial uit en parse deze data
void receiveSerial()
{
    char cReceivedChar = Serial.read();
    sReceivedData.concat(cReceivedChar);

    if (cReceivedChar == '\n')
    {
        Serial.println("Received n");

        parseSerialData();
    }
}

//Deze functie wordt elke halve seconde aangeroepen en dient voor het aandrijven van de motor
void intervalFunctionHalfSecond()
{
    if(bFreeRoam == true)
    {
        if(dDriveTargetKmHJoystick == 0)
        {
            analogWrite(pwm_a, 0);
        }
        else
        {
            if (dDriveTargetKmHJoystick > 0)
            {
                digitalWrite(dir_a, HIGH);
            }
            else
            {
                digitalWrite(dir_a, LOW);
            }

            Serial.println(dDriveTargetKmHJoystick);

            double dDriveOutputCurrent = absolute(dDriveTargetKmHJoystick / 3.6) / OMTREKWIEL * 60 / DRIVEGEARFACTOR * DRIVEOUTPUTCORRECTION;

            if (dDriveOutputCurrent > MAXSPEEDOUTPUT || dDriveOutputCurrent < MINSPEEDOUTPUT)
            {
                if (bGlobalLogs)
                {
                    Serial.print("Wrong speed: ");
                    Serial.println(dDriveOutputCurrent);
                }

                analogWrite(pwm_a, 0);
            }
            else
            {
                analogWrite(pwm_a, dDriveOutputCurrent);
            }
        }
    }
    else if(bFreeRoam == false)
    {
        //Als er niet gereden wordt en de joystick ook niet wordt gebruikt, roep de doDriveMotor functie aan en schrijf het resultaat weg naar de motor
        if (isDriving == 0 && dDriveTargetKmHJoystick == 0)
        {
            dDriveRPMTarget = doDriveMotor(dDriveTargetKmH, iDriveTargetPercent,
                iDriveTargetPrevPercent, dTotalLength, eElementList, iCurrentElement, dXDolly, dYDolly,
                dCurrentMeters, dCurrentRadian, dTargetDistance, iDirection, isDriving, isTimelapse,
                iTimelapseTime, lPreviousPositionUpdate);

            if (iDirection == 1)
            {
                digitalWrite(dir_a, HIGH);
            }
            else if (iDirection == 2)
            {
                digitalWrite(dir_a, LOW);
            }

            dDriveOutput = dDriveRPMTarget * DRIVEOUTPUTCORRECTION;

            if (dDriveOutput > MAXSPEEDOUTPUT || dDriveOutput < MINSPEEDOUTPUT)
            {
                if (bGlobalLogs)
                {
                    Serial.print("Wrong speed: ");
                    Serial.println(dDriveOutput);
                }

                analogWrite(pwm_a, 0);

                stopDolly();
            }
            else
            {
                analogWrite(pwm_a, dDriveOutput);
            }
        }
        //Als er niet gereden wordt en de joystick wel wordt gebruikt, roep de doDriveMotorJoystick functie aan en schrijf het resultaat weg naar de motor
        else if (isDriving == 0 && dDriveTargetKmHJoystick != 0)
        {
            dDriveRPMTarget = doDriveMotorJoystick(dDriveTargetKmHJoystick, iDirection, isDriving,
                lPreviousPositionUpdate, dCurrentMeters, dCurrentRadian, eElementList, iCurrentElement);

            dTargetDistance = -1;

            if (iDirection == 1)
            {
                digitalWrite(dir_a, HIGH);
            }
            else if (iDirection == 2)
            {
                digitalWrite(dir_a, LOW);
            }

            dDriveOutput = dDriveRPMTarget * DRIVEOUTPUTCORRECTION;

            if (dDriveOutput > MAXSPEEDOUTPUT || dDriveOutput < MINSPEEDOUTPUT)
            {
                if (bGlobalLogs)
                {
                    Serial.print("Wrong speed: ");
                    Serial.println(dDriveOutput);
                }
                analogWrite(pwm_a, 0);
            }
            else
            {
                analogWrite(pwm_a, dDriveOutput);
            }
        }
        //Als er wordt gereden en de joystick wel wordt gebruikt, roep de updateCurrentPositionJoystick functie aan en schrijf het resultaat weg naar de motor
        else if (isDriving == 1 && dDriveTargetKmHJoystick != 0)
        {
            dDriveOutput = updateCurrentPositionJoystick(dDriveRPMIncoming, dDriveTargetKmHJoystick,
                lPreviousPositionUpdate, dXDolly, dYDolly, dCurrentMeters, dCurrentRadian, eElementList,
                iCurrentElement, iDirection, isDriving, dDriveRPMTarget, iDriveTargetPercent, dTotalLength);

            if (iDirection == 1)
            {
                digitalWrite(dir_a, HIGH);
            }
            else if (iDirection == 2)
            {
                digitalWrite(dir_a, LOW);
            }

            if (dDriveOutput > MAXSPEEDOUTPUT || dDriveOutput < MINSPEEDOUTPUT)
            {
                if (bGlobalLogs)
                {
                    Serial.print("Wrong speed: ");
                    Serial.println(dDriveOutput);
                }

                analogWrite(pwm_a, 0);

                stopDolly();
            }
            else
            {
                analogWrite(pwm_a, dDriveOutput);
            }
        }
        //Als er wordt gereden en de joystick niet wordt gebruikt, kijk eerst of de joystick net is gebruikt, zoja, stop met rijden (want joystick = 0)
        else if (isDriving == 1 && dDriveTargetKmHJoystick == 0)
        {
            if (dTargetDistance == -1)
            {
                isDriving = 0;
            }
            else
            {
                //Als het doelwit is veranderd, ga door dezelfde code heen als als er niet gereden wordt
                if (iDriveTargetPercent != iDriveTargetPrevPercent)
                {
                    if (bGlobalLogs)
                    {
                        Serial.println("Targetpercent != PrevTargetPercent");
                    }

                    dDriveRPMTarget = doDriveMotor(dDriveTargetKmH, iDriveTargetPercent,
                        iDriveTargetPrevPercent, dTotalLength, eElementList, iCurrentElement, dXDolly,
                        dYDolly, dCurrentMeters, dCurrentRadian, dTargetDistance, iDirection, isDriving,
                        isTimelapse, iTimelapseTime, lPreviousPositionUpdate);

                    if (iDirection == 1)
                    {
                        digitalWrite(dir_a, HIGH);
                    }
                    else if (iDirection == 2)
                    {
                        digitalWrite(dir_a, LOW);
                    }

                    dDriveOutput = dDriveRPMTarget * DRIVEOUTPUTCORRECTION;

                    if (dDriveOutput > MAXSPEEDOUTPUT || dDriveOutput < MINSPEEDOUTPUT)
                    {
                        if (bGlobalLogs)
                        {
                            Serial.print("Wrong speed: ");
                            Serial.println(dDriveOutput);
                        }

                        analogWrite(pwm_a, 0);
                    }
                    else
                    {
                        analogWrite(pwm_a, dDriveOutput);
                    }
                }
                else
                {
                    //Update anders de positie
                    dDriveOutput = updateCurrentPosition(dDriveRPMIncoming, dDriveTargetKmH,
                        lPreviousPositionUpdate, dTargetDistance, dXDolly, dYDolly, dCurrentMeters,
                        dCurrentRadian, eElementList, iCurrentElement, iDirection, isDriving,
                        isTimelapse, dDriveRPMTarget, iDriveTargetPercent, iDriveTargetPrevPercent);

                    if (iDirection == 1)
                    {
                        digitalWrite(dir_a, HIGH);
                    }
                    else if (iDirection == 2)
                    {
                        digitalWrite(dir_a, LOW);
                    }

                    if (dDriveOutput > MAXSPEEDOUTPUT || dDriveOutput < MINSPEEDOUTPUT)
                    {
                        if (bGlobalLogs)
                        {
                            Serial.print("Wrong speed: ");
                            Serial.println(dDriveOutput);
                        }

                        analogWrite(pwm_a, 0);
                    }
                    else
                    {
                        analogWrite(pwm_a, dDriveOutput);
                    }
                }
            }
        }
        if (bGlobalLogs)
        {
            Serial.print("Panangle: ");
            Serial.println(dPanAngle);

            Serial.print("Tiltposition: ");
            Serial.println(dTiltPosition);

            Serial.print("X,Y: ");
            Serial.print(dXDolly);
            Serial.print(" , ");
            Serial.println(dYDolly);

            Serial.print("dCurrentRadian: ");
            Serial.println(dCurrentRadian);

            Serial.print("dCurrentMeters: ");
            Serial.println(dCurrentMeters);

            Serial.print("Direction: ");
            Serial.println(iDirection);

            Serial.print("CurrentElement: ");
            Serial.println(iCurrentElement);
            
            Serial.print("dDriveRPMIncoming: ");
            Serial.println(dDriveRPMIncoming);

            Serial.print("Driveoutput: ");
            Serial.println(dDriveOutput);

            Serial.println("=======");
        }
    }
}

// Functie die elke 100ms wordt aangeroepen
void intervalFunction()
{
    // Bekijk of de snelheid geen 0 is (dus er wordt bewogen)
    if (iTiltSpeed != 0)
    {
        changeTilt(iTiltSpeed, dTiltPosition, iTiltSpeedFactor);
    }
    else
    {
        changeTiltLoop(iTiltTarget, dTiltPosition, iTiltSpeedFactor);
    }

    // Bekijk of er een timelapse / tracking aan de gang is, zoja ga naar de auto functie
    if ((isTimelapse || isTracking) == 1 && isDriving == 1)
    {
        changePanAuto(dCurrentMeters, dCurrentRadian, dXFocus, dYFocus, dXDolly, dYDolly,
            iCurrentElement, eElementList, iDirection, dPanAngle, dPanOutput, dPanOutputFine,
            iDistanceFocusPoint, iDoAlter);
    }

    // Bekijk of de snelheid geen 0 is (dus er wordt bewogen)
    else if (iPanSpeed != 0)
    {
        changePan(iPanSpeed, dPanAngle, dPanOutput, dPanOutputFine);
    }

    // Schrijf alle waardes weg naar de DMX
    DmxMaster.write(1, (int)dPanOutput);
    DmxMaster.write(2, (int)dPanOutputFine);
    DmxMaster.write(5, iPanSpeedFactor);
}

//Stopt de dolly, berekent een aantal waardes die nodig zijn om hem te resetten
void stopDolly()
{
    double dCurrentMetersDone = calculateMetersDone(
        dCurrentMeters,
        dCurrentRadian,
        eElementList,
        iCurrentElement
    );

    double dCurrentPercentDone = dCurrentMetersDone / dTotalLength * 100;

    if(isDriving == 1)
    {
        iDriveTargetPercent = (int) dCurrentPercentDone;
    }

    isDriving = 0;

    analogWrite(pwm_a, 0);
}

BLYNK_WRITE(V56)
{
    if(param.asInt() == 1)
    {
        bFreeRoam = true;
    }
    else
    {
        dXDolly = 0;
        dYDolly = 0;
        iCurrentElement = 0;
        iDirection = 1;
        iDoAlter = 0;
        iDriveTargetPercent = 0;
        iDriveTargetPrevPercent = 0;
        dCurrentMeters = 0;
        dCurrentRadian = 0;
    
        isTracking = 0;
        trackled.off();
        trackled2.off();
    
        isTimelapse = 0;
        timelapseled.off();
        timelapseled2.off();
        bFreeRoam = false;
    }
}

// Wanneer de joystick wordt gebruikt, schrijf de tilt weg in de tiltspeed variabele (tussen -10 en 10)
BLYNK_WRITE(V1)
{
    iTiltSpeed = param.asInt();
    iTiltTarget = 0;
}

// Wanneer de joystick wordt gebruikt en er geen timelapse is én niet wordt getrackt, schrijf de pan weg in de panspeed variabele (tussen -10 en 10)
BLYNK_WRITE(V2)
{
    if (isTracking != 1 && isTimelapse != 1)
    {
        iPanSpeed = param.asInt();
    }
}

//Schrijf de doel KMH weg naar de variable
BLYNK_WRITE(V4)
{
    if (isTimelapse != 1)
    {
        dDriveTargetKmHJoystick = param.asInt() / 10;
    }
}

// Wanneer er niet wordt getrackt en er geen timelapse is, stel de panangle in (tussen -270 en 270, de hoek van de mvhd)
BLYNK_WRITE(V7)
{
    if (isTracking != 1 && isTimelapse != 1)
    {
        dPanAngle = param.asInt();

        // Map de angle naar DMX + een fine variabele voor DMX
        dPanOutput = maptonumber(dPanAngle + 270, 540, 255);
        dPanOutputFine = maptonumber(dPanOutput - (int)dPanOutput, 1, 255);

        // Schrijf de waardes naar de DMX als integers
        DmxMaster.write(1, (int)dPanOutput);
        DmxMaster.write(2, (int)dPanOutputFine);
        DmxMaster.write(5, iPanSpeedFactor);
    }
}

//Ontvang de string vanuit de terminal en parse hem
BLYNK_WRITE(V21)
{
    sReceivedData = param.asStr();

    parseSerialData;
}

//Zet de log in
BLYNK_WRITE(V23)
{
    if (param.asInt() == 1)
    {
        bGlobalLogs = true;
    }
    else
    {
        bGlobalLogs = false;
    }
}

// Stel de tilttarget in
BLYNK_WRITE(V8)
{
    iTiltTarget = param.asInt();
}

// Stel het doel in wanneer er geen timelapse wordt uitgevoerd, wordt niet direct behaald
BLYNK_WRITE(V29)
{
    if (isTimelapse != 1 && eElementList[0].getLength() > 0)
    {
        iDriveTargetPrevPercent = iDriveTargetPercent;
        iDriveTargetPercent = param.asInt();
        Serial.println("ChangedPercent");
    }
}

// Stel de KMH in
BLYNK_WRITE(V30)
{
    if(isTimelapse != 1)
    {
        dDriveTargetKmH = param.asInt() / 10;
    }
}

// Stel de distance in voor tracking
BLYNK_WRITE(V0)
{
    iDistanceFocusPoint = param.asInt();
}

// Wanneer er niet wordt getrackt of een timelapse aan de gang is, haal de focuspoints op en schrijf deze weg naar een text field in app
BLYNK_WRITE(V10)
{
    if (isTracking != 1 && isTimelapse != 1)
    {
        iDoAlter = getPointFocusCoordinates(dPanAngle, iDistanceFocusPoint, dCurrentRadian,
            dCurrentMeters, dXFocus, dYFocus, iCurrentElement, eElementList, iDirection, dXDolly,
            dYDolly, dTotalLength);
        Blynk.virtualWrite(V12, dXFocus);
        Blynk.virtualWrite(V11, dYFocus);
    }
}

// Stel isTracking variabele in, wanneer er geen timelapse aan de gang is is dit mogelijk.
BLYNK_WRITE(V13)
{
    if (isTimelapse != 1)
    {
        isTracking = param.asInt();

        // Lampjes aan of uit, ligt eraan of isTracking 1 is
        if (isTracking == 1)
        {
            trackled.on();
            trackled2.on();
        }
        else
        {
            trackled.off();
            trackled2.off();
        }
    }
}

// Stel de isTimelapse variabele in, wanneer er niet getrackt wordt
BLYNK_WRITE(V14)
{
    if (isTracking != 1)
    {
        isTimelapse = param.asInt();

        // Als timelapse aanstaat, zet ledjes aan en doe meer. Wanneer het uit is gezet doe ledjes uit en zet het doel op de huidige positie. De rest van de timelapse wordt in de loop gedaan.
        if (isTimelapse == 1)
        {
            timelapseled.on();
            timelapseled2.on();

            iDriveTargetPrevPercent = iDriveTargetPercent;
            // Zet de eindpositie als drivetarget
            iDriveTargetPercent = iTimelapseEndPercent;
        }
        else
        {
            timelapseled.off();
            timelapseled2.off();
            stopDolly();
        }
    }
}

// Stel de timelapsetime in (hoelang de timelapse erover doet in seconden)
BLYNK_WRITE(V17)
{
    iTimelapseTime = param.asInt();
}

// Stel de timelapse eindpositie in (ITargetPercent in wezen, alleen deze gaat pas als timelapse aanstaat)
BLYNK_WRITE(V18)
{
    iTimelapseEndPercent = param.asInt();
}

//Reset focuspunt
BLYNK_WRITE(V40)
{
    dXFocus = 0;
    dYFocus = 0;
}

//Reset tilt
BLYNK_WRITE(V41)
{
    iTiltSpeed = 0;
    dTiltPosition = 0;
    iTiltTarget = 0;
}

//Reset dolly
BLYNK_WRITE(V42)
{
    dXDolly = 0;
    dYDolly = 0;
    iCurrentElement = 0;
    iDirection = 1;
    iDoAlter = 0;
    iDriveTargetPercent = 0;
    iDriveTargetPrevPercent = 0;
    dCurrentMeters = 0;
    dCurrentRadian = 0;

    isTracking = 0;
    trackled.off();
    trackled2.off();

    isTimelapse = 0;
    timelapseled.off();
    timelapseled2.off();
}

//Stopdolly
BLYNK_WRITE(V43)
{
    stopDolly();
}

//Stel tiltfactor in
BLYNK_WRITE(V45)
{
    iTiltSpeedFactor = param.asInt();
}

//Stel panfactor in
BLYNK_WRITE(V46)
{
    iPanSpeedFactor = 255 - param.asInt();
}

//Kies rails
BLYNK_WRITE(V74)
{
    int iIndexMenu = param.asInt();

    switch(iIndexMenu)
    {
        case 1:
            sReceivedData = "2_0,0,-0.726,4.115,-12.032,0,2,4.2,12.032,-250,0,0.349|-0.726,4.115,-2.815,7.734,-12.032,0,2,4.2,12.032,-230,-5.934,-5.585";
            break;
        case 2:
            sReceivedData = "2_0,0,9.397,3.42,,,1,10,,20,,|9.397,3.42,5.042,15.385,7.22,9.402,2,20,6."
                    "366,-160,-1.222,1.92";
            break;
        case 3:
            sReceivedData = "1_0,0,-0.726,4.115,-12.032,0,2,4.2,12.032,-250,0,0.349";
            break;
    }
    
    parseSerialData();
}
