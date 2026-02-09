QT += testlib core
CONFIG += console testcase c++11
TEMPLATE = app
TARGET = tst_passwordcipher

INCLUDEPATH += $$PWD/../..

SOURCES += \
    tst_passwordcipher.cpp \
    ../../core/PasswordCipher.cpp

HEADERS += \
    ../../core/PasswordCipher.h

# OpenSSL
!isEmpty(OPENSSL_INCLUDE) {
    INCLUDEPATH += $$OPENSSL_INCLUDE
}
!isEmpty(OPENSSL_LIBPATH) {
    LIBS += -L$$OPENSSL_LIBPATH
}

win32 {
    LIBS += -llibcrypto
} else {
    LIBS += -lcrypto
}
