QT       += core gui sql serialport


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = SizeMeasurer
TEMPLATE = app

TRANSLATIONS += SizeMeasurer_en.ts \
                SizeMeasurer_zh-tw.ts

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += UNICODE

#DEFINES += ADVANTECH_SUSI
DEFINES += BSM_RS485IO

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


CONFIG += c++11

RC_ICONS = intai.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Librarys/HBase.cpp \
    Librarys/HDataBase.cpp \
    Librarys/HError.cpp \
    Librarys/HMachineBase.cpp \
    Librarys/HTimer.cpp \
    Librarys/JQChecksum.cpp \
    Librarys/hdisplaypattern.cpp \
    Librarys/hdisplayscene.cpp \
    Librarys/hgraphicsdxf.cpp \
    Librarys/hhalconcalibration.cpp \
    Librarys/hhalconlibrary.cpp \
    Librarys/himageclient.cpp \
    Librarys/hio.cpp \
    Librarys/hmath.cpp \
    Librarys/hpatternrect.cpp \
    Librarys/hpatternroiscene.cpp \
    Librarys/hroiarc.cpp \
    Librarys/hroicircle.cpp \
    Librarys/hroirect.cpp \
    Librarys/hsearchresult.cpp \
    Librarys/hserver.cpp \
    Librarys/hvalve.cpp \
    Librarys/hvisionalignmenthalcon.cpp \
    Librarys/myserialport.cpp \
    Librarys/qtdisplay.cpp \
    dlgcalibration.cpp \
    dlgmeasuredetail.cpp \
    dlgmeasurepolyline.cpp \
    dlgshowimage.cpp \
    dxflib/dl_dxf.cpp \
    dxflib/dl_writer_ascii.cpp \
    dxflib/hdxf.cpp \
    hcamera.cpp \
    hfeaturedata.cpp \
    hlight.cpp \
    hmachine.cpp \
    hmeasureitem.cpp \
    hpagebutton.cpp \
    hresultclasser.cpp \
    hsearchresultslot.cpp \
    hvisionclient.cpp \
    hvisionsystem.cpp \
    main.cpp \
    vautopage.cpp \
    vdlgerror.cpp \
    vdlglogin.cpp \
    vdlgmap.cpp \
    vdlgtypeselect.cpp \
    vdrawplot.cpp \
    verrorhistory.cpp \
    verrorstatistics.cpp \
    viopage.cpp \
    vmdiview.cpp \
    vmeasureclass.cpp \
    vmeasuredetail.cpp \
    vmotorpage.cpp \
    voptview.cpp \
    vpageview.cpp \
    vparameterpage.cpp \
    vproductpage.cpp \
    vsystemparameter.cpp \
    vtabview.cpp \
    vtabviewbase.cpp \
    vtimerpage.cpp \
    vtopview.cpp \
    vuserpage.cpp \
    vvalvepage.cpp \
    vvisionpage.cpp \
    hhalconlabelauto.cpp \
    hhalconlabel.cpp \
    hhalconlabelcali.cpp \
    hhalconlabelvision.cpp

HEADERS += \
    Librarys/HBase.h \
    Librarys/HDataBase.h \
    Librarys/HError.h \
    Librarys/HMachineBase.h \
    Librarys/HMemMap.h \
    Librarys/HTimer.h \
    Librarys/JQChecksum.h \
    Librarys/hdisplaypattern.h \
    Librarys/hdisplayscene.h \
    Librarys/hgraphicsdxf.h \
    Librarys/hhalconcalibration.h \
    Librarys/hhalconlibrary.h \
    Librarys/himageclient.h \
    Librarys/hio.h \
    Librarys/hmath.h \
    Librarys/hpatternrect.h \
    Librarys/hpatternroiscene.h \
    Librarys/hroiarc.h \
    Librarys/hroicircle.h \
    Librarys/hroirect.h \
    Librarys/hsearchresult.h \
    Librarys/hserver.h \
    Librarys/hvalve.h \
    Librarys/hvisionalignmenthalcon.h \
    Librarys/myserialport.h \
    Librarys/qtdisplay.h \
    Main.h \
    dlgcalibration.h \
    dlgmeasuredetail.h \
    dlgmeasurepolyline.h \
    dlgshowimage.h \
    dxflib/dl_attributes.h \
    dxflib/dl_codes.h \
    dxflib/dl_creationadapter.h \
    dxflib/dl_creationinterface.h \
    dxflib/dl_dxf.h \
    dxflib/dl_entities.h \
    dxflib/dl_exception.h \
    dxflib/dl_extrusion.h \
    dxflib/dl_writer.h \
    dxflib/dl_writer_ascii.h \
    dxflib/hdxf.h \
    hcamera.h \
    hfeaturedata.h \
    hlight.h \
    hmachine.h \
    hmeasureitem.h \
    hpagebutton.h \
    hresultclasser.h \
    hsearchresultslot.h \
    hvisionclient.h \
    hvisionsystem.h \
    vautopage.h \
    vdlgerror.h \
    vdlglogin.h \
    vdlgmap.h \
    vdlgtypeselect.h \
    vdrawplot.h \
    verrorhistory.h \
    verrorstatistics.h \
    viopage.h \
    vmdiview.h \
    vmeasureclass.h \
    vmeasuredetail.h \
    vmotorpage.h \
    voptview.h \
    vpageview.h \
    vparameterpage.h \
    vproductpage.h \
    vsystemparameter.h \
    vtabview.h \
    vtabviewbase.h \
    vtimerpage.h \
    vtopview.h \
    vuserpage.h \
    vvalvepage.h \
    vvisionpage.h \
    hhalconlabelauto.h \
    hhalconlabel.h \
    hhalconlabelcali.h \
    hhalconlabelvision.h

FORMS += \
    dlgcalibration.ui \
    dlgmeasuredetail.ui \
    dlgmeasurepolyline.ui \
    dlgptnset.ui \
    dlgshowimage.ui \
    vautopage.ui \
    vdlgerror.ui \
    vdlglogin.ui \
    vdlgmap.ui \
    vdlgtypeselect.ui \
    verrorhistory.ui \
    verrorstatistics.ui \
    viopage.ui \
    vmdiview.ui \
    vmeasureclass.ui \
    vmeasuredetail.ui \
    vmotorpage.ui \
    voptview.ui \
    vpageview.ui \
    vparameterpage.ui \
    vproductpage.ui \
    vsystemparameter.ui \
    vtabview.ui \
    vtimerpage.ui \
    vtopview.ui \
    vuserpage.ui \
    vvalvepage.ui \
    vvisionpage.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Images.qrc

#inclued
INCLUDEPATH         += "$$(HALCONROOT)/include"
INCLUDEPATH         += "$$(HALCONROOT)/include/halconcpp"
unix:INCLUDEPATH    += "/opt/halcon/include"
unix:INCLUDEPATH    += "/opt/halcon/include/halconcpp"
Advantech_Susi:INCLUDEPATH  += "C:/Program Files/Advantech/SUSI/SDK/SUSI4/include"

#libs
QMAKE_LIBDIR        += "$$(HALCONROOT)/lib/$$(HALCONARCH)"
unix:QMAKE_LIBDIR   += "/opt/halcon/lib/armv7a-linux"
unix:LIBS           += -lhalconcpp -lhalcon -lXext -lX11 -ldl -lpthread
win32:LIBS          += "$$(HALCONROOT)/lib/$$(HALCONARCH)/halconcpp.lib" \
                        "$$(HALCONROOT)/lib/$$(HALCONARCH)/halcon.lib"

Advantech_Susi:LIBS += "C:/Program Files/Advantech/SUSI/SDK/SUSI4/lib/x64/Susi4.lib"


# Disable warning C4819 for msvc
msvc:QMAKE_CXXFLAGS += -execution-charset:utf-8
msvc:QMAKE_CXXFLAGS += -source-charset:utf-8
#QMAKE_CXXFLAGS_WARN_ON += -wd4819


#INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include"
#LIBS+= "C:/Program Files (x86)/Visual Leak Detector/lib/Win64/vld.lib"

#INCLUDEPATH += "C:/Program Files/opencv460/build/include"
#DEPENDPATH  += "C:/Program Files/opencv460/build/include"
#CONFIG(debug,debug|release)
#{
#    LIBS+= "C:/Program Files/opencv460/build/x64/vc15/lib/opencv_world460d.lib"
#}
#CONFIG(release,debug|release)
#{
#    LIBS+= "C:/Program Files/opencv460/build/x64/vc15/lib/opencv_world460.lib"
#}
#LIBS+= -L"C:/Program Files/opencv460/build/x64/vc15/bin"
