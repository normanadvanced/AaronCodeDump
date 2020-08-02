#include <kipr/botball.h>
#include <gyrolib.h>
#include <pidLF.h>
#include <collision.h>

#include <socrates.h> //include all 2 pom tom dependencies above


int main()
{
    int beginRunServoValue = 1650;
    printf("%i", deg);
    ig(lm, rm, deg);

    enable_servos();

    balMove(beginRunServoValue);

    dwg(1500, 3000);

    mav(lm, 500);
    mav(rm, 500);
    msleep(1000);

    mav(lm, 0);
    mav(rm, 0);

    dwg(-1500, 100);
    msleep(500);

    twg(90);

    mav(lm, -500);
    mav(rm, -500);
    msleep(3000);

    mav(lm, 0);
    mav(rm, 0);

    camera_load_config("soc");

    camera_open();

    int greatestArea = 9999;

    while(greatestArea > 1000){
        camera_update();

        printf("Num objects %i\n", get_object_count(couplerChannel));
        greatestArea =  get_object_area(couplerChannel, 0);
        printf("Largest object %i\n", greatestArea);

        msleep(1000);
    }

    msleep(2000);

    camera_update();
    msleep(50);
    
    
    const int centerValue = 70;
    

    double beginLineFind = seconds();
    
    int linePosition = 0;
    do{
        camera_update();
        int yval = 0;
        
        yval = get_object_center(lineChannel, 0).y;
        
        int objCount = get_object_count(lineChannel);
        
        printf("Line pos %i\n", yval);
        if(yval > centerValue){
         	mav(rm, 100);
            mav(lm, -100);
            msleep(1000);
            break;
        }else{
        	mav(rm, -100);
            mav(lm, 100);
            msleep(1000);
            break;
        }
		
    } while(seconds() - beginLineFind < 3);


    double pidLoopSec = seconds();
    int camPreviousError = 0;
    int height = get_camera_height();
    double camIntegral = 0;
    double t0 = seconds();

    double kp = 4;
    double ki = 2;
    double kd = 0.5;

    do{
        camera_update();

        dt = seconds() - t0;
        double error = (height/2) - get_object_center(lineChannel, 0).y;
        camIntegral += error * dt;
        double camDerivative = (error - camPreviousError);
        double output = kp * error + ki * camIntegral + kd * camDerivative;

        printf("Pid loop output %f", output);

        camPreviousError = error;
        t0 = seconds();
        msleep(100);
    }while(seconds() - pidLoopSec < 3);

    dwg(1500, 1000);






    return 0;
}
