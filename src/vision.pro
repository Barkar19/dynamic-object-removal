TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    dynamicobjectremover.cpp \
    imageprovider.cpp
#LIBS += 'pkg-config --libs opencv'
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
        -lboost_filesystem

HEADERS += \
    dynamicobjectremover.h \
    imageprovider.h

