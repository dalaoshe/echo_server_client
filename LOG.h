//
// Created by dalaoshe on 17-10-26.
//

#ifndef C_TEST_LOG_H
#define C_TEST_LOG_H


#include <cstdio>
#include <stdarg.h>
#include <sys/time.h>
#include <string>
#include <time.h>
#define MY_DEBUG_NAME "DEBUG"
#define MY_ERROR_NAME "ERROR"
#define MY_INFO_NAME "INFO"
#define MY_WARN_NAME "WARN"
class LOG {
    static LOG* log;
    static FILE* fd;
    LOG() {}
    LOG(const LOG& log) {}
    LOG& operator=(const LOG& log) {}

public:
    static LOG* init(FILE* fd=stderr) {
        if(log == NULL) {
            log = new LOG();
            log->fd = fd;
            return log;
        }
        else
            return log;
    }

    static void print(const char* file, const char* function, const int line, const char* level,
                      const char* format,  ...) {
        time_t timep;
        time (&timep);
        char tmp[64];
        strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&timep) );
        fprintf(fd,"[%s]\t[%s,%s:%d]\t%s:", tmp, file, function, line, level);
        va_list arg_ptr;
        va_start(arg_ptr, format);
        vfprintf(fd, format, arg_ptr);
        va_end(arg_ptr);
    }



};
#define LOG_DEBUG(fmt, ...) LOG::print(__FILE__, __FUNCTION__, __LINE__, MY_DEBUG_NAME, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG::print(__FILE__, __FUNCTION__, __LINE__, MY_ERROR_NAME, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG::print(__FILE__, __FUNCTION__, __LINE__, MY_INFO_NAME, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG::print(__FILE__, __FUNCTION__, __LINE__, MY_WARN_NAME, fmt, ##__VA_ARGS__)

#endif //C_TEST_LOG_H
