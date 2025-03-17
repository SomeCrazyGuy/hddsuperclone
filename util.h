#ifndef HDDSUPERCLONE_UTIL_H
#define HDDSUPERCLONE_UTIL_H

#include <stdbool.h>

// do not add more log levels without modifying util.c
#define UTIL_LOG_LEVEL_INFO 0
#define UTIL_LOG_LEVEL_WARN 1
#define UTIL_LOG_LEVEL_ERROR 2
#define UTIL_LOG_LEVEL_MAX 3


// implementation of logger, do not call directly
// use INFO, WARN, ERROR instead
extern void util_log_impl(int log_level, const char* const file, const char* const func, const int line, const char* const format, ...);


// set log level at runtime, default is UTIL_LOG_LEVEL_INFO
// if you want to disable logging, set it to UTIL_LOG_LEVEL_MAX
extern void util_set_log_level(int log_level);


// you can disable logging at compile time on a per file basis
// by defining UTIL_DISABLE_INFO, UTIL_DISABLE_WARN or UTIL_DISABLE_ERROR
// or disable all logging by defining UTIL_DISABLE_LOGGING
#ifdef UTIL_DISABLE_LOGGING
  #define UTIL_DISABLE_INFO
  #define UTIL_DISABLE_WARN
  #define UTIL_DISABLE_ERROR
#endif


#ifdef UTIL_DISABLE_INFO
  #define INFO(...) do {} while(0)
#else
  #define INFO(...) util_log_impl(UTIL_LOG_LEVEL_INFO, __FILE__, __func__, __LINE__, "" __VA_ARGS__)
#endif


#ifdef UTIL_DISABLE_WARN
  #define WARN(...) do {} while(0)
#else
  #define WARN(...) util_log_impl(UTIL_LOG_LEVEL_WARN, __FILE__, __func__, __LINE__, "" __VA_ARGS__)
#endif


#ifdef UTIL_DISABLE_ERROR
  #define ERROR(...) do {} while(0)
#else
  #define ERROR(...) util_log_impl(UTIL_LOG_LEVEL_ERROR, __FILE__, __func__, __LINE__, "" __VA_ARGS__)
#endif


// create a hexdump of the data from `in_data` and write it to `out_buffer`
// this function will not write more than `out_size` bytes to `out_buffer`
// `out_buffer` is guaranteed to be NULL terminated
extern void util_hexdump_to_buffer(char* out_buffer, unsigned out_size, const void* in_data, unsigned in_size);

// create a hexdump of the data from `in_data` and write it to `output_file`
extern void util_hexdump_to_file(FILE* output_file, const void* in_data, unsigned in_size);


#endif //HDDSUPERCLONE_UTIL_H