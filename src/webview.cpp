// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview.h"

#include <QDebug>
#include <QGuiApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <qmozcontext.h>

#include <QOpenGLContext>

#include "webwindow.h"

namespace {

void LoadFrameScripts(QOpenGLWebPage* page) {
  static const char* kFrameScripts[] = {
    "chrome://embedlite/content/SelectAsyncHelper.js",
    "chrome://embedlite/content/embedhelper.js",
  };

  int arr_size = sizeof(kFrameScripts) / sizeof(*kFrameScripts);
  for (int i = 0; i < arr_size; ++i) {
    qDebug() << "Loading frame script:" << kFrameScripts[i];
    page->loadFrameScript(kFrameScripts[i]);
  }
}

} // namespace

WebView::WebView(WebWindow* window, QObject* parent)
    : QOpenGLWebPage(parent)
    , window_(window) {

  setWindow(window_);

  connect(this, SIGNAL(requestGLContext()),
          this, SLOT(CreateGLContext()), Qt::DirectConnection);

  initialize();
  LoadFrameScripts(this);
}

WebView::~WebView() {
}

void
WebView::CreateGLContext() {
  QOpenGLContext* context = window_->GLContext();
  context->makeCurrent(window_);

  QOpenGLFunctions* functions = context->functions();
  Q_ASSERT(functions);
  functions->glClearColor(1.0, 1.0, 1.0, 0.0);
  functions->glClear(GL_COLOR_BUFFER_BIT);
  context->swapBuffers(window_);
}
