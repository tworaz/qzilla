// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webwindow.h"

#include <QDebug>
#include <QGuiApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QResizeEvent>
#include <QThread>

#include <qmozwindow.h>

#include "webview.h"

WebWindow::WebWindow(QWindow* parent)
    : QWindow(parent)
    , moz_window_(nullptr)
    , web_view_(nullptr)
    , clear_surface_task_(nullptr) {
  setSurfaceType(QWindow::OpenGLSurface);
  //QSurfaceFormat format(requestedFormat());
  //format.setAlphaBufferSize(0);
  //setFormat(format);
  create();
  EnableMouseEvents(false);
}

WebWindow::~WebWindow() {
  QMutexLocker lock(&clear_surface_task_mutex_);
  if (clear_surface_task_) {
    QMozContext::GetInstance()->CancelTask(clear_surface_task_);
  }
}

void
WebWindow::Initialize() {
  moz_window_.reset(new QMozWindow);

  connect(moz_window_.data(), &QMozWindow::requestGLContext,
          this, &WebWindow::OnRequestGLContext, Qt::DirectConnection);
  connect(moz_window_.data(), &QMozWindow::initialized,
          this, &WebWindow::OnWindowInitialized);
  connect(moz_window_.data(), &QMozWindow::drawUnderlay,
          this, &WebWindow::OnDrawUnderlay, Qt::DirectConnection);

  moz_window_->setSize(QWindow::size());
}

void
WebWindow::SetActiveWebView(WebView* wv) {
  if (web_view_ && wv == web_view_) {
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
  } else {
    ClearWindowSurface();
  }
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
WebWindow::EnableMouseEvents(bool enable) {
  enable_mouse_events_ = enable;
  if (enable) {
    QGuiApplication::instance()->setAttribute(
        Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);
  } else {
    QGuiApplication::instance()->setAttribute(
        Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);
  }
}

void
WebWindow::resizeEvent(QResizeEvent* evt) {
  if (web_view_) {
    moz_window_->setSize(evt->size());
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
    if (web_view_) {
      web_view_->update();
    } else {
      ClearWindowSurface();
    }
  }
}

bool
WebWindow::event(QEvent* event) {
  if (event->type() == QEvent::Close) {
    emit WindowClosed();
  }
  return QWindow::event(event);
}

void
WebWindow::OnTitleChanged() {
  Q_ASSERT(web_view_);
  setTitle(web_view_->title());
}

void
WebWindow::OnWindowInitialized()
{
  emit WindowInitialized();
}

void
WebWindow::OnRequestGLContext() {
  QOpenGLContext* context = GLContext();
  context->makeCurrent(this);

  static bool firstClearDone = false;
  if (!firstClearDone) {
    QOpenGLFunctions* functions = context->functions();
    Q_ASSERT(functions);
    functions->glClearColor(1.0, 1.0, 1.0, 0.0);
    functions->glClear(GL_COLOR_BUFFER_BIT);
    if (QWindow::isExposed()) {
      context->swapBuffers(this);
    }
    firstClearDone = true;
  }
}

void
WebWindow::OnDrawUnderlay() {
  Q_ASSERT(GLContext());

  if (!web_view_) {
     return;
  }

  QOpenGLContext* context = GLContext();
  context->makeCurrent(this);
  QOpenGLFunctions* funcs = context->functions();
  Q_ASSERT(funcs);
  QColor bgColor = web_view_->bgcolor();
  funcs->glClearColor(bgColor.redF(), bgColor.greenF(), bgColor.blueF(), 0.0);
  funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

void
WebWindow::ClearWindowSurfaceImpl(void* data) {
  WebWindow* ww = static_cast<WebWindow*>(data);
  QMutexLocker lock(&ww->clear_surface_task_mutex_);

  QOpenGLContext* context = ww->GLContext();
  // The GL context should always be used from the same thread in which it was created.
  Q_ASSERT(context->thread() == QThread::currentThread());
  context->makeCurrent(ww);
  QOpenGLFunctions* funcs = context->functions();
  Q_ASSERT(funcs);
  funcs->glClearColor(1.0, 1.0, 1.0, 0.0);
  funcs->glClear(GL_COLOR_BUFFER_BIT);
  if (ww->isExposed()) {
    context->swapBuffers(ww);
  }

  ww->clear_surface_task_ = 0;
}
