#ifndef TIME_STAMP_H
#define TIME_STAMP_H

#include <iostream>
#include <string>
#include <sys/time.h>

struct Timestamp {
public:
    Timestamp() : microSecondsSinceEpoch_(0) {}
    explicit Timestamp(int64_t microSecondsSinceEpoch) : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}
    //获取当前时间戳
    static Timestamp now();
    
    std::string toString() const;
    std::string toFormattedString(bool showMicroseconds) const;
    //返回当前时间戳表示的微妙数
    int64_t microSecondsSinceEpoch() const {
        return microSecondsSinceEpoch_;
    }
    //返回当前时间戳表示的秒数
    time_t secondsSinceEpoch() const {
        return static_cast<time_t>(microSecondsSinceEpoch_ / MicroSecondsPerSecond);
    }
    //返回一个无效的时间戳（该时间戳值为0）
    static Timestamp invalid() {
        return Timestamp();
    }

    static const int MicroSecondsPerSecond = 1000000;

private:
    int64_t microSecondsSinceEpoch_;    // 表示时间戳的微妙数（自上一个epoch开始经历的时间）
};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}


inline Timestamp addTime(Timestamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::MicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

#endif