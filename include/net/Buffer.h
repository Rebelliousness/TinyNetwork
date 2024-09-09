#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>
#include <algorithm>

struct Buffer {
public:
    static const size_t kCheapPrepend = 8;

    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t InitialSize = kInitialSize)
        : buffer_(kCheapPrepend + InitialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
    {}

    size_t readableBytes() const {
        return writerIndex_ - readerIndex_;
    }

    size_t writeableBytes() const {
        return buffer_.size() - writerIndex_;
    }

    size_t prependableBytes() const {
        return readerIndex_;
    }

    const char *peek() const {
        return begin() + readerIndex_;
    }
    
    void retrieveUntil(const char *end) {
        retrieve(end - peek());
    }


    void retrieve(size_t len) {
        if(len < readableBytes()) {
            readerIndex_ += len;
        }
        else {
            retrieveAll();
        }
    }

    void retrieveAll() {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }
private:
    char *begin() {
        return &(*buffer_.begin());
    }

    const char *begin() const {
        return &(*buffer_.begin());
    }

    void makeSpace(int len) {
        
    }


    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
    static const char kCRLF[];
};

#endif