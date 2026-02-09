QT += testlib core widgets sql
CONFIG += console testcase c++11
TEMPLATE = app
TARGET = tst_credentialstore

INCLUDEPATH += $$PWD/../..

SOURCES += \
    tst_credentialstore.cpp \
    test_stubs.cpp \
    ../../core/CredentialStore.cpp \
    ../../core/PasswordCipher.cpp \
    ../../core/LogParam.cpp

HEADERS += \
    ../../core/CredentialStore.h \
    ../../core/PasswordCipher.h \
    ../../core/LogParam.h

# QtKeychain
!isEmpty(QTKEYCHAININCLUDEPATH) {
   INCLUDEPATH += $$QTKEYCHAININCLUDEPATH
}

!isEmpty(QTKEYCHAINLIBPATH) {
   LIBS += -L$$QTKEYCHAINLIBPATH
}

equals(QT_MAJOR_VERSION, 6): LIBS += -lqt6keychain
equals(QT_MAJOR_VERSION, 5): LIBS += -lqt5keychain

# OpenSSL (for PasswordCipher)
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
