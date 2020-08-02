#include <kipr/botball.h>
#include <pthread.h>
#include <time.h>


int gyro_bias = -1;

double thetaCount = 0;


int motorBiasProp = 1.28;


void calibrateGyro(){
    int i;
    for(i = 0; i < 500; i++){
     	gyro_bias += gyro_z();
        msleep(10);
    }
    msleep(10);

    gyro_bias = gyro_bias/500;
    printf("Gyro bias %d", gyro_bias);
}

double kipr90 = 0;

double kiprDegConversion(){
    printf("Turn 360 degrees and hit rbutton");
    double theta = 0;
    while(right_button() != 1){
        double tT = (gyro_z()-gyro_bias)*10;
        theta += tT;
        //printf("%f ", tT);
        msleep(10);
    }
    printf("\n%f\n", theta);
    printf("Turned %f kipr degrees\n", theta);
    printf("Meaning 90 degrees equals %f kipr degrees. In kdg/ms\n\n", theta/4);
    kipr90 = theta/4;
    return kipr90;
    
}


void trackTheta(){
 	while(1){
        double ttt = (gyro_z() - gyro_bias) * 20;
    	thetaCount += ttt;
        msleep(20);
    }
}


void clearTheta(){
 thetaCount = 0;   
}


void turn(double deg){
	clearTheta();
    int ninetyDegInKipr = 595000;
    int totalTheta = (deg / 90.0) * ninetyDegInKipr;
    while(abs(thetaCount) < abs(totalTheta)){
        
        //double mVel = (thetaCount - totalTheta) * ((100 - 400) /  (0 - totalTheta)) + 100;
        int maxVel = 700;
        double mVel = (-maxVel/(totalTheta*1.1))*(thetaCount) + maxVel;
    	mav(0, (int) mVel);
        mav(3, (int)-mVel);
        msleep(50);
        printf("%i/%f, ", totalTheta, thetaCount);
    }
    //printf("%f vs %f", thetaCount, kipr90);    
	
    clearTheta();
    ao();
    
}

void dwg(double seconds, int speed, int increaseGMotor, int decreaseGMotor){
 	int iters = 0;
    clearTheta();
    int lastGyro = gyro_z();
    while(iters < seconds*100.0){
        //printf("%f ", speed*(thetaCount/250000));
        //double increaseSpeed = 0.000000007 * thetaCount * thetaCount + speed;
        if(thetaCount >= 0){
            
        double increaseSpeed = abs(1250 * (1/(1+pow(2.71, -0.000009*abs(thetaCount))))) - (625 - speed);
            
            printf("high ");
        	mav(increaseGMotor, speed * motorBiasProp);
            mav(decreaseGMotor, increaseSpeed);
        }else{
            speed = speed * motorBiasProp;
        double increaseSpeed = 1.2 * abs(1250 * (1/(1+pow(2.71, -0.000009*abs(thetaCount))))) - (625 - speed);
            
            printf("low ");
         	mav(increaseGMotor, increaseSpeed);
            mav(decreaseGMotor, speed);
        }
        iters++;
        lastGyro = gyro_z();
    	msleep(10);    
    }
    printf("Total Theta: %f", thetaCount);
	clearTheta();
}

void findMotorBias(int nonBiasedMotor, int biasedMotor, int speed){
 	int iters = 1;
    int avgGyro = 0;
    mav(nonBiasedMotor, speed);
    mav(biasedMotor, speed);
    msleep(2000);
    avgGyro = gyro_z();
    int gyroDir = gyro_z() > 0;
    int newVel = speed;
    while(abs(gyro_z()) > 5 && iters < 5000){
        //printf("%o", !(gyro_z() < 5 && gyro_z() > -5));
        newVel += (int) abs(gyro_z()/10);
     	mav(nonBiasedMotor, speed);
        mav(biasedMotor, newVel);
        printf("%i ", gyro_z());
        msleep(10);
    }
    printf("\n\n\n\nCorrected: %f, \n", (newVel) / (double) speed);
    printf("Gyro val: %i\n", gyro_z());
    printf("ITERS:_%i \n\n", iters);
    motorBiasProp = (newVel) / (double) speed;
    ao();
    msleep(1000);
    int p;
    for(p = 0; p < 1000; p++){
    mav(nonBiasedMotor, speed);
    mav(biasedMotor, speed * motorBiasProp);
    printf("%i, ", gyro_z());
        msleep(10);
    }
}

void fMD(int badMotor, int goodMotor){
 	
    int i;
    
    int goodGyroAvg = 0;
    
    for(i = 0; i < 300; i++){
        mav(goodMotor, 300);
        goodGyroAvg += gyro_z();
        msleep(10);
    }
    ao();
    int badGyroAvg = 0;
    msleep(1000);
    for(i = 0; i < 300; i++){
        mav(badMotor, 300);
        badGyroAvg += gyro_z();
        msleep(10);
    }
    
    badGyroAvg = badGyroAvg/300;
    goodGyroAvg = goodGyroAvg/300;
    double prop = (double)goodGyroAvg/badGyroAvg;
    motorBiasProp = abs(prop);
    printf("Good gyro avg: %i, Bad: %i, so the prop: %f", goodGyroAvg, badGyroAvg, prop);
    ao();
    msleep(500);
    for(i = 0; i < 200; i++){
    mav(badMotor, abs(700*prop));
    mav(goodMotor, 700);
    printf("\n%i", gyro_z());
    msleep(10);
    }
}

thread thetaCounterThread;

int main(){
    thetaCounterThread = thread_create(trackTheta);
	thread_start(thetaCounterThread);
    turn(90);
    thread_destroy(thetaCounterThread);
    return 0;
    
}

int exampleMain(){
    thetaCounterThread = thread_create(trackTheta);
	thread_start(thetaCounterThread);
    dwg(15, 500, 0, 3);
    ao();
    msleep(1000);
    turn(90);
    dwg(6, 500, 0, 3);
    thread_destroy(thetaCounterThread);
    return 0;
}