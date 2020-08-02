#include <kipr/botball.h>
int main6(){
 	mav(0, 606);
    mav(3, 536);
	msleep(20000);
}


int main()
{
    printf("Press rbutton to record\n");
	while(!right_button()){}
    
    printf("\n\n\n\n");
    
    int maxIters = 200;
    
    int i = 0;
    
    int pos1[maxIters];
    int pos2[maxIters];
    cmpc(0);
    cmpc(3);
    
    while(i < maxIters){
        
        msleep(100);
    	pos1[i] = gmpc(0);
		pos2[i] = gmpc(3);
        printf("%i: %i %i \n", i, pos1[i], pos2[i]);
    	i++;
    }
    printf("\nfinished recording\n");
    int j;
    int firstLastPos = 0;
    int secondLastPos = 0;
    
    while(!right_button()){}
    
    cmpc(0);
    cmpc(3);
    
    for(j = 0; j < maxIters; j++){
       
        printf("Target: %i, %i, Diff: %i, %i\n", pos1[j], pos2[j], pos1[j] - firstLastPos, pos2[j] - secondLastPos);
        mav(0, 10 * (pos1[j] - firstLastPos));
        mav(3, 10 * (pos2[j] - secondLastPos));
        
        msleep(100);
        ao();
        double p1 = (double) gmpc(0)/ (double)pos1[j];
        double p2 = (double) gmpc(3)/ (double)pos2[j];
        printf("Accuracy0 = %f \n", p1);
        printf("Accuracy3 = %f \n", p2);
     
        
        firstLastPos = pos1[j];
        secondLastPos = pos2[j];
        
        
    }
    
    
    return 0;
}
