// This header has all the (extern) declarations of the globals.
// "extern" means "this is instantiated somewhere, but here's what the name
// means.
#include "globals.h"
#include "hardware.h"

// We need to actually instantiate all of the globals (i.e. declare them once
// without the extern keyword). That's what this file does!

// Hardware initialization: Instantiate all the things!
uLCD_4DGL uLCD(p9,p10,p11);             // LCD Screen (tx, rx, reset)
//SDFileSystem sd(p5, p6, p7, p8, "sd");  // SD Card(mosi, miso, sck, cs)
Serial pc(USBTX,USBRX);                 // USB Console (tx, rx)
MMA8452 acc(p28, p27, 100000);        // Accelerometer (sda, sdc, rate)
DigitalIn button1(p21);                 // Pushbuttons (pin)
DigitalIn button2(p22);
DigitalIn button3(p23);
AnalogOut DACout(p18);                  // Speaker (pin)
PwmOut speaker(p25); 
wave_player waver(&DACout);


// Some hardware also needs to have functions called before it will set up
// properly. Do that here.
int hardware_init()
{    
    //Initialize pushbuttons
    button1.mode(PullUp); 
    button2.mode(PullUp);
    button3.mode(PullUp);
    
    return ERROR_NONE;
}

GameInputs read_inputs() 
{
    GameInputs in;
    in.b1 = !button1; // Inverted, because low voltage means "pressed"
    in.b2 = !button2;
    in.b3 = !button3;
    // initialize x,y,z
    double x, y, z;     
    acc.readXYZGravity(&x,&y,&z);   // read acceleration data in x,y,z directions
    in.ax = x;                      // x direction acceleration data
    in.ay = y;                      // y direction acceleration data
    in.az = z;                      // z direction acceleration data
    //pc.printf("Inputs: %d %d %d %f %f %f\r\n", in.b1, in.b2, in.b3, in.ax, in.ay, in.az);
    return in;
}
