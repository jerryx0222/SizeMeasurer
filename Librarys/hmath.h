#ifndef HMATH_H
#define HMATH_H
#include <QPointF>
#include <QLineF>
#include <vector>


class HMath
{
public:
    HMath();

    double      m_dblMin;

    // 2元1次求解:ax+by=c
    bool FindXYResult(double a1,double b1,double c1,double a2,double b2,double c2,double &x,double &y);


    // 兩線段求交叉點
    bool FindCrossPoint(QLineF line1,QLineF line2,QPointF& point);
    bool FindCrossPoint(double a,double b,double c,double a2,double b2,double c2,QPointF& point);

    // 圓線交叉點
    bool FindCircleLineCross(QPointF center,double radius,double a,double b,double c,std::vector<QPointF>& points);

    // 弧線交叉點
    bool FindArcLineCross(QPointF center,double radius,double ang,double aLen,double a,double b,double c,std::vector<QPointF>& points);
    bool FindArcLineCross(QPointF center,double radius,double ang,double aLen,QLineF line,std::vector<QPointF>& points);

    // 求垂直線
    bool FindVerticalLine(double a,double b,double c,QPointF point,double &a2,double &b2,double &c2);
    bool FindVerticalLine(QLineF line,QPointF point,double &a2,double &b2,double &c2);
    bool FindVerticalLine(QLineF line,QPointF point,double len,QLineF &lineOut);

    // 求平行線
    bool FindParallelLine(QLineF line,QPointF point,QLineF& lineOut);
    bool FindParallelLine(QLineF line,double distance,QLineF& lineOut);

    // 線段轉ABC
    void TransLine(QLineF line,double &a,double &b,double &c);

    // 求點到線的距離及投影點
    bool GetPoint2LineDistance(QPointF point,QLineF line,double &dis,QPointF& ptCross);
    bool GetPoint2LineDistance(QPointF point,double a,double b,double c,double &dis,QPointF& ptCross);

    // 最小平方法求擬合線
    bool FitLine(std::vector<QPointF> &points,double &a,double &b,double &c);

    // 求平行線的中間線
    bool FindParallelCenterLine(QLineF line1,QLineF line2,QLineF& lineOut);


    // 求線的角度
    bool GetAngle(QLineF line,double& angle);
    double GetLineLength(QLineF line);

    // 線的旋轉
    bool RotateLine(QLineF line,double sita,QLineF &lineOut);
    bool RotateLineByCenter(QLineF line,double sita,QLineF &lineOut);
    bool RotateLineByPoint1(QLineF line,double sita,double len,QLineF &lineOut);

    // 點的旋轉
    bool RotatePoint(QPointF point,QPointF center,double sita,QPointF &pointOut);


    // 找線上的點
    bool FindPointInLine(QLineF line,double len,QPointF& ptOut);

    // 線線的角度
    bool FindAngleFromLine(QLineF line,double& sita);

    // 線段由中心點點外延或內縮
    bool ExtendLine(QLineF lineIn,double len,QLineF& lineOut);


    //線段Shift
    bool ShiftLine(QLineF lineIn,QPointF pDir,QLineF& lineOut);

    // 線段交換
    bool ExchangeLine(QLineF &line1,QLineF& line2);
};

#endif // HMATH_H
