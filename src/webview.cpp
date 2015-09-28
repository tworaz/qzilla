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

WebView::WebView(WebWindow* window)
    : QOpenGLWebPage(nullptr)
    , window_(window) {

  setMozWindow(window->MozWindow());

  connect(this, &QOpenGLWebPage::viewInitialized,
          this, &WebView::OnViewInitialized);

  initialize();
  LoadFrameScripts(this);
}

WebView::~WebView() {
}

void
WebView::OnViewInitialized() {
  qDebug() << "View initialized: " << this;
}
