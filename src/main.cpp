// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QGuiApplication>
#include <QTimer>
#include <qmozcontext.h>

#include "browser.h"
#include "webview.h"

int main(int argc, char* argv[]) {
  QVariant url;
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] != '-') {
      url = QString(argv[i]);
      break;
    }
  }

  setenv("CUSTOM_UA", "Mozilla/5.0 (Maemo; Linux; U; Jolla; Sailfish; Mobile; "
         "rv:31.0) Gecko/31.0 Firefox/31.0 SailfishBrowser/1.0", 1);
  setenv("USE_ASYNC", "1", 1);

  qmlRegisterType<QOpenGLWebPage>();
  qmlRegisterType<WebView>();
  qmlRegisterType<Browser>();

  QGuiApplication app(argc, argv);
  Browser browser;

  app.setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);

  if (url.isValid())
    browser.SetInitialURL(url.toString());

  QTimer::singleShot(0, QMozContext::GetInstance(), SLOT(runEmbedding()));
  QObject::connect(&app, SIGNAL(lastWindowClosed()),
      QMozContext::GetInstance(), SLOT(stopEmbedding()));

  return app.exec();
}
