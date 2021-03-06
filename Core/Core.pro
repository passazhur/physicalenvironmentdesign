#/// \author Nazariy Jaworski

TEMPLATE    =   app

TARGET      =   Core

#QT dependences
CONFIG      +=  console
CONFIG      +=  no_keywords # no Qt-keywords for avoiding boost/Wt and Qt conflicts
CONFIG      +=  c++11
QT          +=  network
QT          +=  sql

#Wt Web ToolKit
INCLUDEPATH += -isystem E:\Wt\Wt_3.3.0_boost_1.54.0_mingw_4.8.0_x32\include\
INCLUDEPATH += -isystem E:\Boost\boost_1_54_0
LIBS += E:\Wt\Wt_3.3.0_boost_1.54.0_mingw_4.8.0_x32\lib\libwt.a
LIBS += E:\Wt\Wt_3.3.0_boost_1.54.0_mingw_4.8.0_x32\lib\libwthttp.a
LIBS += E:\Boost\boost_1_54_0\lib\libboost_signals-mgw48-mt-1_54.a
LIBS += E:\Boost\boost_1_54_0\lib\libboost_system-mgw48-mt-1_54.a
LIBS += E:\Boost\boost_1_54_0\lib\libboost_thread-mgw48-mt-1_54.a
LIBS += E:\Boost\boost_1_54_0\lib\libboost_filesystem-mgw48-mt-1_54.a
LIBS += E:\Boost\boost_1_54_0\lib\libboost_regex-mgw48-mt-1_54.a
LIBS += E:\Boost\boost_1_54_0\lib\libboost_random-mgw48-mt-1_54.a
LIBS += E:\Boost\boost_1_54_0\lib\libboost_program_options-mgw48-mt-1_54.a
LIBS += E:\Boost\boost_1_54_0\lib\libboost_date_time-mgw48-mt-1_54.a
LIBS += -lws2_32
LIBS += -lwsock32

#Flags
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs #disable a lot of boost warnings
QMAKE_CXXFLAGS += -Wno-write-strings

#Versions
system("SubWCRev $$PWD $$PWD/version.h.tpl $$OUT_PWD/version.h") #Tortise Svn is used

SOURCES += \
    main.cpp \
    core.cpp \
    commandserver.cpp \
    guard.cpp \
    logger.cpp \
    Ui/Web/uiwebserver.cpp \
    Ui/Web/mainwidget.cpp \
    Ui/Web/authenticationwidget.cpp \
    Ui/Web/DatabaseConnection/dbcwidget.cpp \
    Ui/Web/DatabaseConnection/dbcitemsubwidget.cpp \
    Ui/Web/DatabaseConnection/dbceditwidget.cpp \
    Ui/Web/DatabaseConnection/dbccreateconnectionitemwidget.cpp \
    Session/project.cpp \
    Session/usersession.cpp \
    DataBase/databasemanager.cpp \
    DataBase/databaseupdater.cpp \
    workersession.cpp

HEADERS += \
    core.h \
    commandserver.h \
    guard.h \
    logger.h \
    Ui/Web/uiwebserver.h \
    Ui/Web/mainwidget.h \
    Ui/Web/authenticationwidget.h \
    Ui/Web/DatabaseConnection/dbcwidget.h \
    Ui/Web/DatabaseConnection/dbcitemsubwidget.h \
    Ui/Web/DatabaseConnection/dbceditwidget.h \
    Ui/Web/DatabaseConnection/dbccreateconnectionitemwidget.h \
    Session/project.h \
    Session/usersession.h \
    DataBase/databasemanager.h \
    DataBase/databaseupdater.h \
    workersession.h \
    configurationparameters.h

OTHER_FILES += \
    Icons/mainicon.png \
    version.h.tpl
