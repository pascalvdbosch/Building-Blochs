#ifndef __LOG_H
#define __LOG_H

#include "config.h"

// Helpers
#ifndef __STRINGIFY
#  define __STRINGIFY(x)    #x
#endif
#define ___STRINGIFY(x) __STRINGIFY(x)
#define __LINESTR__ ___STRINGIFY(__LINE__)

#define DEBUG_DEVICE        Serial
#define DBG(msg, ...)      DEBUG_DEVICE.printf("%s.%s: " msg "\n", __FILE__, __FUNCTION__, ##__VA_ARGS__)
#define INFO(msg, ...)	   DEBUG_DEVICE.printf("%s: " msg "\n", __FUNCTION__, ##__VA_ARGS__)
#define WARNING(msg, ...)  DEBUG_DEVICE.printf("WARN :" __FILE__ ".%s(" __LINESTR__ "): " msg "\n", __FUNCTION__, ##__VA_ARGS__)
#define ERROR(msg, ...)    DEBUG_DEVICE.printf("ERROR:" __FILE__ ".%s(" __LINESTR__ "): " msg "\n", __FUNCTION__, ##__VA_ARGS__)

#endif // __LOG_H