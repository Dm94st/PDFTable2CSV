TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -L/usr/local/lib/
QT_CONFIG -= no-pkg-config

CONFIG  += link_pkgconfig

PKGCONFIG += opencv

SOURCES += main.cpp \
    segmentation.cpp \
    imagefromfile.cpp \
    ocr.cpp \
    converter.cpp

HEADERS += \
    settings.h \
    segmentation.h \
    imagefromfile.h \
    ocr.h \
    converter.h


macx: LIBS += -L$$PWD/../../../../usr/local/Cellar/tesseract/3.05.00/lib/ -ltesseract.3

INCLUDEPATH += $$PWD/../../../../usr/local/Cellar/tesseract/3.05.00/include
DEPENDPATH += $$PWD/../../../../usr/local/Cellar/tesseract/3.05.00/include

macx: LIBS += -L$$PWD/../../../../usr/local/Cellar/ghostscript/9.21_1/lib/ -lgs.9.21

INCLUDEPATH += $$PWD/../../../../usr/local/Cellar/ghostscript/9.21_1/include
DEPENDPATH += $$PWD/../../../../usr/local/Cellar/ghostscript/9.21_1/include
