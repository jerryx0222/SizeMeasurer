#include "hsearchresultslot.h"
#include <QImage>

HSearchResultSlot::HSearchResultSlot()
{
    m_pImage=nullptr;
}

HSearchResultSlot::~HSearchResultSlot()
{
    if(m_pImage!=nullptr)
        delete m_pImage;
}

void HSearchResultSlot::DrawResults(std::map<int, HSearchResult *> *pResult)
{
    std::map<int, HSearchResult *>::iterator itMap;
    for(itMap=pResult->begin();itMap!=pResult->end();itMap++)
        OnDrawResult(itMap->second,false);
}

void HSearchResultSlot::OnGetPatternMake(stcPattern *pPattern)
{
    OnDrawPattern(pPattern);
}

void HSearchResultSlot::OnImageShow(QImage *pImage)
{
    OnDrawImage(pImage);
}

void HSearchResultSlot::OnGetSearchResult(QImage *pImage, HSearchResult *pResult)
{
    if(pImage!=nullptr)
        OnDrawImage(pImage);
     OnDrawResult(pResult,true);
}

void HSearchResultSlot::OnGetSearchResult(std::map<int, HSearchResult *> *pResult)
{
    DrawResults(pResult);
}



/***************************************************************/

VSearchResultSlot::VSearchResultSlot(QObject *)
{
    m_pDisplay=nullptr;
    m_pPtnDsp=nullptr;
}

void VSearchResultSlot::OnDrawImage(QImage *pImage)
{
    if(m_pImage!=nullptr)
        delete m_pImage;
    if(m_pDisplay==nullptr)
        return;

    m_pImage=pImage;
    m_pDisplay->DrawImage(*m_pImage);
}

void VSearchResultSlot::OnDrawResult(HSearchResult *pResult,bool drawText)
{
    if(m_pImage==nullptr)
        return;

    if(pResult==nullptr)
    {
        m_pDisplay->DrawText1(QPoint(100,m_pImage->height()-150),QColor(255,0,0),"Search Failed!");
        return;
    }

    QString strMessage;
    QPointF         center,ptTemp;
    HLineResult*    pLResult;
    HArcResult*     pAResult;
    HCircleResult*  pCResult;
    HPatternResult* pPResult;

    HSearchResult::TYPE type=static_cast<HSearchResult::TYPE>(pResult->GetType());
    switch(type)
    {
    case  HSearchResult::tCircle:
        pCResult=static_cast<HCircleResult*>(pResult);
        ptTemp.setX(pCResult->center.x());
        ptTemp.setY(pCResult->center.y());
        m_pDisplay->DrawCircle(ptTemp,pCResult->radius,pCResult->range);
        if(drawText)
        {
            strMessage=QString("X:%1,Y:%2,R:%3").arg(pCResult->center.x()).arg(pCResult->center.y()).arg(pCResult->radius);
            m_pDisplay->DrawText1(QPoint(100,m_pImage->height()-150),QColor(0,255,0),strMessage);
        }
        break;
    case HSearchResult::tArc:
        pAResult=static_cast<HArcResult*>(pResult);
        ptTemp.setX(pAResult->center.x());
        ptTemp.setY(pAResult->center.y());
        m_pDisplay->DrawArc(ptTemp,pAResult->radius,pAResult->angleStart,pAResult->angleEnd,pAResult->range);
        if(drawText)
        {
            strMessage=QString("X:%1,Y:%2,R:%3").arg(pAResult->center.x()).arg(pAResult->center.y()).arg(pAResult->radius);
            m_pDisplay->DrawText1(QPoint(100,m_pImage->height()-150),QColor(0,255,0),strMessage);
        }
        break;
    case HSearchResult::tRetange:
        break;
    case HSearchResult::tLine:
        pLResult=static_cast<HLineResult*>(pResult);
        m_pDisplay->DrawLine(pLResult->m_Line.p1(),pLResult->m_Line.p2());
        if(drawText)
        {
            strMessage=QString("X1:%1,Y1:%2,X2:%3,Y2:%4").arg(
                        pLResult->m_Line.p1().x()).arg(
                        pLResult->m_Line.p1().y()).arg(
                        pLResult->m_Line.p2().x()).arg(
                        pLResult->m_Line.p2().y());
            m_pDisplay->DrawText1(QPoint(100,m_pImage->height()-150),QColor(0,255,0),strMessage);
        }
        break;
    case HSearchResult::tPattern:
        pPResult=static_cast<HPatternResult*>(pResult);
        center=pPResult->GetCenterPixel();
        m_pDisplay->DrawRect(pPResult->rectangle,pPResult->angle);
        if(drawText)
        {
            strMessage=QString("X:%1,Y:%2,A:%3,S:%4").arg(
                        center.x()).arg(
                        center.y()).arg(
                        pPResult->angle*180/3.1415926).arg(
                        pPResult->score);
            m_pDisplay->DrawText1(QPoint(100,m_pImage->height()-150),QColor(0,255,0),strMessage);
        }
        break;
    }
}



void VSearchResultSlot::OnDrawPattern(stcPattern *pPattern)
{
    if(m_pPtnDsp!=nullptr)
    {
        if(pPattern!=nullptr && !pPattern->image.isNull())
            m_pPtnDsp->DrawPattern(pPattern->image);
        else
        {
            QImage image=QImage(100,100,QImage::Format_Mono);
            image.fill(255);
            m_pPtnDsp->DrawPattern(image);
        }
    }

}
