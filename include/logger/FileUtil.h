#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <stdio.h>
#include <string>

struct FileUtil {
public:
    explicit FileUtil(std::string &fileName);
    ~FileUtil();

    void append(const char *data, size_t len);
    void flush();
    off_t writtenBytes() const {
        return writtenBytes_;
    }
private:
    size_t write(const char *data, size_t len);

    FILE *fp_;
    char buffer_[64 * 1024];
    off_t writtenBytes_;
};

#endif