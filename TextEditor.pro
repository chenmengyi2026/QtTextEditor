QT       += core gui widgets

CONFIG   += c++17

TARGET = TextEditor
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    finddialog.cpp \
    replacedialog.cpp \
    codeeditor.cpp

HEADERS += mainwindow.h \
    finddialog.h \
    replacedialog.h \
    codeeditor.h
