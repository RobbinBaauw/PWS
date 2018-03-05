#include <Wire.h>
#define reed 2 // pin waar de magneetsensor op is aangesloten

// grootte van de array is 5 die hieronder staat
// deze wordt gebruikt om de RPM in op te slaan later
// deze char array wordt later namelijk verstuurt
char buffer[5];

// variable
float radius = 1.2598; // omtrek wiel (in inches)
int iReedVal;
long lTimer = 0;
float fMPH = 0.00;
float fCircumference;
// definieert variable om de snelheid mee te berekenen
double dKMH = 0.00;
double dMPS = 0.00;
double dMPM = 0.00;
double dRPM = 0.00;
double dRPM1 = 0.00;
double dRPM2 = 0.00;
double dRPM3 = 0.00;
double dRPMgem = 0.00;
double dRPMbij = 0.00;

int iMaxReedCounter = 100;
//
int reedCounter;
// nodig om het gemiddelde RPM te berekenen
int iChangingVar = 0;

void setup()
{
    Wire.begin(4);
    Wire.onRequest(requestEvent);
    Serial.begin(9600);

    // zet reedCounter gelijk aan de iMaxReedCounter
    reedCounter = iMaxReedCounter;
    // berekent de omtrek
    fCircumference = 2 * 3.14 * radius;

    // Maak timer 1
    //de in de chip ingebouwde time registers worden hierin geconfigureerd
    cli(); // zorgt ervoor dat er geen onderbrekingen komen zodat de timers instellen niet wordt onderbroken

    // Hieronder worden soorten registers in geheel ingesteld op nul deze registers hebben 8 bytes en die kunnen individueel ingesteld worden
    TCCR1A = 0; 
    TCCR1B = 0;
    TCNT1 = 0;
    //dit is de waarde die wordt gebruikt bij TIMSK1 
    // de standaard arduino clock runt op 16mhz maar door een zogenoemde prescaler te gebruiken kun je dit aanpassen volgens de onderstaande formule
    //(timer speed (Hz)) = (Arduino clock speed (16MHz)) / prescaler
    // de prescaler is in dit geval 8, die wordt op 8 gezet onderaan bij (TCCR1B |= (1 << CS11);)
    // je kan de onderbrekingswaarderegister berekenen met de volgende formule
    //onderbrekingswaarderegister = [ 16,000,000Hz/ (prescaler * onderbrekingsfrequentie) ] - 1
    // we hebben gekozen voor een 1000hz onderbreking
    //het getal (onoderbrekingswaarderegister) wordt hieronder ingevuld
    OCR1A = 1999; // = (16*10^6) / (1000*8) - 1
    // zet CTC aan (Clear Timer on Compare Match), deze timer checkt of de waarde van deze timer gelijk is aan een bepaalde waarde in het register van de timer
    // als het de waarden overeenkomen dan kan deze timer een actie triggeren zoals een onderbreking
    TCCR1B |= (1 << WGM12);
    // zet deze timer op CS11, dit betekent dat de prescaler op 8 wordt gezet
    TCCR1B |= (1 << CS11);
    //als de waarde hierboven (OCR1A) kleiner is dan 1 dan zorgt de timer voor een onderbreking
    TIMSK1 |= (1 << OCIE1A);

    sei(); // nu laat hij de onderbrekingen weer toe want de timers zijn ingesteld
    // END Timer 1
}
///
///

ISR(TIMER1_COMPA_vect)//gebruikt timer 1 die hierboven is ingesteld en voert de code hieronder uit wanneer de timer dat aangeeeft van hierboven
{ // onderbreekt met een frequentie van 1kHz om de magneetsensor af te lezen
    iReedVal = digitalRead(reed); // krijgt the waarde van de magneetsensor
    if (iReedVal)
    { // als de magneetsensor dicht is
        if (reedCounter == 0)
        { // als de minimale tijd is verstreken tussen de pulsen
            fMPH = (56.8 * float(fCircumference)) / float(lTimer); // bereken miles per hour
            lTimer = 0; // reset de lTimer
            reedCounter = iMaxReedCounter; // reset de reedCounter
        }
        else
        {
            if (reedCounter > 0)
            { // laat de reedCounter niet negatief worden
                reedCounter -= 1; // verminder de reedCounter
            }
        }
    }
    else
    { // als de magneetsensor open is
        if (reedCounter > 0)
        { // laat de magneetsensor niet negatief worden
            reedCounter -= 1; // verminder reedCounter
        }
    }
    if (lTimer)
    {
        fMPH = 0; // als er geen nieuwe pulsen meer binnenkomen staat de dolly stil en wordt de fMPH
                  // op nul gezet
    }
    else
    {
        lTimer; // toename lTimer
    }
}


void loop()
{
    // bereken elke loop de RPM
    berekenRPM();
}

void berekenRPM()
{
    // bereken RPM
    float(dKMH) = float(fMPH * 1.61);
    float(dMPS) = float(dKMH / 3.6);

    float(dMPM) = float(dMPS * 60);
    float(dRPM) = float(dMPM / 0.2);

    // zet de RPM naar een vorige RPM als deze RPM te hoog is doordat hij denkt dat het magneetje
    // twee keer langs is gegaan in plaats van 1 keer
    if (dRPM >= 240)
    {
        dRPM = dRPM1;
    }
    // sla de laatste 5 RPM metingen op in RPM, dRPM1, dRPM2, dRPM3
    switch (iChangingVar)
    {

        case 0:
            iChangingVar++;
            break;
        case 1:
            //
            dRPM1 = RPM;
            iChangingVar++;
            break;
        case 2:
            //
            dRPM2 = RPM;
            iChangingVar++;
            break;
        case 3:
            //
            dRPM3 = RPM;
            iChangingVar++;
            break;
        case 4:
            iChangingVar = 0;
            break;
    }

    // tel alle RPM's bij elkaar op
    dRPMbij = dRPM + dRPM1 + dRPM2 + dRPM3;
    // deel het door 4 om het gemiddelde te krijgen
    dRPMgem = dRPMbij / 4;
}

void requestEvent()
{
    // dtostrf zorgt ervoor dat de double RPMgem geconverteerd wordt naar een string.
    // Deze string wordt in een char array geplaatst genaamd buffer
    // de waardes achter dtostrf zijn eerst doublevariable, minimaleStringlengteinclusiefdepunt,
    // aantalGetallenAchterDeKomma, lege array waar de waardes van de nu nieuwe String in worden
    // gezet)
    dtostrf(dRPMgem, 3, 3, buffer);

    // hij print de array in de console
    Serial.println(String(buffer));
    // uiteindelijk stuurt hij de array per byte door naar de master arduino
    Wire.write(buffer);
}
