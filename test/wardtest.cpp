#include "ward.h"

int main(int argc, char *argv[])
{
    Ward *ward = new Ward;
    ward->addSearchPath("/Users/jdrago/work/ward");
    ward->start();
    while(1)
    {
        WardThread::sleep(5000);
        printf("Ward count now: %d\n", ward->count());
    }
    delete ward;
    return 0;
}