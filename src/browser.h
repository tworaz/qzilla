// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BROWSER_H_
#define BROWSER_H_

#include <QObject>

#include "webwindow.h"
#include "webview.h"
#include "webviewlist.h"

class QMozContext;
class QQmlComponent;
class QQmlApplicationEngine;

class Browser : public QObject {
  Q_OBJECT
 public:
  explicit Browser(QObject* parent = 0);
  ~Browser();

  void SetInitialURL(QString url) { initial_url_ = url; }

  Q_INVOKABLE void CreateNewWebView(QString url);
  Q_INVOKABLE void CloseWebView(WebView*);
  Q_INVOKABLE void SetActiveWebView(WebView*);

  // Memory debug features.
  Q_INVOKABLE void DumpMemoryInfo();
  Q_INVOKABLE void MemoryPressure();

  // Graphics debug features.
  Q_INVOKABLE void ThrottlePainting(bool);
  Q_INVOKABLE void EnablePaintFlashing(bool);
  Q_INVOKABLE void DrawLayerInfo(bool);
  Q_INVOKABLE void DrawLayerBorders(bool);
  Q_INVOKABLE void ShowFPS(bool);
  Q_INVOKABLE void Rotate(Qt::ScreenOrientation);

 private slots:
  void OnMozContextInitialized();

 private:
  void ApplyCustomSettings();
  void SetGraphicsPref(const char*, QVariant);

  QScopedPointer<WebWindow> web_window_;
  WebViewList web_view_list_;
  QMozContext& context_;
  QString initial_url_;

  QScopedPointer<QQmlApplicationEngine> engine_;
  QScopedPointer<QQmlComponent> component_;

  Q_DISABLE_COPY(Browser);
};

#endif // BROWSER_H_
