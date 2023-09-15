#ifndef HHALCONLIBRARY_H
#define HHALCONLIBRARY_H

#include <QObject>

#ifndef HC_LARGE_IMAGES
    #include <halconcpp/HalconCpp.h>
#else
    #include <HALCONCppx1/HalconCpp.h>
#endif

class HHalconLibrary : public QObject
{
    Q_OBJECT
public:
    explicit HHalconLibrary(QObject *parent = nullptr);
    virtual ~HHalconLibrary(void);

    static QImage* Hobject2QImage(const HalconCpp::HObject& hoImage);
    static HalconCpp::HImage* QImage2HImage(QImage *pFrom);

    static QByteArray* TransHImage2ByteArray(HalconCpp::HImage* pImage);
    static HalconCpp::HImage* TransByteArray2HImage(QByteArray* pBArray);


private:


signals:

public slots:
};

#endif // HHALCONLIBRARY_H
