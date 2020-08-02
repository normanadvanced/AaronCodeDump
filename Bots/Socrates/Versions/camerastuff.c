#include <kipr/botball.h>

int main()
{

    int couplerChannel = 0;
    int lineChannel = 1;

    int lm = 2;
    int rm = 3;


    camera_open();


    camera_load_config("soc");

    msleep(500);

    int p = 0;
    while( p == 0){
        camera_update();
        camera_update();
        camera_update();


        msleep(500);

        p = get_object_count(lineChannel);
        
        printf("\n\n%i\n", get_object_count(lineChannel));

        msleep(500);
    }

    
//    printf("Camera height %i", get_camera_width());

    
    double pidLoopSec = seconds();
    int camPreviousError = 0;
    int height = 120;
    double camIntegral = 0;
    double t0 = seconds();

    double kp = 3;
    double ki = 2;
    double kd = 2;

    do{
        camera_update();

        double dt = seconds() - t0;
        int objCount = get_object_count(lineChannel);
        double avgPos = 0;
        
        int o;
        for(o = 0; o < objCount; o++){
            avgPos +=  get_object_center(lineChannel, o).y;
        }
        
        avgPos = avgPos / objCount;
        
        
        double error = (height/2) - avgPos;
        
        
        printf("Avg pos %f\n", avgPos);
        
        camIntegral += error * dt;
        double camDerivative = (error - camPreviousError);
        double output = kp * error + ki * camIntegral + kd * camDerivative;

        printf("Error %f, from pos %i on camera height %f\n", error, get_object_center(lineChannel, 0).y, get_camera_height());
        printf("Output %f \n", output);

        mav(lm, -output);
        mav(rm, output);

        camPreviousError = error;
        t0 = seconds();
        
        camera_update();
        msleep(100);
    }while(seconds() - pidLoopSec < 30);

    return 0;
}
