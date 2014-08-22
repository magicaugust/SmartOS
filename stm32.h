#ifndef __STM32_H
#define __STM32_H

#ifdef STM32F0XX_LD
    #ifndef STM32F0XX
        #define STM32F0XX
    #endif
#endif

#ifdef STM32F10X_HD
    #ifndef STM32F10X
        #define STM32F10X
    #endif
#endif

#ifdef STM32F0XX
    #ifndef STM32F0
        #define STM32F0
    #endif
#else
    #ifdef STM32F0
        #define STM32F0XX
    #endif
#endif

#ifdef STM32F10X
    #ifndef STM32F1
        #define STM32F1
    #endif
#else
    #ifdef STM32F1
        #define STM32F10X
    #endif
#endif

#ifdef STM32F4XX
    #ifndef STM32F4
        #define STM32F4
    #endif
#else
    #ifdef STM32F4
        #define STM32F4XX
    #endif
#endif

#if defined(STM32F4)
	#define HSE_VALUE    ((uint32_t)8000000) /*!< 定义晶振频率为8MHz */
	#include "stm32f4xx.h"
#elif defined(STM32F2)
	#include "stm32f2xx.h"
#elif defined(STM32F1)
	#include "stm32f10x.h"
#elif defined(STM32F3)
	#include "stm32f3xx.h"
#elif defined(STM32F0)
	#include "stm32f0xx.h"
#else
	#include "stm32f10x.h"
#endif

#endif
