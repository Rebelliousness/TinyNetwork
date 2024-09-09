#include <LogFile.h>

LogFile::LogFile(const std::string &basename, 
                off_t rollSize, 
                int flushInterval, 
                int checkEveryN)
    : basename_(basename)
    , rollSize_(rollSize)
    , flushInterval_(flushInterval)
    , checkEveryN_(checkEveryN)
    , count_(0)
    , mutex_(new std::mutex)
    , startOfPeriod_(0)
    , lastRoll_(0)
    , lastFlush_(0){
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char *data, int len) {
    std::lock_guard<std::mutex> lock(*mutex_);
    appendInLock(data, len);
}

void LogFile::appendInLock(const char *data, int len) {
    file_ -> append(data, len);
    
    if(file_ -> writtenBytes() > rollSize_) {
        rollFile();
    }
    else {
        ++count_;
        if(count_ >= checkEveryN_) {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod = now / kRollPerSeconds_ * kRollPerSeconds_;
            if(thisPeriod != startOfPeriod_) {
                rollFile();
            }
            else if(now - lastFlush_ > flushInterval_) {
                lastFlush_ = now;
                file_ -> flush();
            }
        }
    }
}

void LogFile::flush() {
    std::lock_guard<std::mutex> lock(*mutex_);
    file_ -> flush();
}

//日志滚动
//basename + time + hostname + pid + ".log"
bool LogFile::rollFile() {
    time_t now = 0;
    std::string filename = getLogFileName(basename_, &now);

    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if(now > lastRoll_) {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new FileUtil(filename));
        return true;
    }
    return false;
}

std::string LogFile::getLogFileName(const std::string &basename, time_t *now) {
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    localtime_r(now, &tm);
    
    strftime(timebuf, sizeof(timebuf), ".%Y%m%D-%H%M%S", &tm);
    filename += timebuf;
    filename += ".log";
    return filename;
}