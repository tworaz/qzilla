// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview.h"

#include <QGuiApplication>
#include <QOpenGLContext>
#include <QDebug>
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
    , window_(window)
    , context_(0) {

  setWindow(window_);

  connect(this, SIGNAL(requestGLContext()),
          this, SLOT(CreateGLContext()), Qt::DirectConnection);
  connect(this, &QOpenGLWebPage::completedChanged,
          this, &WebView::OnCompletedChanged);
  connect(this, &QOpenGLWebPage::activeChanged,
          this, &WebView::OnActiveChanged);

  initialize();
  LoadFrameScripts(this);
}

WebView::~WebView() {
}

void
WebView::CreateGLContext() {
  if (!context_) {
    context_ = new QOpenGLContext();
    context_->setFormat(window_->requestedFormat());
    if (!context_->create())
      qFatal("Failed to create QOpenGLContext!");
    context_->makeCurrent(window_);
  } else {
    context_->makeCurrent(window_);
  }
  QOpenGLFunctions* functions = context_->functions();
  Q_ASSERT(functions);
  functions->glClearColor(1.0, 1.0, 1.0, 0.0);
  functions->glClear(GL_COLOR_BUFFER_BIT);
  context_->swapBuffers(window_);
}

void
WebView::OnCompletedChanged() {
  qDebug() << "WebView creation completed";
}

void
WebView::OnActiveChanged() {
  if (active()) {
    window_->SetActiveWebView(this);
  }
}
