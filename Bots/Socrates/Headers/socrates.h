const int reg = 3; //mainServo
const int neg = 0; //offMainServo

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
int backLFThreshold = 300;


int initBalMoveVal = 1601;



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

    int i;
    for(i = 0; i < steps; i++){

        set_servo_position(reg, get_servo_position(reg) + (chunkSize * mult));
        set_servo_position(neg, get_servo_position(neg) - (chunkSize * mult));
        
        msleep(30);
    }
    msleep(200);

    set_servo_position(reg, initPosReg + (pos*mult));
    set_servo_position(neg, initPosNeg - (pos * mult));
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