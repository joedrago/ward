#include "Ward.h"

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

// --------------------------------------------------------------------------------------
// WardMutex

struct WardMutexInternal
{
    pthread_mutex_t mutex_;
};

WardMutex::WardMutex()
{
    internal_ = new WardMutexInternal;
    pthread_mutex_init(&internal_->mutex_, NULL);
}

WardMutex::~WardMutex()
{
    pthread_mutex_destroy(&internal_->mutex_);
    delete internal_;
    internal_ = NULL;
}

void WardMutex::lock()
{
    pthread_mutex_lock(&internal_->mutex_);
}

void WardMutex::unlock()
{
    pthread_mutex_unlock(&internal_->mutex_);
}

// --------------------------------------------------------------------------------------
// WardThread

struct WardThreadInternal
{
    pthread_t thread_;
};

WardThread::WardThread()
: stopRequested_(0)
{
    internal_ = new WardThreadInternal;
    internal_->thread_ = NULL;
}

WardThread::~WardThread()
{
    stop();
    wait();

    delete internal_;
    internal_ = NULL;
}

static void *wardThreadFunc(void *rawWardThread)
{
    WardThread *wardThread = (WardThread *)rawWardThread;
    wardThread->run();
    wardThread->setComplete();
    WARD_PRINTF("thread complete.\n");
    pthread_exit(NULL);
    return 0;
}

void WardThread::start()
{
    pthread_create(&internal_->thread_, NULL, wardThreadFunc, (void *)this);
}

void WardThread::stop()
{
    stopRequested_ = 1;
}

void WardThread::wait()
{
    if(internal_->thread_ != NULL)
    {
        pthread_join(internal_->thread_, NULL);
        internal_->thread_ = NULL;
    }
}

unsigned int WardThread::now()
{
    // return GetTickCount();
    return 0;
}

void WardThread::sleep(int ms)
{
    usleep(ms * 1000);
}

// --------------------------------------------------------------------------------------
// WardPathWalker

struct WardPathWalkerInternal
{
    DIR *dir_;
};

WardPathWalker::WardPathWalker(const std::string &walkPath)
: walkPath_(walkPath)
, isDirectory_(false)
{
    internal_ = new WardPathWalkerInternal;
    internal_->dir_ = NULL;
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
        if(*src == '\\')
        {
            *src = '/';
        }

        if((*src != '/') || (prev != '/'))
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

    internal_->dir_ = opendir(walkPath_.c_str());
    return (internal_->dir_ != NULL);
}

bool WardPathWalker::next()
{
    if(internal_->dir_ == NULL)
    {
        return false;
    }

    bool foundAnotherPath = false;
    for(;;)
    {
        struct dirent *dp = readdir(internal_->dir_);
        foundAnotherPath = (dp != NULL);
        if(foundAnotherPath)
        {
            const char *name = dp->d_name;
            if((!strcmp(name, ".")) || (!strcmp(name, "..")))
            {
                continue;
            }

            currentPath_ = walkPath_ + "/" + name;
            cleanup(currentPath_);

            struct stat st;
            lstat(currentPath_.c_str(), &st);
            isDirectory_ = (S_ISDIR(st.st_mode) != 0);
        }
        break;
    }

    return foundAnotherPath;
}

void WardPathWalker::end()
{
    if(internal_->dir_ != NULL)
    {
        closedir(internal_->dir_);
        internal_->dir_ = NULL;
    }
    currentPath_.clear();
}
