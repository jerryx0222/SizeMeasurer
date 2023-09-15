#ifndef HSEARCHRESULTSLOT_H
#define HSEARCHRESULTSLOT_H

#include <QObject>
#include <QImage>
#include "Librarys/hsearchresult.h"
#include "Librarys/qtdisplay.h"
#include "Librarys/hdisplaypattern.h"


class HSearchResultSlot:public QObject
{
    Q_OBJECT
public:
    HSearchResultSlot();

    virtual ~HSearchResultSlot();

    virtual void OnDrawImage(QImage* pImage)=0;
    virtual void OnDrawResult(HSearchResult*,bool drawText)=0;
    virtual void OnDrawPattern(stcPattern*)=0;

    void DrawResults(std::map<int,HSearchResult*>* pResult);

public slots:
    void OnGetSearchResult(QImage*,HSearchResult* pResult);
    void OnGetSearchResult(std::map<int,HSearchResult*>* pResult);
    void OnGetPatternMake(stcPattern*);
    void OnImageShow(QImage*);

public:
    QImage          *m_pImage;
    //HSearchResult   *m_pResult;
};

/********************************************************************************/
class VSearchResultSlot : public HSearchResultSlot
{
    Q_OBJECT
public:
    explicit VSearchResultSlot(QObject *parent = nullptr);

    QtDisplay       *m_pDisplay;
    HDisplayPattern *m_pPtnDsp;

    virtual void OnDrawImage(QImage* pImage);
    virtual void OnDrawResult(HSearchResult*,bool drawText);
    virtual void OnDrawPattern(stcPattern*);

signals:

public slots:
};

#endif // HSEARCHRESULTSLOT_H
