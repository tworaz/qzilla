# Copyright (c) 2015 Jolla Mobile. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

TARGET = qzilla
CONFIG += c++11
QT += qml

SOURCES += \
	src/main.cpp \
	src/browser.cpp \
	src/webwindow.cpp \
	src/webview.cpp \
	src/webviewlist.cpp

HEADERS += \
	src/browser.h \
	src/webwindow.h \
	src/webview.h \
	src/webviewlist.h

RESOURCES += qzilla.qrc
