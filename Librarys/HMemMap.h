#ifndef HMEMMAP_H
#define HMEMMAP_H
/*
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
*/
#define  IMGMAXCOUNT    100
#define  SMEMORYSIZE    216000000        // 6000*6000*2*3 bytes
#define  SMBASEOFFSET   32               // 32 bytes



enum ERIMAGECODE
{
    erNone,
    erNoShareMemory,
    erImageUnlock,
    erNotInStep,
    erWorkNull,

};

enum MEMSTATUS
{
    msNone,
    msReady,    // src > target
    msGrabOK,   // src > target
    msGrabNG,   // src > target
    //msImageOK,  // src < target

    msGrabCmd,  // src < target
};

#endif // HMEMMAP_H
