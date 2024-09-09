#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

/*
禁止拷贝操作的基类
*/
struct noncopyable{
public:
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};


#endif