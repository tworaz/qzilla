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
    qDebug() << "GL context requested after" << startup_timer_.elapsed() << "ms";
  } else {
    context_->makeCurrent(window_);
  }
}

void
WebView::OnCompletedChanged() {
  qDebug() << "WebView creation completed";
  startup_timer_.start();
}

void
WebView::OnActiveChanged() {
  if (active()) {
    window_->SetActiveWebView(this);
  }
}
