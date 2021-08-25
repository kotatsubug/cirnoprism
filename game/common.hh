#pragma once

#include <stdio.h>

#define _DEBUG 1

#define LOG_FATAL "FATAL"
#define LOG_ERROR "ERROR"
#define LOG_WARNING "WARNING"
#define LOG_SUCCESS "SUCCESS"

#define DEBUG_LOG(category, level, message, ...) \
	fprintf(stderr, "[%s] ", category); \
	fprintf(stderr, "[%s] (%s:%d): ", level, __FILE__, __LINE__); \
	fprintf(stderr, message, ##__VA_ARGS__); \
	fprintf(stderr, "\n")

#define EXPECT(expr) if (!expr) DEBUG_LOG("Common", LOG_ERROR, "Assertion failed")

template <typename T, size_t N>
char (&ArraySizeHelper(T(&arr)[N]))[N];
#define ARRAY_SIZE_NOEXCEPT(array) (sizeof(ArraySizeHelper(arr)))