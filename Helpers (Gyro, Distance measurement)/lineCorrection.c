#include <kipr/botball.h>

int main()
{

    int blackThreshold = 1000;

    int sensorPort = 2;

    int previousError = 0;

    double integral = 0;

    double kp, ki, kd;

    //complete guesses
    kp = 0.4; //magnitude of error will be between 0 and 2800, about half seems reasonable to be within motor velocities 
    ki = 0.01; //the times 4 rule. May be opposite direction
    kd = 0.0025;

    double t0 = seconds();

    int iters = 0;

    double speed = speed;


    int rm = 3;
    int lm = 2;

    while(1){
        double mult = 1;
        double beginBlackSec = seconds();
        double errorHigh;
        while(analog(2) > blackThreshold){
            mav(rm, speed);
            mav(lm, speed * mult);
        	errorHigh = seconds() - beginBlackSec;
            iters++;
            mult = 1 - errorHigh;
            if(iters % 25 == 0){
             	printf("Mult %f\n", mult);   
            }
        }

        double beginWhiteSec = seconds();
        double errorLow;
        double multLow = 1;
        while(analog(2) < blackThreshold){
            mav(rm, speed * multLow);
            mav(lm, speed);
            errorLow = seconds() - beginWhiteSec;
            
            multLow = 1 - errorLow;
            
            if(iters % 50 == 0){
             	printf("Multlow %f\n", multLow);   
            }
            iters++;
        }



        iters++;

        if(1){
            printf("tot %f\n\n", errorHigh + errorLow);   
        }

    }

    while (1)
    {
        double dt = seconds() - t0;
        double error = blackThreshold - analog(sensorPort);
        integral = integral + (error * dt);
        double derivative = (error - previousError) / dt;

        double output = (kp * error) + (ki * integral) + (kd * derivative);

        previousError = error;


        if(iters % 50 == 0){

            printf("Error %f\n", error);
            /*printf("Integral %f\n", integral);
            printf("Derivitave %f\n", derivative);
            printf("P: %f, I: %f, D: %f\n", (kp * error), (ki * integral), (kd * derivative));
            printf("Total arbitrary ratio %f\n", output);*/
        }
        iters++;
        t0 = seconds();
        mav(2, output / (1 + abs(output)) * speed);
        mav(3, 0.25 * output / (1 + abs(output)) * 100);
        msleep(10);
    }
    return 0;
}