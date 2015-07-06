// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webwindow.h"

#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QResizeEvent>
#include <QThread>

#include "webview.h"

WebWindow::WebWindow(QWindow* parent)
    : QWindow(parent)
    , web_view_(nullptr)
    , clear_surface_task_(nullptr) {
  setSurfaceType(QWindow::OpenGLSurface);
  QSurfaceFormat format(requestedFormat());
  format.setAlphaBufferSize(0);
  setFormat(format);
  create();
}

void
WebWindow::SetActiveWebView(WebView* wv) {
  if (wv == web_view_) {
    qWarning() << "Tryig to activate already active webview";
    return;
  }

  if (web_view_ && web_view_ != wv) {
    disconnect(web_view_, &WebView::titleChanged,
               this, &WebWindow::OnTitleChanged);
    web_view_->setActive(false);
  }

  web_view_ = wv;

  if (web_view_) {
    web_view_->setActive(true);
    web_view_->update();

    connect(web_view_, &WebView::titleChanged,
            this, &WebWindow::OnTitleChanged);

    web_view_->setSize(QSizeF(width(), height()));
    web_view_->updateContentOrientation(Qt::PortraitOrientation);
  } else {
    ClearWindowSurface();
  }
}

QOpenGLContext*
WebWindow::GLContext() const {
  static QOpenGLContext* context = nullptr;
  if (!context) {
    context = new QOpenGLContext();
    context->setFormat(requestedFormat());
    if (!context->create())
      qFatal("Failed to create QOpenGLContext!");
  }
  return context;
}

bool
WebWindow::ClearWindowSurface() {
  QMutexLocker lock(&clear_surface_task_mutex_);
  if (clear_surface_task_) {
      return true;
  }
  clear_surface_task_ = QMozContext::GetInstance()->PostCompositorTask(
      &WebWindow::ClearWindowSurfaceImpl, this);
  return clear_surface_task_ != 0;
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
WebWindow::exposeEvent(QExposeEvent*) {
  if (QWindow::isExposed()) {
    web_view_->update();
  }
}

void
WebWindow::OnTitleChanged() {
  Q_ASSERT(web_view_);
  setTitle(web_view_->title());
}

void
WebWindow::ClearWindowSurfaceImpl(void* data) {
  WebWindow* ww = static_cast<WebWindow*>(data);
  QOpenGLContext* context = ww->GLContext();
  // The GL context should always be used from the same thread in which it was created.
  Q_ASSERT(context->thread() == QThread::currentThread());
  context->makeCurrent(ww);
  QOpenGLFunctions* funcs = context->functions();
  Q_ASSERT(funcs);
  funcs->glClearColor(1.0, 1.0, 1.0, 0.0);
  funcs->glClear(GL_COLOR_BUFFER_BIT);
  context->swapBuffers(ww);
}
