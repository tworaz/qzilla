// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_WINDOW_H_
#define WEB_WINDOW_H_

#include <QWindow>
#include <QAbstractListModel>

class WebView;

class WebWindow : public QWindow {
  Q_OBJECT
 public:
  explicit WebWindow(QWindow* parent = 0);

  void SetActiveWebView(WebView* wv);

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
  WebView* web_view_;
};

#endif // WEB_WINDOW_H_
