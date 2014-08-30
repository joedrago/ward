#ifndef WARD_H
#define WARD_H

#include <string>
#include <vector>

#ifdef _DEBUG
#include <assert.h>
#define WARD_ASSERT assert
#define WARD_PRINTF printf
#else
#define WARD_ASSERT(V)
#define WARD_PRINTF(...)
#endif

class WardMutex
{
public:
    WardMutex();
    ~WardMutex();

    void lock();
    void unlock();

private:
    struct WardMutexInternal *internal_;
};

class WardScopedMutex
{
public:
    WardScopedMutex(WardMutex *mutex)
    : mutex_(mutex)
    {
        mutex_->lock();
    }

    ~WardScopedMutex()
    {
        mutex_->unlock();
    }

protected:
    WardMutex *mutex_;
};

class WardThread
{
public:
    WardThread();
    virtual ~WardThread();

    void start();
    void stop();
    void wait();

    bool isComplete() { return complete_; }
    void setComplete() { complete_ = true; }

    virtual int run() = 0;

    static void sleep(int ms);
    static unsigned int now(); // returns MS timer

private:
    bool started_;
    bool complete_;
    int stopRequested_;
    struct WardThreadInternal *internal_;
};

class WardPathWalker
{
public:
    WardPathWalker(const std::string &walkPath);
    ~WardPathWalker();

    bool begin();
    bool next();
    void end();

    const std::string &currentPath() const { return currentPath_; }
    bool isDirectory() const { return isDirectory_; }

    static std::string &cleanup(std::string &path); // modifies incoming string in-place

protected:
    std::string walkPath_;
    std::string currentPath_;
    bool isDirectory_;
    struct WardPathWalkerInternal *internal_;
};

class Ward
{
public:
    Ward();
    ~Ward();

    void clearSearchPaths();
    void addSearchPath(const std::string &path);
    void getSearchPaths(std::vector<std::string> &paths);

    void start();
    void stop();
    void restart();

    int count();

protected:
    bool watching_;
    WardMutex *mutex_;
    WardThread *scanThread_;
    std::vector<std::string> searchPaths_;
};

#endif