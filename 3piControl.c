#include "3piLibPack.h"
#include "packets.h"
#include <math.h>

// 10 levels of bar graph characters
const char bar_graph_characters[10] = {' ',0,0,1,2,3,3,4,5,255};

const char levels[] = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
};

bool displaySensors = false;
bool accel = true;
bool calibrated = false;
bool checkButtons();

void run()
{
    display.printToXY("0", 0, 1);
    display.printToXY("0", 7, 1);

    // Load characters to make bars
    display.loadCustomCharacter(levels+0,0);
    display.loadCustomCharacter(levels+1,1);
    display.loadCustomCharacter(levels+2,2);
    display.loadCustomCharacter(levels+4,3);
    display.loadCustomCharacter(levels+5,4);
    display.loadCustomCharacter(levels+6,5);

    uint16_t displayTimer = 100;

    while(true)
    {
       if(!displayTimer)
        {
            rs232.wait();
            if(!displaySensors)
            {
                uint16_t vol = getBatteryVoltage();
                display.printNumToXY(vol, 0, 0);
                display.print("mV ");
                display.printNumber(uint8_t(accel));

                rs232.sendCharacter('\r');
                rs232.sendNumber(vol);
                rs232.send(" mv");
                displayTimer = 1000;
            }
            else
            {
                display.gotoXY(0,0);
                int16_t value;
                for(uint8_t i = 0; i < 5; ++i)
                {
                    value = getSensorValue(i, calibrated);
                    display.send_data(bar_graph_characters[value/103]);
                    rs232.sendNumber(value, 5);
                }
                rs232.sendCharacter('\n');
                display.print("   ");
                displayTimer = 100;
            }
        } else --displayTimer;

        if(readPacket())
            handlePacket(&pkt);

        if(!checkButtons())
            delayMicroseconds(1000);
    }
}

bool checkButtons()
{
    if(isPressed(BUTTON_B))
    {
        delay(5);
        if(isPressed(BUTTON_B))
        {
            waitForRelease(BUTTON_B);
            accel = !accel;
            setSoftAccel(accel);
            display.printNumToXY(uint8_t(accel), 7, 0);
        }
        return true;
    }
    if(isPressed(BUTTON_A))
    {
        delay(5);
        if(isPressed(BUTTON_A))
        {
            waitForRelease(BUTTON_A);
            displaySensors = !displaySensors;
        }
        return true;
    }
    if(isPressed(BUTTON_C))
    {
        delay(5);
        if(isPressed(BUTTON_C))
        {
           waitForRelease(BUTTON_C);
           delay(500);
           cal_round();
           calibrated = true; 
        }
    }
    return false;
}
