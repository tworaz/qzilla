// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_WINDOW_H_
#define WEB_WINDOW_H_

#include <QWindow>
#include <QMutex>
#include <qmozcontext.h>

class WebView;
class QOpenGLContext;

class WebWindow : public QWindow {
  Q_OBJECT
 public:
  explicit WebWindow(QWindow* parent = 0);

  void SetActiveWebView(WebView* wv);
  WebView* ActiveWebView() const { return web_view_; }
  QOpenGLContext* GLContext() const;

  bool ClearWindowSurface();

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
  void exposeEvent(QExposeEvent *) override;

 private slots:
  void OnTitleChanged();

 private:
  static void ClearWindowSurfaceImpl(void* data);

  WebView* web_view_;

  QMutex clear_surface_task_mutex_;
  QMozContext::TaskHandle clear_surface_task_;
};

#endif // WEB_WINDOW_H_
