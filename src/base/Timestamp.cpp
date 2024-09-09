#include <Timestamp.h>

//获取当前时间戳
Timestamp Timestamp::now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * MicroSecondsPerSecond + tv.tv_usec);
}

std::string Timestamp::toFormattedString(bool showMicroseconds) const {
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / MicroSecondsPerSecond);
    tm *tm_time = localtime(&seconds);
    if(showMicroseconds) {
        int microseconds = static_cast<int>(microSecondsSinceEpoch_ / MicroSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d.%06d", 
                                    tm_time -> tm_year + 1900, 
                                    tm_time -> tm_mon + 1, 
                                    tm_time -> tm_mday, 
                                    tm_time -> tm_hour, 
                                    tm_time -> tm_min, 
                                    tm_time -> tm_sec, 
                                    microseconds);
    }
    else {
        snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d", 
                                    tm_time -> tm_year + 1900, 
                                    tm_time -> tm_mon + 1, 
                                    tm_time -> tm_mday, 
                                    tm_time -> tm_hour, 
                                    tm_time -> tm_min, 
                                    tm_time -> tm_sec);
    }
}