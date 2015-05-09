// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webwindow.h"

#include <QDebug>
#include <QResizeEvent>

#include "webview.h"

WebWindow::WebWindow(QWindow* parent)
    : QWindow(parent)
    , web_view_(0) {
  setSurfaceType(QWindow::OpenGLSurface);
  QSurfaceFormat format(requestedFormat());
  format.setAlphaBufferSize(0);
  setFormat(format);
  create();
}

void
WebWindow::SetActiveWebView(WebView* wv) {
  if (wv == web_view_)
    return;

  if (web_view_) {
    disconnect(web_view_, &WebView::titleChanged,
               this, &WebWindow::OnTitleChanged);
    web_view_->setActive(false);
  }

  web_view_ = wv;

  web_view_->setActive(true);

  connect(web_view_, &WebView::titleChanged,
          this, &WebWindow::OnTitleChanged);

  web_view_->setSize(QSizeF(width(), height()));
  web_view_->updateContentOrientation(Qt::PortraitOrientation);
}

void
WebWindow::resizeEvent(QResizeEvent* evt) {
  if (web_view_) {
    web_view_->setSize(evt->size());
    web_view_->updateContentOrientation(Qt::PortraitOrientation);
  }
}

void
WebWindow::focusInEvent(QFocusEvent* evt) {
  if (web_view_)
    web_view_->focusInEvent(evt);
}

void
WebWindow::focusOutEvent(QFocusEvent* evt) {
  if (web_view_)
    web_view_->focusOutEvent(evt);
}

void
WebWindow::touchEvent(QTouchEvent* evt) {
  if (web_view_)
    web_view_->touchEvent(evt);
}

void
WebWindow::mouseMoveEvent(QMouseEvent* evt) {
  // Let Qt synthesize touch event
  evt->ignore();
}

void
WebWindow::mousePressEvent(QMouseEvent* evt) {
  // Let Qt synthesize touch event
  evt->ignore();
}

void
WebWindow::mouseReleaseEvent(QMouseEvent* evt) {
  // Let Qt synthesize touch event
  evt->ignore();
}

void
WebWindow::keyPressEvent(QKeyEvent* evt) {
  if (web_view_)
    web_view_->keyPressEvent(evt);
}

void
WebWindow::keyReleaseEvent(QKeyEvent* evt) {
  if (web_view_)
    web_view_->keyReleaseEvent(evt);
}

void
WebWindow::OnTitleChanged() {
  Q_ASSERT(web_view_);
  setTitle(web_view_->title());
}
