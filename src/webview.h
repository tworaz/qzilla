// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_VIEW_H_
#define WEB_VIEW_H_

#include <QWindow>
#include <QOpenGLFunctions>
#include <QElapsedTimer>
#include <qopenglwebpage.h>

class QOpenGLContext;
class WebWindow;

class WebView: public QOpenGLWebPage {
 Q_OBJECT

 public:
  explicit WebView(WebWindow* window, QObject *parent = 0);
  ~WebView();

 private slots:
  void CreateGLContext();
  void OnCompletedChanged();
  void OnActiveChanged();

 private:
  WebWindow* window_;
  QOpenGLContext* context_;
  QElapsedTimer startup_timer_;
};

#endif // !WEB_VIEW_H_
