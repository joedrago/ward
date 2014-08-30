#include "Ward.h"

#include <queue>

class WardScanThread : public WardThread
{
public:
    WardScanThread(Ward *ward)
    : ward_(ward)
    {
    }

    ~WardScanThread()
    {
    }

    virtual int run()
    {
        std::vector<std::string> searchPaths;
        ward_->getSearchPaths(searchPaths);
        if(searchPaths.empty())
        {
            WARD_PRINTF("no search paths specified, giving up\n");
            return -1;
        }

        std::queue<std::string> searchQueue;

        for(std::vector<std::string>::iterator searchPathIter = searchPaths.begin(); searchPathIter != searchPaths.end(); ++searchPathIter)
        {
            searchQueue.push(*searchPathIter);
        }

        int startTime = WardThread::now();
        int fileCount = 0;
        while(!searchQueue.empty())
        {
            std::string searchPath = searchQueue.front();
            searchQueue.pop();

            WardPathWalker walker(searchPath);
            if(!walker.begin())
            {
                WARD_PRINTF("Failed to begin search on %s\n", searchPath.c_str());
                continue;
            }

            while(walker.next())
            {
                if(walker.isDirectory())
                {
                    WARD_PRINTF("Queueing: %s\n", walker.currentPath().c_str());
                    searchQueue.push(walker.currentPath());
                }
                else
                {
                    WARD_PRINTF("* Found file: %s\n", walker.currentPath().c_str());
                    fileCount++;
                }
            }
            walker.end();
        }

        int endTime = WardThread::now();
        WARD_PRINTF("Found %d files. (%ums)\n", fileCount, endTime - startTime);

        return 0;
    }

protected:
    Ward *ward_;
};

Ward::Ward()
: watching_(false)
{
    mutex_ = new WardMutex;
    scanThread_ = NULL;
}

Ward::~Ward()
{
    stop();

    delete mutex_;
    mutex_ = NULL;
}

void Ward::addSearchPath(const std::string &path)
{
    bool wasWatching = watching_;
    stop();

    {
        WardScopedMutex lock(mutex_);
        searchPaths_.push_back(path);
    }

    if(wasWatching)
        start();
}

void Ward::clearSearchPaths()
{
    stop();

    WardScopedMutex lock(mutex_);
    searchPaths_.clear();
}

void Ward::getSearchPaths(std::vector<std::string> &paths)
{
    WardScopedMutex lock(mutex_);

    paths.clear();
    for(std::vector<std::string>::iterator it = searchPaths_.begin(); it != searchPaths_.end(); ++it)
    {
        paths.push_back(*it);
    }
}

void Ward::start()
{
    WardScopedMutex lock(mutex_);

    if(watching_)
        return;

    scanThread_ = new WardScanThread(this);
    scanThread_->start();
    watching_ = true;
}

void Ward::stop()
{
    WardScopedMutex lock(mutex_);

    if(!watching_)
        return;

    if(scanThread_)
    {
        delete scanThread_;
        scanThread_ = NULL;
    }

    watching_ = false;
}

void Ward::restart()
{
    stop();
    start();
}

int Ward::count()
{
    return 0;
}
