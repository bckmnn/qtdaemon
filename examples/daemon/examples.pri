# Prepare the example's base path and relative include/linker path
basepath = $$relative_path($$_PRO_FILE_PWD_, $$PWD)
basepath = $$dirname(basepath)

!isEmpty(basepath): basepath = $$basepath/

### Do the standard stuff

TEMPLATE = app

QT += core daemon

CONFIG += console
CONFIG -= app_bundle

target.path = $$[QT_INSTALL_EXAMPLES]/daemon/$${basepath}$${TARGET}
INSTALLS += target
