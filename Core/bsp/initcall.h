#include "Types.h"

#ifndef    __INITCALL_H_
#define    __INITCALL_H_

#if  DBG_INITCALL
	#define INITCALL_DEBUG( X)   do {DPrint X ;} while(0)
#else
	#define INITCALL_DEBUG( X)    
#endif
		
#ifndef INITCALL_GLOBALS
	#define EXT_INITCALL                   extern
#else
	#define EXT_INITCALL
#endif


typedef void (*INIT_CALL)(void);
#define __define_initcall(level, secAlias, fn) \
    static const INIT_CALL __initcall_##level##fn \
   __attribute__((__section__(".initcall." #level "." secAlias ".init"))) = fn

#define CoreInitCall(fn) __define_initcall(1, "core",fn) 

#define AppInitCall(fn) __define_initcall(2, "app",fn)  


EXT_INITCALL void AppCallInit(void);   
#endif
