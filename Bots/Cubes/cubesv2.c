#include <kipr/botball.h>
#include <gyrolib.h>

int main()
{
    int lm = 3; //left motor
    int rm = 0; //right motor

    int lf1 = 0; //linefollow analog port
    int lf2 = 1;

    int lfthresh = 1500;
    int degrees = 631;
	//calibrate_degrees();
 	ig(lm, rm, degrees);
	
    dwg(850, 3000);
    
    int initangle = 40;
    
    twg(-initangle);
    
    //dwg(850, 8500);
    //printf("greghew");
    
    dua(850, lf2, lfthresh);
    msleep(50);
    dwg(250, 500);
    msleep(50);
    dua(850, lf2, lfthresh);
    msleep(50);
    awg(500, 600, initangle);
    //linefollow along the main board line until it hits the perpindicular cube line
    while(!(analog(lf1) > lfthresh && analog(lf2) > lfthresh)){ //while both sensors aren't on black
        if(analog(lf1) < lfthresh){ //on white
            mav(lm, 750);
            mav(rm, 500);
        }else{ //on black
            mav(lm, 500);
            mav(rm, 750);
        }
    }

    printf("on perp line\n");

    //linefollow until right sensor hits white
    while(analog(lf2) > lfthresh){
        mav(lm, 0);
        mav(rm, 500);
    }


    //drive forward until it's in the box
    dwg(1000, 5000);


    return 0;
}
