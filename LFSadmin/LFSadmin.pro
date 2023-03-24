#-------------------------------------------------
#
# Project created by QtCreator 2022-03-10T15:52:34
#
#-------------------------------------------------

QT       += core

TARGET = LFSadmin
TEMPLATE = lib

CONFIG += C++11 plugin

DEFINES += LFSADMIN_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../sys/synch.cpp \
        LFSadmin.cpp \
    ../sys/exception.cpp \
    ../sys/library.cpp \
    ../sys/shmem.cpp \
    ../log/log.cpp \
    ../sys/msgwnd.cpp \
    inisetting.cpp \
    ../lfs/version.cpp

HEADERS += \
    ../sys/synch.hpp \
        LFSadmin.h \
    ../util/constraints.hpp \
    ../util/memory.hpp \
    ../util/methodscope.hpp \
    ../util/noexcept.hpp \
    ../util/operators.hpp \
    ../util/singleton.hpp \
    ../sys/exception.hpp \
    ../sys/library.hpp \
    ../sys/shmem.hpp \
    ../log/log.hpp \
    ../sys/handle.hpp \
    ../sys/msgwnd.hpp \
    spmsg_sample.h \
    ../msgqueue/MessageQueue.h \
    ../msgqueue/Semaphore.h \
    ../msgqueue/SharedMemory.h \
    inisetting.h \
    ../lfs/version.hpp

INCLUDEPATH += ..

unix {
    target.path = /usr/lib
    INSTALLS += target
}

LIBS += -L/usr/local/lib -lboost_system

QMAKE_LFLAGS += -Wl,-rpath=/usr/local/lib:.

