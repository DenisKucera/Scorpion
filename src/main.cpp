#include <Arduino.h>
#include "RBControl_manager.hpp"
#include <Servo.h>
#include <Wire.h>
#include "time.hpp"
#include <stdint.h>
#include "stopwatch.hpp"
#include "nvs_flash.h"
#include "BluetoothSerial.h"

#include "ini.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

void testovaci();
bool read_joystick(); // definice dole pod hlavnim programem

BluetoothSerial SerialBT;

void setup() {
    Serial.begin (115200); 
    if (!SerialBT.begin("PM_Falcon")) //Bluetooth device name - na pocitaci Burda port 19
    {
        Serial.println("!!! Bluetooth initialization failed!");
    }
    else
    {
        SerialBT.println("!!! Bluetooth work!");
        Serial.println("!!! Bluetooth work!");
    }

    Serial.print ("Starting...\n");
    rbc();
    Serial.print ("RBC initialized1\n");
    // auto& batt = rbc().battery();
    // batt.setCoef(9.0);  // toto musí být napevno, aby si cip nemyslel, ze je nizke napeti na baterce, toto napeti se musi kontrolovat rucne  
  
    servo0.attach(33); 
    servo0.write(servo0_step); 
    position_servo0 = servo0_step;
    servo1.attach(32); 
    servo1.write(position_servo1); 
    
    printf("ZKONTROLOVAL JSI BATERKU ?\n" );
    printf("ZKONTROLOVAL JSI STOP-TLAČÍTKO ?\n" ); 
    Serial.println("Ready to Start Roadside Assistance Beginner - Štourač" );

}
    
timeout send_data { msec(500) }; // timeout zajistuje posilani dat do PC kazdych 500 ms

void loop() 
{
    rbc().leds().blue(SerialBT.hasClient());  // modrá LED sviti, kdyz jede bluetooth 
    rbc().leds().yellow(sw1()); // rucni zapinani zlute ledky - testovani odezvy desky 
    if (send_data) {
        send_data.ack();
        if (L_G_light) L_G_light = false; else  L_G_light = true;
        rbc().leds().green(L_G_light);
        SerialBT.println(millis());
    }

    if ( read_joystick() )
    {
        // float axis_0 = (abs(axis[2]) < 3) ? 0 : axis[2] /128.0; 
        // axis_0 = axis_0*axis_0*axis_0;
        // float axis_1 = (abs(axis[3]) < 3) ? 0 : axis[3] /128.0; 
        // axis_1 = axis_1*axis_1*axis_1;
        // int levy_m = (axis_1 + (axis_0 /2 )) * speed_coef;
        // int pravy_m = (axis_1 - (axis_0 /2 )) * speed_coef;?
        int levy_m, pravy_m;
        if (axis[3] <= 0 )
        {
            levy_m = trim( (-axis[3] + (axis[2] /2 )) , -99 , 99 );
            pravy_m = trim( (-axis[3] - (axis[2] /2 )) , -99, 99 );
        }
        else 
        {
            levy_m = trim( (-axis[3] - (axis[2] /2 )) , -99 , 99 );
            pravy_m = trim( (-axis[3] + (axis[2] /2 )) , -99, 99 );
        }
        printf(" %i %i \n ", levy_m, pravy_m );
        rbc().setMotors().power(LEFT_MOTOR, levy_m)
                         .power(RIGHT_MOTOR, pravy_m)
                         .set();
       
        // if (btn[0] == 1 )
        //     catch_thing(0); // chyti malou kostku 
        // if (btn[1] == 1 )
        //     catch_thing(1); // chyti velkou kostku 
        // if (btn[2] == 1 )
        //     catch_thing(2); // chyti puk
        // if (btn[3] == 1 )
        //    let();           // pusti cokoliv 

        float rychlost = 1;
        if (btn[6] == 1 )
            rychlost_0 = rychlost;
        else if (btn[4] == 1 )
             rychlost_0 = -rychlost;
        else
            rychlost_0 = 0;

        if (btn[5] == 1 )
            rychlost_1 = rychlost;
        else if (btn[7] == 1 )
             rychlost_1 = -rychlost;
        else
            rychlost_1 = 0;

        Serial.print(position_servo0); Serial.print(" "); 
        Serial.print(position_servo1); Serial.print(" ");
        SerialBT.print(levy_m); SerialBT.print(" "); SerialBT.println(pravy_m);
    }
    position_servo0 = trim(position_servo0 + rychlost_0, servo0_up, servo0_down);  
    position_servo1 = trim(position_servo1 + rychlost_1, servo1_open_max, servo1_close_max);
    servo0.write(position_servo0);
    servo1.write(position_servo1);
    delay(10);
}

void testovaci()
{
    if(Serial.available()) {
        char c = Serial.read();
        switch(c) {
            case 't':
                if (position_servo0 >= 5)  position_servo0 = position_servo0 -5;               
                servo0.write(position_servo0);
                Serial.write(" 0: "); 
                Serial.println(position_servo0);
                break;
            case 'u':
                if (position_servo0 <= 175)  position_servo0 = position_servo0 +5;               
                servo0.write(position_servo0);
                Serial.write(" 0: "); 
                Serial.println(position_servo0);
                break;
            case 'g':
                if (position_servo1 >= 5)  position_servo1 = position_servo1 -5;               
                servo1.write(position_servo1);
                Serial.write(" 1: "); 
                Serial.println(position_servo1);
                break;
            case 'h':
                if (position_servo1 <= 175)  position_servo1 = position_servo1 +5;               
                servo1.write(position_servo1);
                Serial.write(" 1: "); 
                Serial.println(position_servo1);
                break;
            
            case 'w':
                rbc().setMotors().power(LEFT_MOTOR, power_motor)
                                 .power(RIGHT_MOTOR, power_motor)
                                 .set();
                break;
            case 's':
                rbc().setMotors().power(LEFT_MOTOR, -power_motor)
                                 .power(RIGHT_MOTOR, -power_motor)
                                 .set();
                break;
            case 'a':
                rbc().setMotors().power(LEFT_MOTOR, -power_motor)
                                 .power(RIGHT_MOTOR, power_motor)
                                 .set();
                break;
            case 'd':
                rbc().setMotors().power(LEFT_MOTOR, power_motor)
                                 .power(RIGHT_MOTOR, -power_motor)
                                 .set();
                break;
          

            case ' ':
                rbc().setMotors().stop(LEFT_MOTOR)
                                 .stop(RIGHT_MOTOR)
                                 .set();
                break; 

            default:
                Serial.write(c);
                break;
        } 
    }

}

// ********************************************************************

bool read_joystick()
{
    if ( SerialBT.available() == 0 )
        return false;

    int test = SerialBT.read();
    if (test == 0x80)
    {
        int axis_count = SerialBT.read();
        if (axis_count >= AXIS_COUNT)
        {
            Serial.println("********* CHYBA V POCTU OS !!! ************");
        }
        else
        {
            for (int x = 0; x < axis_count; x++)
            {
                while(SerialBT.available() < 1)
                {
                    // DO NOTHING - WAITING FOR PACKET
                    delay(1);
                }

                int8_t tmp = SerialBT.read();
                axis[x] = tmp;
                Serial.print(x);  
                Serial.print(": ");
                Serial.print(axis[x], DEC);
                Serial.print(" ");
                SerialBT.print(x);
                SerialBT.print(": ");
                SerialBT.print(axis[x], DEC);
                SerialBT.print(" ");

            }
            return true;
        }
        
        
    }
    else if  ( test == 0x81 )
    {
        while(SerialBT.available() < 1) {
            // DO NOTHING - WAITING FOR PACKET
            delay(1);
        }
        byte a = SerialBT.read();
        if ( a >= BTN_COUNT )
        {
            Serial.println("********* CHYBA V POCTU TLACITEK !!! ************");
        }
        else 
        {
            while(SerialBT.available() < 1) {
                // DO NOTHING - WAITING FOR PACKET
                delay(1);
            }
            btn_last[a] = btn[a];
            btn[a] = SerialBT.read();
            Serial.print(a, DEC); Serial.print(": "); Serial.print(btn[a], DEC); Serial.print("last: "); Serial.print(btn_last[a], DEC);
            return true;
        }

    }
    return false;
}