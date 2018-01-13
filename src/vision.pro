TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11


SOURCES += main.cpp \
    dynamicobjectremover.cpp \
    imageprovider.cpp \
    imagematcher.cpp \
    xmlsettings.cpp \
    imageprocesspipe.cpp
#LIBS += 'pkg-config --libs opencv'

LIBS += -L$$PWD/../ext/opencv-3.3.1/bin/lib/
INCLUDEPATH += $$PWD/../ext/opencv-3.3.1/bin/include
INCLUDEPATH += $$PWD/../ext/rapidxml

LIBS += -lopencv_face \
        -lopencv_shape \
        -lopencv_stitching \
        -lopencv_objdetect \
        -lopencv_superres \
        -lopencv_videostab \
        -lopencv_calib3d \
        -lopencv_features2d \
        -lopencv_highgui \
        -lopencv_videoio \
        -lopencv_imgcodecs \
        -lopencv_video \
        -lopencv_photo \
        -lopencv_ml \
        -lopencv_imgproc \
        -lopencv_flann \
        -lopencv_core \
        -lboost_system \
        -lboost_filesystem \
        -lboost_program_options \
        -lopencv_xfeatures2d

HEADERS += \
    dynamicobjectremover.h \
    imageprovider.h \
    imagematcher.h \
    xmlsettings.h \
    imageprocesspipe.h


