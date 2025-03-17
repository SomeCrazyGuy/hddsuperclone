#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#include "util.h"

static int global_log_level = UTIL_LOG_LEVEL_INFO;

static const char* const log_level_names[] = {
    "INFO",
    "WARN",
    "ERROR"
};

#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

extern void util_set_log_level(int log_level) {
    global_log_level = log_level;
}

// prints formatted message to stderr with color if stderr is a file
static void util_write_message(int log_level, const char* message);

extern void util_log_impl(int log_level, const char* const file, const char* const func, const int line, const char* const format, ...) {
    char out_buffer[2048];
    
    if (global_log_level > log_level) {
        return;
    }

    const int log_level_names_count = sizeof(log_level_names) / sizeof(log_level_names[0]);

    if (UTIL_LOG_LEVEL_MAX != log_level_names_count) {
        fprintf(stderr, "Invalid log level names count: %d != %d\n", UTIL_LOG_LEVEL_MAX, log_level_names_count);
        return;
    }

    if (log_level < 0 || log_level >= log_level_names_count) {
        fprintf(stderr, "Invalid log level specified: %d (%s:%s:%d)\n", log_level, file, func, line);
        return; 
    }
        
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);

    size_t bytes_written = strftime(out_buffer, sizeof(out_buffer), "%F %T ", tm);
    if (bytes_written == 0) {
        fprintf(stderr, "Failed to format timestamp\n");
        return; 
    }
    
    int prefix_len = snprintf(out_buffer + bytes_written, sizeof(out_buffer) - bytes_written, "[%s] %s:%d:%s(): ", log_level_names[log_level], file, line, func);
    if (prefix_len <= 0) {
        fprintf(stderr, "Failed to format log prefix\n");
        return; 
    }

    bytes_written += prefix_len;

    va_list args;
    va_start(args, format);
    int message_len = vsnprintf(out_buffer + bytes_written, sizeof(out_buffer) - bytes_written, format, args);
    va_end(args);

    if (message_len <= 0) {
        fprintf(stderr, "Failed to format log message\n");
        return; 
    }

    bytes_written += message_len;

    if (bytes_written >= (sizeof(out_buffer) - 2)) {
        fprintf(stderr, "Log message too long\n");
        return; 
    }

    util_write_message(log_level, out_buffer);
}

static void util_write_message(int log_level, const char* message) {
    static int initialized = 0;
    static int is_tty = 0;
    //static FILE* log_file = NULL; //TODO: log to file

    if (!initialized) {
        initialized = 1;
        is_tty = isatty(fileno(stderr));
    }

    if (is_tty) {
        switch(log_level) {
            case UTIL_LOG_LEVEL_INFO:
                fprintf(stderr, COLOR_GREEN "%s\n" COLOR_RESET, message);
                break;
            case UTIL_LOG_LEVEL_WARN:
                fprintf(stderr, COLOR_YELLOW "%s\n" COLOR_RESET, message);
                break;
            case UTIL_LOG_LEVEL_ERROR:
                fprintf(stderr, COLOR_RED "%s\n" COLOR_RESET, message);
                break;
            default:
                fprintf(stderr, "%s\n", message);
        }
    } else {
        fprintf(stderr, "%s\n", message);
    }
    fflush(stderr);
    //TODO: could log to a file too
}


extern void util_hexdump_to_buffer(char* out_buffer, unsigned out_size, const void* in_data, unsigned in_size) {
    char hex_buff[49];
    char ascii_buff[17];
    static const char hex[] = "0123456789ABCDEF";
    const unsigned char* data = (const unsigned char*)in_data;

    //position in hex buff, ascii buff, output buff
    unsigned hp = 0, ap = 0, op = 0;

    for (unsigned i = 0; i < in_size; ++i) {
        if (i && !(i & 15)) {
            int bytes = snprintf(&out_buffer[op], out_size - op, "%s|%s\n", hex_buff, ascii_buff);
            hp = 0;
            ap = 0;
            if(bytes >= 0) {
                op += bytes;
                if (op == out_size) return;
            }
        }

        unsigned char d = data[i];

        hex_buff[hp++] = hex[d >> 4];
        hex_buff[hp++] = hex[d & 15];
        hex_buff[hp++] = ' ';
        hex_buff[hp] = '\0';

        ascii_buff[ap++] = ((d < 0x7F) && (d > 0x1F))? (char)d : '.';
        ascii_buff[ap] = '\0';
    }

    //print remaining buffer if unaligned
    if (hp) {
        snprintf(&out_buffer[op], out_size - op, "%s|%s\n", hex_buff, ascii_buff);
    }
}


extern void util_hexdump_to_file(FILE* output_file, const void* in_data, unsigned in_size) {
    const unsigned char *in_ptr = (const unsigned char*)in_data;

    for(unsigned offset = 0; offset < in_size; offset += 16) {
        char line_buffer[128];
        unsigned len = in_size - offset;
        if (len > 16) len = 16;
        util_hexdump_to_buffer(line_buffer, sizeof(line_buffer), in_ptr, len);
        fputs(line_buffer, output_file);
    }
}