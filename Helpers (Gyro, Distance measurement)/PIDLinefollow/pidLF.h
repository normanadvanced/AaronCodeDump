void pidLF(double timeInMS, int speed, int sensorPort, int blackThreshold, int lm, int rm){
    if(timeInMS < 1000){
        printf("[pidLf.h]: pLF accepts time in milliseconds.\n You have given an unusually small amount of milliseconds to linefollow for.\n");   
    }

    int iters = 0;


    double beginTimeMS = seconds();
    while((seconds() - beginTimeMS) * 1000 < timeInMS){
        double mult = 1;
        double timeAtCrossover = seconds();
        double error;
        double integral = 0;
        double t0 = seconds();
        double previousError = 0;

        int i;
        int k = 0;
        for(i = -1; i <= 1; i += 2){
            int onBlack = (i == -1);
            double pid;
            while(((analog(2) - blackThreshold) * i) < 0){ 
                //this is a cool hack to not repeat code.
                //if i == -1 you are on black, otherwise white.
                //determined by blackThreshold, of course

                if(++k % 50 == 0)
                    //printf("running on %s\n", (i == -1) ? "high" : "low");

                    error = seconds() - timeAtCrossover; // error

                integral += error / (seconds() - t0);
                double derivative = (error - previousError) / (seconds() - t0);
                iters++;
                mult = 1 - error * (speed / 1000.0); // equivalent to p*kp;
                pid = (speed * mult);// + (integral * ki) + (derivative * kd);
                if(onBlack){
                    mav(rm, speed);
                    mav(lm, pid);
                }else{
                    mav(lm, speed);
                    mav(rm, pid);

                }
                t0 = seconds();
                previousError = error;
            }
            timeAtCrossover = seconds();
            error = 0;
            integral = 0;
            previousError = 0;
            t0 = seconds();
        }
    }
}

void plf(double timeInMS, int speed, int sensorPort, int blackThreshold){
    if(left_motor == right_motor){ //global variables from gyrolib
        printf("[pidLf.h]: plf requires gyrolib to be included and ig to be called.\n");
        printf("[pidLF.h]: if you do not wish to use gyrolib, use the pidLf function instead\n");
    }
    pidLF(timeInMS, speed, sensorPort, blackThreshold, left_motor, right_motor);   
}
