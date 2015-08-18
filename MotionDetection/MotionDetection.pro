#-------------------------------------------------
#
# Project created by QtCreator 2013-12-15T23:31:38
#
#-------------------------------------------------

QT       += core gui

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MotionDetection
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    videoprocessing.cpp \
    displaywidget.cpp \
    myapplication.cpp \
    test.cpp \
    featureextraction.cpp \
    preprocess.cpp \
    trainingwidget.cpp \
    classification.cpp \
    initialprob.cpp \
    updateprob.cpp \
    munkres.cpp \
    displayresult.cpp \
    HungarianAlg.cpp \
    updatenew.cpp \
    iohandle.cpp \
    rectifywidget.cpp


HEADERS  += mainwindow.h \
    videoprocessing.h \
    displaywidget.h \
    myapplication.h \
    test.h \
    featureextraction.h \
    preprocess.h \
    trainingwidget.h \
    classification.h \
    initialprob.h \
    updateprob.h \
    munkres.h \
    displayresult.h \
    HungarianAlg.h \
    updatenew.h \
    iohandle.h \
    rectifywidget.h


INCLUDEPATH += /usr/local/include/opencv
LIBS += -L/usr/local/lib
LIBS += -lopencv_core
LIBS += -lopencv_imgproc
LIBS += -lopencv_highgui
LIBS += -lopencv_ml
LIBS += -lopencv_video
LIBS += -lopencv_features2d
LIBS += -lopencv_calib3d
LIBS += -lopencv_objdetect
LIBS += -lopencv_contrib
LIBS += -lopencv_legacy
LIBS += -lopencv_flann


