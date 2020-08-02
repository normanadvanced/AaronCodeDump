#include <kipr/botball.h>


int driveCM(double dist, int weakMotor, int strongMotor, double diff){
        
    double wheelDiameter = 6.9;
    
    double PI = 3.14159;
    
    double wheelCircumference = PI * wheelDiameter;
    
    double ticksPerRev3 = 1508;
    
    double ticksPerRev0 = 1720;
    
    cmpc(0);
    cmpc(3);
    
    mtp(weakMotor, 500 * diff, dist/wheelCircumference * ticksPerRev0);
    mtp(strongMotor, 500, dist/wheelCircumference * ticksPerRev3);
    
    
    bmd(0);
    bmd(3);    
        
    mtp(weakMotor, 200, dist/wheelCircumference * ticksPerRev0);
    mtp(strongMotor, 200, dist/wheelCircumference * ticksPerRev3);
    
    bmd(0);
    bmd(3);
    
    mtp(weakMotor, 50, dist/wheelCircumference * ticksPerRev0);
    mtp(strongMotor, 50, dist/wheelCircumference * ticksPerRev3);
    
    bmd(0);
    bmd(3);
    msleep(100);
}



int main(){
	driveCM(30, 0, 3, 1.18);
    ao();
    msleep(1500);
        
    }
    

