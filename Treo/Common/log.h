/**
 * 裸机日志系统，V1.0.0
 * 2026年8月18日-Treo
 */
#ifndef AURON_LOG_H_
#define AURON_LOG_H_



#define COLOR_RED            "\033[31m"
#define COLOR_GREEN          "\033[32m"
#define COLOR_YELLOW         "\033[33m"
#define COLOR_BLUE           "\033[34m"
#define COLOR_MAGENTA        "\033[35m"
#define COLOR_CYAN           "\033[36m"
#define COLOR_WHITE          "\033[37m"
#define COLOR_BRIGHT_RED     "\033[91m"
#define COLOR_BRIGHT_GREEN   "\033[92m"
#define COLOR_BRIGHT_YELLOW  "\033[93m"
#define COLOR_BRIGHT_BLUE    "\033[94m"
#define COLOR_BRIGHT_MAGENTA "\033[95m"
#define COLOR_BRIGHT_CYAN    "\033[96m"
#define COLOR_BRIGHT_WHITE   "\033[97m"
#define COLOR_RESET          "\033[0m"

#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_RESET   "\033[49m"

#define STYLE_BOLD      "\033[1m"
#define STYLE_UNDERLINE "\033[4m"
#define STYLE_RESET     "\033[22m"

#ifndef LOG_TAG
#define LOG_TAG "NONE"
#endif



#define LOG_PRINT(fmt, ...)                 printf("[" LOG_TAG "] " fmt "\r\n", ##__VA_ARGS__)
#define LOG_PRINT_COLOR(_color, fmt, ...)   printf(_color "[" LOG_TAG "] " fmt COLOR_RESET "\r\n", ##__VA_ARGS__)
#define LOG(fmt, ...)                       LOG_PRINT(fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)                  LOG_PRINT_COLOR(COLOR_GREEN, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)                  LOG_PRINT_COLOR(COLOR_YELLOW, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)                 LOG_PRINT_COLOR(COLOR_RED, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)                 LOG_PRINT_COLOR(COLOR_WHITE, fmt, ##__VA_ARGS__)
#define LOG_TRACE(fmt, ...)                 LOG_PRINT_COLOR(COLOR_CYAN, fmt, ##__VA_ARGS__)













#endif /* AURON_LOG_H_ */
