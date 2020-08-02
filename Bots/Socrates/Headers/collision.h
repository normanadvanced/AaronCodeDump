#include <kipr/botball.h>
#include <pthread.h>


thread accelThread;

double average = 0;

void watchCollision(){
    int iters = 0;
    int gracePeriod = 3000;
    double last = 0;
    
    average = 0;
    while(1){
            average = average + ((accel_y() - average)/(iters+1));
        
        
        if(iters < gracePeriod){
            if(iters % 100 == 0){
        	//printf("val %f\n", (accel_y() - average)/(iters+1));
            printf("avg: %f\n", average);
            }
        }
        
        iters++;
    }
}


double gaa(){
 	return average; 
}

void startWatch(){
    accelThread = thread_create(watchCollision);
    thread_start(accelThread);
}

void endWatch(){
    thread_destroy(accelThread);
}