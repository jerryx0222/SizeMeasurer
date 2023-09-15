#ifndef MAIN_H
#define MAIN_H
#include "Librarys/HMachineBase.h"

enum MACHINETYPE
{
    mtOthers,
    mtMSCAP,
    mtLigamaxAssembly,
};

extern HMachineBase* gMachine;
extern int      gMachineType;
extern QSizeF   gDataSize;


#endif // MAIN_H
