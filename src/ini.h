#define RIGHT_MOTOR  rb::MotorId::M2
#define LEFT_MOTOR  rb::MotorId::M1

Servo servo0, servo1; 

int servo0_down = 145; // ruka uplne dole
int servo0_catch_small = servo0_down - 5; 
int servo0_catch_big = servo0_down - 10;  
int servo0_catch_puck = servo0_down -5;
int servo0_step = servo0_down -30; // pozice pro vylozeni nakladu na schod a na kamion 
int servo0_up = servo0_down - 80;


int servo1_open = 90;  // otevreno do praveho uhlu 
int servo1_open_small = 100;
int servo1_open_puck = 70; 
int servo1_open_max = 0; // maximalni otevreni ruky robota - radlice 
int servo1_close_max = 130;
int servo1_close_big_brick = 110; // zavre servo pro velkou kostku
int servo1_close_small_brick = 115; 
int servo1_close_puck = 105; 


float position_servo0 = 100; // pro postupne krokovani serva pro kalibraci 
float position_servo1 = 100; 

float rychlost_0 = 0;
float rychlost_1 = 0;

float cm = 24.1; // pocet tiku na popojeti o cm 

const int AXIS_COUNT = 7;
const int BTN_COUNT = 13;
int axis[AXIS_COUNT] = {5,6,7,8,9,10,11};
byte btn[BTN_COUNT] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
byte btn_last[BTN_COUNT] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
int speed_coef = -100; // nasobeni hodnoty, co leze z joysticku

int power_motor = 90;
int slow_power_motor = 60;
long pauza = 600;

bool L_G_light = false; // pro blikani zelene LED - indikuje, ze deska funguje 

rb::Manager& rbc() 
{
    static rb::Manager m(false, false);  // ve výchozím stavu se motory po puštění tlačítka vypínají, false zařídí, že pojedou, dokud nedostanou další pokyn 
    return m;
}

bool sw1() { return !rbc().expander().digitalRead(rb::SW1); }
bool sw3() { return !rbc().expander().digitalRead(rb::SW3); }

float trim(float value, float min, float max)
{
    if (value < min)
        return min; 
    if (value > max)
        return max; 
    return value;
}

std::atomic_bool end_L;
std::atomic_bool end_R;
void end_left(rb::Encoder&)
{  
    end_L = true;
    Serial.println("stop L");
}

void end_right(rb::Encoder&)
{  
    end_R = true;
    Serial.println("stop R");
}

void catch_thing(int what) // 0 - mala kostka, 1 - velka kostka, 2 - puk/znacka
{                           // predpokladam, ze na zacatku stoji robot v primerene vzdalenosti pred kostkou
    Serial.print("what: "); Serial.println(what);
    switch(what) {       
        case 0:
            servo1.write(servo1_open_small);  
            position_servo1 = servo1_open_small;
            delay(pauza);
            servo0.write(servo0_catch_small); 
            position_servo0 = servo0_catch_small;
            Serial.print("in:0: "); Serial.print(position_servo0); Serial.print(" "); 
            Serial.print("in:0: "); Serial.print(position_servo1); Serial.print(" ");
            break;
         case 1:
            servo1.write(servo1_open); 
            position_servo1 = servo1_open;
            delay(pauza);
            servo0.write(servo0_catch_small); 
            position_servo0 = servo0_catch_small;
            Serial.print("in:1: "); Serial.print(position_servo0); Serial.print(" "); 
            Serial.print("in:1: "); Serial.print(position_servo1); Serial.print(" ");
            break;
        case 2:
            servo1.write(servo1_open_puck); 
            position_servo1 = servo1_open_puck; 
            delay(pauza);
            servo0.write(servo0_catch_puck); 
            position_servo0 = servo0_catch_puck;
            Serial.print("in:2: "); Serial.print(position_servo0); Serial.print(" "); 
            Serial.print("in:2: "); Serial.print(position_servo1); Serial.print(" ");
            break;           
    }  
    delay(800);
    end_L = false;
    end_R = false; 
    rbc().motor(LEFT_MOTOR)->drive(2*cm, slow_power_motor, end_left);
    rbc().motor(RIGHT_MOTOR)->drive(2*cm, slow_power_motor, end_right);
    while (! (end_L and end_R) ) {
        delay(10); // ceka, na provedeni pojezdu 
    }
    switch(what) {
        case 0:
            servo1.write(servo1_close_small_brick); 
            position_servo1 = servo1_close_small_brick;
            delay(pauza);
            servo0.write(servo0_step); 
            position_servo0 = servo0_step;
            break;
         case 1:
            servo1.write(servo1_close_big_brick); 
            position_servo1 = servo1_close_big_brick;
            delay(pauza);
            servo0.write(servo0_step); 
            position_servo0 = servo0_step;            
            break;
        case 2:
            servo1.write(servo1_close_puck); 
            position_servo1 = servo1_close_puck; 
            break;       
    }
}

void let() // pusti, co zrovna drzi
{
    servo1.write(servo1_open); 
    position_servo1 = servo1_open;
    delay(300);
}

