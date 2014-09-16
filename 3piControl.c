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

    uint8_t cal = load_eeprom<uint8_t>(1);
    if(cal == 42)
    {
        load_sensor_cal(2);
        calibrated = true;
    }

    uint16_t displayTimer = 100;
    Packet pkt_send;

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

                displayTimer = 100;
            }
            else
            {
                display.gotoXY(0,0);
                int16_t value;
                for(uint8_t i = 0; i < 5; ++i)
                {
                    value = getSensorValue(i, calibrated);
                    display.send_data(bar_graph_characters[value/103]);
                }
                display.print("   ");
                displayTimer = 100;
            }

            pkt_send.reset(0x10);
            pkt_send.writeUInt16(getBatteryVoltage());
            pkt_send.send();

            pkt_send.reset(0x11);
            pkt_send.writeInt16(getLeftMotor());
            pkt_send.writeInt16(getRightMotor());
            pkt_send.send();

            pkt_send.reset(0x12);
            for(uint8_t i = 0; i < 5; ++i)
                pkt_send.writeInt16(getSensorValue(i, calibrated));
            pkt_send.send();
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
           store_sensor_cal(2);
           store_eeprom(1, uint8_t(42));
           calibrated = true; 
        }
    }
    return false;
}
