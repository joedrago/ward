#include "Ward.h"

#include <windows.h>

// --------------------------------------------------------------------------------------
// WardMutex

struct WardMutexInternal
{
    HANDLE handle_;
};

WardMutex::WardMutex()
{
    internal_ = new WardMutexInternal;
    internal_->handle_ = CreateMutex(NULL, FALSE, NULL);
}

WardMutex::~WardMutex()
{
    CloseHandle(internal_->handle_);
    delete internal_;
    internal_ = NULL;
}

void WardMutex::lock()
{
    DWORD ret = WaitForSingleObject(internal_->handle_, INFINITE);
    WARD_ASSERT(ret == WAIT_OBJECT_0);
}

void WardMutex::unlock()
{
    ReleaseMutex(internal_->handle_);
}

// --------------------------------------------------------------------------------------
// WardThread

struct WardThreadInternal
{
    HANDLE handle_;
};

WardThread::WardThread()
: stopRequested_(0)
{
    internal_ = new WardThreadInternal;
    internal_->handle_ = INVALID_HANDLE_VALUE;
}

WardThread::~WardThread()
{
    stop();
    wait();

    delete internal_;
    internal_ = NULL;
}

static DWORD wardThreadFunc(void *rawWardThread)
{
    WardThread *wardThread = (WardThread *)rawWardThread;
    int ret = wardThread->run();
    wardThread->setComplete();
    WARD_PRINTF("thread complete.\n");
    return ret;
}

void WardThread::start()
{
    DWORD id;
    internal_->handle_ = CreateThread(NULL, 0, wardThreadFunc, (void *)this, 0, &id);
}

void WardThread::stop()
{
    stopRequested_ = 1;
}

void WardThread::wait()
{
    if(internal_->handle_ != INVALID_HANDLE_VALUE)
    {
        WaitForSingleObject(internal_->handle_, INFINITE);
        CloseHandle(internal_->handle_);
        internal_->handle_ = INVALID_HANDLE_VALUE;
    }
}

unsigned int WardThread::now()
{
    return GetTickCount();
}

void WardThread::sleep(int ms)
{
    Sleep(ms);
}

// --------------------------------------------------------------------------------------
// WardPathWalker

struct WardPathWalkerInternal
{
    HANDLE handle_;
    WIN32_FIND_DATA wfd_;
};

WardPathWalker::WardPathWalker(const std::string &walkPath)
: walkPath_(walkPath)
, isDirectory_(false)
{
    internal_ = new WardPathWalkerInternal;
    internal_->handle_ = INVALID_HANDLE_VALUE;
}

WardPathWalker::~WardPathWalker()
{
    end();

    delete internal_;
    internal_ = NULL;
}

std::string & WardPathWalker::cleanup(std::string &path)
{
    if(path.empty())
        return path;

    char *front = &path[0];
    char *src = front;
    char *dst = src;

    char prev = 0;
    for(; *src; ++src)
    {
        if(*src == '/')
        {
            *src = '\\';
        }

        if((*src != '\\') || (prev != '\\'))
        {
            *dst = *src;
            ++dst;
        }
        prev = *src;
    }
    *dst = 0;
    path.resize(dst - front);

    return path;
}

bool WardPathWalker::begin()
{
    end();
    std::string wildcard = walkPath_ + "\\\\\\*.*";
    cleanup(wildcard);

    internal_->handle_ = FindFirstFile(wildcard.c_str(), &internal_->wfd_);
    return (internal_->handle_ != INVALID_HANDLE_VALUE);
}

bool WardPathWalker::next()
{
    if(internal_->handle_ == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    bool foundAnotherPath = false;
    for(;;)
    {
        foundAnotherPath = FindNextFile(internal_->handle_, &internal_->wfd_) != 0;
        if(foundAnotherPath)
        {
            if((!strcmp(internal_->wfd_.cFileName, ".")) || (!strcmp(internal_->wfd_.cFileName, "..")))
            {
                continue;
            }

            isDirectory_ = (internal_->wfd_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            currentPath_ = walkPath_ + "\\" + internal_->wfd_.cFileName;
            cleanup(currentPath_);
        }
        break;
    }

    return foundAnotherPath;
}

void WardPathWalker::end()
{
    if(internal_->handle_ != INVALID_HANDLE_VALUE)
    {
        FindClose(internal_->handle_);
        internal_->handle_ = INVALID_HANDLE_VALUE;
    }
    currentPath_.clear();
}


// protected:
// std::string walkPath_;
// std::string currentPath_;
// struct WardPathWalkerInternal *internal_;
// };