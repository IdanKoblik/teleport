#ifndef LOG_H_
#define LOG_H_

#define LOG(fmt, ...) \
    do { \
        time_t _now = time(NULL); \
        struct tm _tm; \
        localtime_r(&_now, &_tm); \
        char _buf[64]; \
        strftime(_buf, sizeof(_buf), "%Y-%m-%d %H:%M:%S", &_tm); \
        fprintf(stderr, "[%s %s:%d] " fmt "\n", _buf, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define ERROR(fmt, ...) \
    LOG("[ERROR] " fmt, ##__VA_ARGS__)

#define WARN(fmt, ...) \
    LOG("[WARN] " fmt, ##__VA_ARGS__)

#endif // LOG_H_
