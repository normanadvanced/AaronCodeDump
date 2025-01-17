#include <kipr/botball.h>
#include "gyrolib.h"
#include "collision.h"


const int reg = 0; //mainServo
const int neg = 3; //offMainServo

const int neginit = 2047 - 55;
const int reginit = 0;



const int deg = 654;

const int lm = 2;

const int rm = 3;

const int bm = 1;

const int llf = 2;
const int rlf = 1;
const int blf = 4;

const int blackThreshold = 2000;
const int blackOnBrownThreshold = 2000;
const int backLFThreshold = 3700;

//Gradually moves motors to a relative position. Valid values are from -2047 - 2047.
void moveRel(int pos){

    int initPosReg = get_servo_position(reg);
    int initPosNeg = get_servo_position(neg);

    //printf("init pos: %i,%i\n", initPosReg, initPosNeg);

    int mult = 1;

    if(pos < 0){
        mult = -1;
        pos = -pos;
    }

    int chunkSize = 50;

    int steps = pos / chunkSize;

    double remainder = steps % chunkSize;

    int i;
    for(i = 0; i < steps; i++){

        set_servo_position(reg, get_servo_position(reg) + (chunkSize * mult));
        //printf("Setting reg pos to %i\n", get_servo_position(reg) + (chunkSize * mult) );

        set_servo_position(neg, get_servo_position(neg) - (chunkSize * mult));
        //printf("Setting neg pos to %i\n", get_servo_position(neg) - (chunkSize * mult));

        msleep(30);
    }
    msleep(200);


    set_servo_position(reg, initPosReg + (pos*mult));

    //printf("Setting final reg pos %i\n", initPosReg + (pos*mult));

    set_servo_position(neg, initPosNeg - (pos * mult));
    //printf("Setting final neg pos %i\n", initPosNeg - (pos * mult));




}


//squares the bot up on a line using parallel line sensors
//int time represents seconds to spend squaring up
//int flip will be multiplied to the motor velocity
//negative flips will make you move backward on black and forward on white.
//by default, this will move forward on black and backward on white.
//care needs to be exercised to select a flip that ensures that
//the movement is slow enough to reliably align on a line, as well as 
//have the proper sign (positive/negative) to actually hit a line and not
//go in some direction forever
void doubleSquareUp(int time, int flip){


    printf("entered sq up \n");

    int startSeconds = seconds();


    while(seconds() - startSeconds < time){

        int lmfinished = 0;

        int rmfinished = 0;
        if(analog(rlf) > blackThreshold){
            mav(rm, 200 * flip);   
        }else{
            //lmfinished = 1;
            mav(rm, -200 * flip);
        }
        if(analog(llf) > blackThreshold){
            mav(lm, 200 * flip);   
        }else{

            mav(lm, -200 * flip); 
            //rmfinished = 1; 
        }

        //stop = lmfinished && rmfinished;

    }

    printf("exited squp\n");
    freeze(lm);
    freeze(rm);
    msleep(500);   
}

//instantly moves both servos to an absolute position.
//this is almost always less preferable to moveRel
void balMove(int pos){
    set_servo_position(reg, reginit + pos);
    set_servo_position(neg, neginit - pos);
    msleep(1000);
}

//oscillation functions for bucket sucking. 
//osc1 operates on a cosine function
double osc1(int x){
    int period = 1000;
    int mult = (((x / period) % 2) == 0) ? 1 : 1;
    return mult * cos((2 * M_PI * x)/(period));
}
//osc 2 works on a sine function that flips sign on odd multiples
//of the period, ensuring a full cone of movement instead of a half cone
double osc2(int x){
    int period = 1000;
    int mult = (((x / period) % 2) == 0) ? 1 : -1;
    return mult * sin((2 * M_PI * (x + M_PI))/(period));
}

//Hard aligning with gyro is usually a poor idea
/*void hardAlign(int et1,int et2, int tolerance,int length,int intervals)
{
    int startTime = seconds();
    while(startTime + Length > seconds())
    {
    //int et1start = analog(et1);
    int et2start = analog(et2);
    while(analog(et2) > et2start + tolerance && analog(et2) < et2start - tolerance)
    	{
			mav(lm, 1500);
     	   mav(rm, 1500);
     	   msleep(intervals);
    	}
    }
}*/

int initBalMoveVal = 1601;


int rampApproach(int duaLength, int startDuaSeconds, int moves){
    if((seconds() - startDuaSeconds) * 1000 > duaLength) return 0;

    printf("\nRunning rampApproach for %f ms\n", duaLength - (seconds() - (double)startDuaSeconds)*1000);

    if(duaac(-1200, llf, rlf, blackThreshold, duaLength - (seconds() - (double)startDuaSeconds)*1000)){
        printf("\n\n*****************************DUA BROKEN****************************\n");

        int timeElapsed = seconds() - startDuaSeconds;

        int portToMove = lm;
        int sensorPort = llf;

        int vel = 500;

        int falseAlarm = 0;

        if(analog(llf) > blackThreshold){
            //hit left sensor 
            printf("\nDUAAC broke on left sensor\n");
            portToMove = lm;
            sensorPort = llf;

            if(moves == llf){
                vel = -1 * vel;
                moves = 0;
            }else{
                moves = lm;
            }
        }else if(analog(rlf) > blackThreshold){
            //hit right sensor
            printf("\nDUAAC broke on right sensor\n");
            portToMove = rm;
            sensorPort = rlf;

            if(moves == rlf){ 
                vel = -1 * vel;
                moves = 0;
            }else{
                moves = rm;
            }
        }else{
            falseAlarm = 1;
        }



        if(!falseAlarm){
            mav(portToMove, vel);
            msleep(100);
            freeze(portToMove);
        }

        int overlyCarefulTime = seconds();
        while(analog(sensorPort) > blackThreshold && seconds() - overlyCarefulTime < 5){ //be more tolerant of black
            mav(portToMove, vel);
        }
        freeze(portToMove);
        mav(portToMove, 0);
        msleep(50);


        printf("\nDWG for remaining time %f\n", (duaLength) - (timeElapsed*1000));

        rampApproach(duaLength, startDuaSeconds, moves);

        printf("Now outside of compensating dwg\n");
    }
}

int main()
{
    enable_servos();

    moveRel(-1 * (get_servo_position(reg) - 900));
    balMove(900);
    msleep(500);


    ig(lm, rm, deg);
    msleep(500);

    wait_for_light(5);
    shut_down_in(119);

    int startRunSeconds = seconds();
    moveRel(-1 * (get_servo_position(reg) - initBalMoveVal));
    balMove(initBalMoveVal);



    //turn 45 deg to the right to face the black tape perpendicular to board edge
    twg(-45);


    //drive until said black line is hit
    dua(1400, llf, blackThreshold);

    //tick forward to ensure line sensors are firmly on black;
    dwg(500, 500);

    //align with far edge of tape
    doubleSquareUp(2.5, 1);

    //reset absolute theta to reduce drift
    set_absolute_theta(45);
    msleep(50);
    printf("abbytheta%d\n", get_absolute_theta());
    msleep(50);

    //turn 13 degrees towards the coupler
    twg(13);


    //drive forward until coupler is within arm
    dwg(1000, 3250);


    printf("\n%f\n", get_absolute_theta());

    //turn the coupler into the corner of the board
    int cSec = seconds();
    while(get_absolute_theta() > 5 && (seconds() - cSec) < 5){
        mav(lm, 1500);
        mav(rm, -1500);   
    }
    freeze(lm);
    freeze(rm);
    printf("\nabsThetaAfterCornerPipe:%f\n", get_absolute_theta());

    int startTime;


    //drive forward into corner of board to shove coupler safely into place
    mav(lm, 600);
    mav(rm, 600);
    msleep(2000);

    //turn to absolute 45, which is the absolute value we initialized earlier.
    //aligns robot for a double square up
    set_mode(1);
    twg(45);
    set_mode(0);

    msleep(50);

    //drives backwards towards black line
    dwg(-1500, 2000);


    //align with far edge of black tape
    doubleSquareUp(2, 2);
    msleep(50);
    set_absolute_theta(0);
    msleep(20);

    //turn almost all the way around, leaving some tolerance for almost no reason
    twg(-165);

    //drive for a bit because its faster than double square uping
    msleep(50);
    dwg(-1500, 1500);


    //perform long square up
    doubleSquareUp(3, 1);

    //update absolute theta while were nicely aligned
    set_absolute_theta(0);
    msleep(50);

    //drive to align with ramp
    dwg(-1000, 1200);
    msleep(250);

    //turn so that back faces the ramp but misses coupler

    set_timeout(4000);
    twg(60);
    set_timeout(120000);
    msleep(100);

    //drive onto ramp
    dwg(-850, 1600);


    //why isn't this a twg? I don't know

    int beginCoupSec = seconds();
    while(get_absolute_theta() < 90 && seconds() - beginCoupSec < 3){
        mav(rm, 300);   
        mav(lm, -150);
    }
    freeze(rm);
    freeze(lm);
    msleep(250);

    //drive back a bit to firmly secure on ramp
    dwg(-1200, 2000);


    //align with the black line on the ramp
    /*if(analog(llf) < blackOnBrownThreshold){
        int alignSec = seconds();
        while(analog(llf) < blackOnBrownThreshold && seconds() - alignSec < 4){
            mav(rm, -300);
        }
        freeze(rm);
    }*/



    int alignSecR = seconds();
    while(analog(rlf) < blackOnBrownThreshold && seconds() - alignSecR < 2){
        mav(rm, -300);
    }
    freeze(rm);




    printf("\n\n\n\nAbsolute Theta: %f", get_absolute_theta());
    set_mode(1);
    msleep(50);


    //turn to absolute 91, slightly overshooting the line to ensure the dua sensor will hit the line
    //twg(91);
    set_mode(0);
    freeze(lm);
    freeze(rm);
    printf("\n\n\n\nAbsolute Theta: %f", get_absolute_theta());

    /*int singleLFSec = seconds();
    while(seconds() - singleLFSec < 1){
     	if(analog(rlf) < blackOnBrownThreshold){
         	mav(lm, -400);   
        }else{
         	mav(lm, 400);   
        }
    }
    */
    mav(rm, -300);
    msleep(200);
    freeze(rm);




    int startDuaSeconds = seconds();

    //starts accel averaging thread
    startWatch();
    int duaLength = 7000;


    //dua, but run the if code if broken prematurely
    //the block itself moves the right sensor past the black tape
    //so that our trajectory doesn't hit the ramp wall
    //return 0;


    double beginTheta = get_absolute_theta();

    printf("\nAbs theta beginning ascent %f\n", beginTheta);

    int paraAlignSec = seconds();
    int afterEdge = 0;

    while(seconds() - paraAlignSec < 7){


        if(analog(rlf) > blackThreshold && analog(blf) > backLFThreshold){
            int verifySensor = 0;
            int fake = 0;
            while(verifySensor < 100){
                printf("\n&&&&&&VERIFYING SENSOR&&&&&&&&\n");
                if(analog(rlf) > blackThreshold && analog(blf) > backLFThreshold){
                    verifySensor++;
                }else{
                    fake = 1;
                    verifySensor = 101;
                }
            }

            if(!fake){

                printf("\n <<<<<<<<<<<Edge Case>>>>>>>\n");
                int edgeCaseSec = seconds();
                while(analog(rlf) > blackThreshold && seconds() - edgeCaseSec < 5){
                    mav(rm, 400);   
                }
                freeze(rm);
                afterEdge++;
                break;
            }

        }


        if(afterEdge > 0){
            break;
        }	

        int lfCheckSec = seconds();
        while(analog(rlf) < blackThreshold && seconds() - lfCheckSec < 3){
            mav(lm, -1400);   
        }

        freeze(lm);

        int lfCheckSec2 = seconds();
        while(analog(blf) < backLFThreshold && seconds() - lfCheckSec2 < 3 && get_absolute_theta() - beginTheta > -5){
            mav(rm, -1400);
        }
        freeze(rm);
    }


    printf("Broke parallel lf\n");

    if(afterEdge == 0){
        printf("No edge case \n");
        int insurance = seconds();
        while(analog(rlf) < blackThreshold && seconds() - insurance < 3){
            mav(lm, -1400);   
        }
    }

    freeze(lm);

    //rampApproach(duaLength, startDuaSeconds, 0);
    //printf("\nBroke ramp approach\n");


    /*
    int badIdeaSeconds = seconds();
    while(seconds() - badIdeaSeconds < 7){

        if(((int)seconds() - badIdeaSeconds) % 2 == 0){
            while(analog(rlf) < blackOnBrownThreshold){
                mav(lm, -1400);
                mav(rm, -200); 
            }

            while(analog(rlf) > blackOnBrownThreshold){
                mav(rm, -1400);
                mav(lm, -200); 
            }
        }else{
             while(analog(llf) < blackOnBrownThreshold){
                mav(rm, -1400);
                mav(lm, -200); 
            }

            while(analog(llf) > blackOnBrownThreshold){
                mav(lm, -1400);
                mav(rm, -200); 
            }
        }
    }
*/
    freeze(lm);
    freeze(rm);

    msleep(500);

    //stops updating the average
    endWatch();

    printf("\n\n movingAverage %f", gaa());

    int beginHASec = seconds();

    //drive backward until significant forward accel is detected.
    //i.e, collides with wall.

    mav(lm, -1500);
    mav(rm, -1500);
    msleep(6000);


    while(accel_y() < -150 && seconds() - beginHASec < 2){
        printf("HACC: %d\n", accel_y());
        mav(lm, -1500);
        mav(rm, -1500);
    }



    //drive forward a little so that twg doesn't run us off the platform
    dwg(500, 500);


    //turn towards mine
    twg(-90);

    //hard align on pvc parallel to ramp
    mav(lm, -800);
    mav(rm, -800);
    msleep(1500);

    //set absolute theta value so that we can twg after
    //sucking and align with black line
    set_absolute_theta(0);
    msleep(50);

    printf("\n\n\nABS THETA AFTER PARALLEL EDGE HARD ALIGN: %f\n", get_absolute_theta());


    //drive forward to pass the dreaded bump
    dwg(1000, 2000);



    int moveRelDelay = seconds();


    //linefollow for five seconds
    while(seconds() - moveRelDelay < 4){
        if(analog(rlf) < blackThreshold){
            mav(rm, 800);
            mav(lm, 200);
        }else{
            mav(lm, 800);
            mav(rm, 200);

        } 
    }

    mav(lm, 0);
    mav(rm, 0);




    //stop lf to moverel to absolute position 600
    //note that this is the best way to moverel to absolute position
    moveRel(-1 * (initBalMoveVal - 500));


    //continue until rangefinder finds the wooden shield of mine
    while(analog(0) < 1900){


        if(analog(rlf) < blackThreshold){
            mav(rm, 800);
            mav(lm, 200);
        }else{
            mav(lm, 800);
            mav(rm, 200);

        }


    }
    freeze(lm);
    freeze(rm);
    msleep(250);



    set_absolute_theta(0);
    msleep(50);


    //drive foward to put bin within mine
    dwg(1400, 650);
    msleep(50);



    int lowpos = 90;

    //moverel to absolute 100
    moveRel(-get_servo_position(reg) + lowpos);
    msleep(250);
    //snap to absolute 100 to ensure we're as low as possible
    balMove(lowpos);
    msleep(250);

    set_mode(1);

    int iters = 0;
    startTime = seconds();
    printf("%i", seconds);
    while(seconds() - startTime < 7){

        //This commented block moves bin up and down infrequently.
        //not enough time anymore.
        /*if(iters % 10000 > 9000 && iters % 10000 < 9500){
            moveRel(5);
            //msleep(200);
        }else if(iters % 10000 > 9000){
            moveRel(-5);
            //msleep(200);
        }*/


        //200/5000 times, push stuff out
        if(iters % (5000) > 4800){
            mav(0, 1300);

        }else{
            mav(0, -1300);
        }


        //printf("%f\n", sin((2*3.14 * iters)/(1500)));
        mav(lm, -250 * osc1(iters));
        mav(rm, 250 * osc2(iters));

        iters++;

        iters++;
    }
    mav(0, 0);

    set_timeout(3000);
    //twg to absolute 0, the value we set before the suck
    turn_with_gyro_advanced(0, 1500, 12, 0, 0);
    printf("\n\n\nABS THETA AFTER POST SUCK TURN: %f\n", get_absolute_theta());
    set_timeout(120000);


    //stop bucket motor
    mav(0, 0);

    dwg(-800, 50);


    set_mode(0);
    msleep(20);

    //pick bucket up and move backwards out of the mine
    int desired = get_servo_position(reg) + 525;
    printf("\n desired pos %i\n", desired);

    balMove(desired);
    msleep(500);
    printf("Servo position exiting bucket: %i\n", get_servo_position(reg));
    dwg(-850, 2000);

    /* if(analog(rlf) > blackThreshold){
     	while(analog(rlf) > blackThreshold){
         	mav(rm, -200);   
        }
    }else{
     	 while(analog(rlf) < blackThreshold){
         	mav(lm, -200);   
        }  
    }*/


    int beginBSec = seconds();

    if(duaac(-1400, llf, rlf, blackThreshold, 2250)){ //returns 1 if hit black
        printf("***************************************broke at line lf: %i, rf: %i\n", analog(llf), analog(rlf));


        int pSec = seconds();
        while(analog(llf) > blackOnBrownThreshold && seconds() - pSec < 3){
            mav(lm, 400); 
        }

        int pbSec = seconds();
        while(analog(rlf) > blackOnBrownThreshold && seconds() - pSec < 3){
            mav(rm, 400); 
        }

        freeze(rm);
        freeze(lm);

        dwg(-1400, 1000);
    }


    moveRel(-300);
    mav(lm, -800);
    mav(rm, -800);
    msleep(2500);
    moveRel(300);
    moveRel(-400);
    moveRel(400);
    balMove(900);
    msleep(300);

    int shortLfSec = seconds();
    while(accel_y() > (gaa()/2) && seconds() - shortLfSec < 3){
        mav(lm, -800);
        mav(rm, -800);
    }


    dwg(1000, 500);
    msleep(50);
    set_timeout(2000);
    twg(90);
    set_timeout(120000);

    dwg(-500, 500);

    mav(lm, 800);
    msleep(400);


    while(analog(rlf) > 160){ //arbitrary threshold for white board. Brown seemed to always exceed 200
        if(analog(rlf) < blackThreshold){
            mav(rm, 800);
            mav(lm, 0);
        }else{
            mav(lm, 800);
            mav(rm, 0);
        }
    }

    freeze(lm);
    freeze(rm);

    mav(lm, 800);
    mav(rm, 800);
    msleep(2500);

    twg(-90);

    twg(30);

    dwg(800, 1000);

    twg(-30);


    int weGonnaMiss = 0;
    if(analog(3) > 1500){
        printf("WERE GONNA MISS");   
        weGonnaMiss = 1;
        mav(lm, 800);
        msleep(800);
    }

    dwg(800, 2000);

    dua(-1000, llf, blackThreshold);
    dwg(-200, 700);

    int startTheta = get_absolute_theta();

    printf("\nTheta during squp: %d\n", startTheta);

    set_absolute_theta(0);
    doubleSquareUp(3.5, -1);
    printf("\nTheta after squp: %d\n", get_absolute_theta());

    twg(17 - get_absolute_theta());

    if(!weGonnaMiss){
        dwg(-1400, 300);
    }

    moveRel(-(get_servo_position(reg) - 720));
    mav(0, 900);
    msleep(5000);
    mav(0, -600);
    msleep(2000);
    mav(0, 1000);
    msleep((119 - (seconds() - startRunSeconds) ) * 1000);


    printf("\n\nrun time: %f\n", (seconds() - startRunSeconds));
    ao();
    return 0;

    int beginBackLf = seconds();
    while(seconds() - beginBackLf < 4){
        if(analog(llf) < blackThreshold){
            mav(rm, -1400);
            mav(lm, -100);
        }else{
            mav(lm, -1400);
            mav(rm, -100);

        }

    }

    int beginBackHardalign = seconds();
    while(seconds() - beginBackHardalign < 3){
        mav(rm, -1400);
        mav(lm, -1400);
    }




    disable_servos();
    msleep(500);
    ao();
    return 0;
}
