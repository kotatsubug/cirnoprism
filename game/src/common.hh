#pragma once

#include <stdio.h>

#define LOG_FATAL "FATAL"
#define LOG_ERROR "ERROR"
#define LOG_WARNING "WARN"
#define LOG_SUCCESS "SUCCESS"

#define DEBUG_LOG(category, level, message, ...) \
	fprintf(stderr, "[%s] ", category); \
	fprintf(stderr, "[%s] (%s:%d): ", level, __FILE__, __LINE__); \
	fprintf(stderr, message, ##__VA_ARGS__); \
	fprintf(stderr, "\n")

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))



#define OpenAL_ErrorCheck(msg)\
{\
	ALenum error = alGetError();\
	if (error != AL_NO_ERROR)\
	{\
		DEBUG_LOG("OpenAL", LOG_ERROR, "OpenAL throws [%i] with call for [%s]", error, (#msg));\
	}\
}

#define alec(FUNCTION_CALL)\
FUNCTION_CALL;\
OpenAL_ErrorCheck(FUNCTION_CALL)

