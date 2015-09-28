// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_WINDOW_H_
#define WEB_WINDOW_H_

#include <QMutex>
#include <QScopedPointer>
#include <QWindow>
#include <qmozcontext.h>

class WebView;
class QOpenGLContext;
class QMozWindow;

class WebWindow : public QWindow {
  Q_OBJECT
 public:
  explicit WebWindow(QWindow* parent = 0);
  ~WebWindow();

  void Initialize();
  void SetActiveWebView(WebView* wv);
  WebView* ActiveWebView() const { return web_view_; }
  QMozWindow* MozWindow() const { return moz_window_.data(); }

  bool ClearWindowSurface();

  void EnableMouseEvents(bool);

 signals:
  void WindowInitialized();
  void WindowClosed();

 protected:
  void resizeEvent(QResizeEvent*) override;
  void focusInEvent(QFocusEvent*) override;
  void focusOutEvent(QFocusEvent*) override;
  void touchEvent(QTouchEvent*) override;
  void mouseMoveEvent(QMouseEvent*) override;
  void mousePressEvent(QMouseEvent*) override;
  void mouseReleaseEvent(QMouseEvent*) override;
  void keyPressEvent(QKeyEvent*) override;
  void keyReleaseEvent(QKeyEvent*) override;
  void exposeEvent(QExposeEvent*) override;
  bool event(QEvent*) override;

 private slots:
  void OnTitleChanged();
  void OnWindowInitialized();
  void OnRequestGLContext();
  void OnDrawUnderlay();

 private:
  QOpenGLContext* GLContext() const;
  static void ClearWindowSurfaceImpl(void* data);

  QScopedPointer<QMozWindow> moz_window_;
  WebView* web_view_;
  bool enable_mouse_events_;

  QMutex clear_surface_task_mutex_;
  QMozContext::TaskHandle clear_surface_task_;

  Q_DISABLE_COPY(WebWindow);
};

#endif // WEB_WINDOW_H_
