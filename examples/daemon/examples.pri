INCLUDEPATH += ../../../include

LIBS += -L$$OUT_PWD/../../../lib

TEMPLATE = app

QT += core daemon

CONFIG += console
CONFIG -= app_bundle


basepath = $$relative_path($$_PRO_FILE_PWD_, $$PWD)
basepath = $$dirname(basepath)
!isEmpty(basepath): basepath = $${basepath}/

target.path = $$[QT_INSTALL_EXAMPLES]/daemon/$${basepath}$${TARGET}
INSTALLS += target
