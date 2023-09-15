#ifndef HRESULTCLASSER_H
#define HRESULTCLASSER_H

#include <vector>
#include <map>
#include <QPoint>
#include "Librarys/HDataBase.h"

class HResultClasser
{
public:
    HResultClasser();
    virtual ~HResultClasser();

    void CreateDBTable(HDataBase *pWD);
    bool LoadWorkData(HDataBase *pWD);
    bool SaveWorkData(HDataBase *pWD);

    int GetClass(double xValue,double yValue);

    void operator=(HResultClasser& other);

public:
    int     m_bEnabled;
    int     m_nXFeature;
    int     m_nYFeature;
    int     m_nXCount;
    int     m_nYCount;
    int     m_nClassCount;

    std::vector<double>      m_vXPitch,m_vYPitch;
    std::map<uint32_t,int>   m_mapClassType;

private:
    HDataBase   *m_pWDB;


};

#endif // HRESULTCLASSER_H
