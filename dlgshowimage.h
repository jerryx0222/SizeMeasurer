#ifndef DLGSHOWIMAGE_H
#define DLGSHOWIMAGE_H

#include <QDialog>
#include <QImage>

namespace Ui {
class dlgShowImage;
}

class dlgShowImage : public QDialog
{
    Q_OBJECT

public:
    explicit dlgShowImage(QImage *imgSource,QImage *imgPlot,QWidget *parent = nullptr);
    ~dlgShowImage();

private slots:
    void on_btnSrc_clicked();
    void on_btnResult_clicked();

    void on_btnExport_clicked();

private:
    void DisplayImage(QImage* pImage);

private:
    Ui::dlgShowImage *ui;

    QImage* m_pImgSource,*m_pImgPlot;
    QImage* m_pImageDraw;

};

#endif // DLGSHOWIMAGE_H
