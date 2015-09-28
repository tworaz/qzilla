// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_VIEW_H_
#define WEB_VIEW_H_

#include <QWindow>
#include <qopenglwebpage.h>

class QOpenGLContext;
class WebWindow;

class WebView: public QOpenGLWebPage {
 Q_OBJECT

 public:
  explicit WebView(WebWindow* window);
  ~WebView();

 private slots:
  void OnViewInitialized();

 private:
  WebWindow* window_;

  Q_DISABLE_COPY(WebView);
};

#endif // !WEB_VIEW_H_
