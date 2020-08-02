#include <kipr/botball.h>

int main()
{

    int blackThreshold = 1000;

    int sensorPort = 2;

    int previousError = 0;

    double integral = 0;

    double kp, ki, kd;

    //complete guesses
    kp = 0.6; //magnitude of error will be between 0 and 2800, about half seems reasonable to be within motor velocities 
    ki = 0.01; //the times 4 rule. May be opposite direction
    kd = 0.0025;

    double t0 = seconds();

    int iters = 0;

    double speed = 800;


    int rm = 3;
    int lm = 2;

    ki = 0;
    kd = 0;
    
    while(1){
        double mult = 1;
        double beginBlackSec = seconds();
        double errorHigh;
        double integralHigh = 0;
        double t0 = seconds();
        double previousError = 0;

        int i;
        int k = 0;
        for(i = -1; i <= 1; i += 2){
            while(((analog(2) - blackThreshold) * i) < 0){ 
                //this is a cool hack to not repeat code.
                //if i == -1 you are on black, otherwise white.
                //determined by blackThreshold, of course
                int onBlack = (i == -1);
                k++;
                if(k % 50 == 0)
                    printf("running on %s\n", (i == -1) ? "high" : "low");

                errorHigh = seconds() - beginBlackSec; // error
                integral += errorHigh / (seconds() - t0);
                double derivative = (errorHigh - previousError) / (seconds() - t0);
                iters++;
                mult = 1 - errorHigh; // equivalent to p*kp;

                double pid = (speed * mult);// + (integral * ki) + (derivative * kd);


                if(onBlack){
                    mav(rm, speed);
                    mav(lm, pid);
                }else{
                    mav(lm, speed);
                    mav(rm, pid);
                    
                }
                if(iters % 25 == 0){
                    printf("New vel: %f\n", speed * mult);
                    printf("Mult %f, I: %f, D: %f, PID: %f\n", mult, integral, derivative, pid);   
                }
                t0 = seconds();
                previousError = errorHigh;


            }
            
            beginBlackSec = seconds();
            errorHigh = 0;
            integralHigh = 0;
            previousError = 0;
            t0 = seconds();
        }
        continue;
        printf("after continue");

        while(analog(2) > blackThreshold){
            errorHigh = seconds() - beginBlackSec; // error
            integral += errorHigh / (seconds() - t0);
            double derivative = (errorHigh - previousError) / (seconds() - t0);
            iters++;
            mult = 1 - errorHigh; // equivalent to p*kp;

            double pid = (speed * mult) + (integral * ki) + (derivative * kd);

            mav(rm, speed);
            mav(lm, pid);
            if(iters % 25 == 0){
                printf("New vel: %f\n", speed * mult);
                printf("Mult %f, I: %f, D: %f, PID: %f\n", mult, integral, derivative, pid);   
            }
            t0 = seconds();
            previousError = errorHigh;
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