#pragma once

#include <stdio.h>

#define LOG_FATAL "FATAL"
#define LOG_ERROR "ERROR"
#define LOG_WARN "WARN"
#define LOG_SUCCESS "SUCCESS"

#define DEBUG_LOG(category, level, message, ...) \
	fprintf(stderr, "[%s] ", category); \
	fprintf(stderr, "[%s] (%s:%d): ", level, __FILE__, __LINE__); \
	fprintf(stderr, message, ##__VA_ARGS__); \
	fprintf(stderr, "\n")

#define DEBUG

#ifdef DEBUG
#define EXPECT(condition)\
{\
	do\
	{\
		if (!(condition))\
		{\
			DEBUG_LOG("Common", LOG_ERROR, "Assertion failed! [%s]", (#condition));\
		}\
	} while(0);\
}
#else  
#define EXPECT(condition)\
{\
	do\
	{\
		(void)sizeof(condition);\
	} while(0);\
}
#endif




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


#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i) \
	(((i) & 0x80ll) ? '1' : '0'), \
	(((i) & 0x40ll) ? '1' : '0'), \
	(((i) & 0x20ll) ? '1' : '0'), \
	(((i) & 0x10ll) ? '1' : '0'), \
	(((i) & 0x08ll) ? '1' : '0'), \
	(((i) & 0x04ll) ? '1' : '0'), \
	(((i) & 0x02ll) ? '1' : '0'), \
	(((i) & 0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
	PRINTF_BINARY_PATTERN_INT8 PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
	PRINTF_BYTE_TO_BINARY_INT8((i) >> 8), PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32 \
	PRINTF_BINARY_PATTERN_INT16 PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
	PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), PRINTF_BYTE_TO_BINARY_INT16(i)
#define PRINTF_BINARY_PATTERN_INT64 \
	PRINTF_BINARY_PATTERN_INT32 PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i) \
	PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), PRINTF_BYTE_TO_BINARY_INT32(i)
