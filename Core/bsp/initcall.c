#define INITCALL_GLOBALS
                    
#include	"Types.h"
#include <initcall.h>  
                
void DemoAppInit(void)
{
}
void DemoAppInitStart(void)
{
}
void DemoAppInitEnd(void)
{
}
AppInitCall(DemoAppInitStart);
AppInitCall(DemoAppInitEnd);
AppInitCall(DemoAppInit);
   
extern int Image$$MY_REGION$$Base;
extern int Image$$MY_REGION$$Limit;
void AppCallInit(void){   
    INT32U *MyRegion_Start = (INT32U *)&Image$$MY_REGION$$Base;
    INT32U *MyRegion_End = (INT32U *)&Image$$MY_REGION$$Limit;
    for(INT32U *pAddr = MyRegion_Start; pAddr < MyRegion_End;){  
        //INITCALL_DEBUG(("AppCallInit pAddr =%d \r\n", *pAddr)); 
        ((INIT_CALL)*pAddr)();
         *pAddr++;
    }   
	INITCALL_DEBUG(("AppCallInit init over\r\n"));
}




