// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "browser.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>

#include <qmozcontext.h>
#include <qmozwindow.h>

namespace {

const char kHomePageURL[] = "http://www.google.com";
const int kDefaultWindowWidth = 540;
const int kDefaultWindowHeight = 960;

const char* kMemoryDumpMsg = "Memory:Dump";
const QString kMemoryPressureMsg = "memory-pressure";
const QString kMemoryPressureArg = "heap-minimize";

const char* kExtraComponentPaths[] = {
  "components/EmbedLiteBinComponents.manifest",
  "components/EmbedLiteJSComponents.manifest",
  "chrome/EmbedLiteJSScripts.manifest",
  "chrome/EmbedLiteOverrides.manifest"
};

struct {
  QString key;
  QVariant value;
} kCustomSettingsArray[] = {
  { QStringLiteral("nglayout.initialpaint.delay"), QVariant(500) },
  { QStringLiteral("browser.ui.touch.left"), QVariant(32) },
  { QStringLiteral("browser.ui.touch.right"), QVariant(32) },
  { QStringLiteral("browser.ui.touch.top"), QVariant(48) },
  { QStringLiteral("browser.ui.touch.bottom"), QVariant(16) },

  { QStringLiteral("embedlite.azpc.handle.singletap"), QVariant(false) },
  { QStringLiteral("embedlite.azpc.json.singletap"), QVariant(true) },
  { QStringLiteral("embedlite.azpc.handle.longtap"), QVariant(false) },
  { QStringLiteral("embedlite.azpc.json.longtap"), QVariant(true) },
  { QStringLiteral("embedlite.azpc.json.viewport"), QVariant(true) },

  { QStringLiteral("browser.download.useDownloadDir"), QVariant(true) },
  { QStringLiteral("browser.download.folderList"), QVariant(2) },
  { QStringLiteral("browser.download.dir"), QVariant("/tmp") },
  { QStringLiteral("browser.download.manager.retention"), QVariant(2) },
  { QStringLiteral("browser.download.manager.quitBehavior"), QVariant(2) },
  { QStringLiteral("browser.helperApps.deleteTempFileOnExit"), QVariant(true) },

  // Without this pref placeholders get cleaned as soon as a character gets committed
  // by VKB and that happens only when Enter is pressed or comma/space/dot is entered.
  { QStringLiteral("dom.placeholder.show_on_focus"), QVariant(false) },

  { QStringLiteral("security.alternate_certificate_error_page"), QVariant("certerror") },

  { QStringLiteral("geo.wifi.scan"), QVariant(false) },
  { QStringLiteral("browser.enable_automatic_image_resizing"), QVariant(true) },

  // Make long press timeout equal to the one in Qt
  { QStringLiteral("ui.click_hold_context_menus.delay"), QVariant(800) },
  { QStringLiteral("apz.fling_stopped_threshold"), QVariant("0.13f") },

  // Don't force 16bit color depth
  { QStringLiteral("gfx.qt.rgb16.force"), QVariant(false) },
  { QStringLiteral("gfx.compositor.clear-context"), QVariant(false) },
  { QStringLiteral("embedlite.compositor.external_gl_context"), QVariant(true) },
  { QStringLiteral("embedlite.compositor.request_external_gl_context_early"), QVariant(true) },

  { QStringLiteral("media.resource_handler_disabled"), QVariant(true) },

  // Disable asmjs
  { QStringLiteral("javascript.options.asmjs"), QVariant(false) },

  { QStringLiteral("keyword.enabled"), QVariant(true) },

  // Theme.fontSizeSmall
  { QStringLiteral("embedlite.inputItemSize"), QVariant(28) },
  { QStringLiteral("embedlite.zoomMargin"), QVariant(14) },

  // Memory management related preferences.
  // We're sending "memory-pressure" when browser is on background
  // (cover by another application) and when the browser page is inactivated.
  { QStringLiteral("javascript.options.gc_on_memory_pressure"), QVariant(true) },

  // Disable SSLv3
  { QStringLiteral("security.tls.version.min"), QVariant(1) },

  { QStringLiteral("layers.progressive-paint"), QVariant(true) },
  { QStringLiteral("layers.low-precision-buffer"), QVariant(true) },

  { QStringLiteral("layers.acceleration.force-enabled"), QVariant(true) },
  { QStringLiteral("layout.spammy_warnings.enabled"), QVariant(false) },
};

void LoadEmbedLiteComponents(const char* root_dir) {
  int arr_size = sizeof(kExtraComponentPaths) / sizeof(*kExtraComponentPaths);
  for (int i = 0; i < arr_size; ++i) {
    QString full_path;
    QTextStream tstream(&full_path);
    tstream << root_dir << "/" << kExtraComponentPaths[i];
    qDebug() << "Loading component from:" << full_path;
    QMozContext::GetInstance()->addComponentManifest(full_path);
  }
}

} // namespace

Browser::Browser(QObject* parent)
    : QObject(parent)
    , web_window_(new WebWindow)
    , context_(*QMozContext::GetInstance())
    , engine_(new QQmlApplicationEngine)
    , component_(new QQmlComponent(engine_.data()))
    , should_terminate_(false) {

  LoadEmbedLiteComponents(getenv("GRE_HOME"));

  connect(web_window_.data(), &WebWindow::WindowInitialized,
          this, &Browser::OnWindowInitialized);
  connect(web_window_.data(), &WebWindow::WindowClosed,
          this, &Browser::OnWindowClosed);

  web_window_->resize(kDefaultWindowWidth, kDefaultWindowHeight);

  engine_->rootContext()->setContextProperty("WebViewList", &web_view_list_);
  engine_->rootContext()->setContextProperty("Browser", this);
  component_->loadUrl(QUrl("qrc:/ui/Toolbox.qml"));

  if (!component_->isReady())
    qFatal("Failed to load QML component: %s",
           qPrintable(component_->errorString()));
  QObject* result = component_->create();
  toolbox_window_.reset(qobject_cast<QWindow*>(result));

  connect(&context_, &QMozContext::onInitialized,
          this, &Browser::OnMozContextInitialized);
  connect(&context_, &QMozContext::destroyed,
          this, &Browser::OnMozContextDestroyed);
  connect(&context_, &QMozContext::lastViewDestroyed,
          this, &Browser::OnLastViewDestroyed);
  connect(&context_, &QMozContext::lastWindowDestroyed,
          this, &Browser::OnLastWindowDestroyed);
}

Browser::~Browser() {
}

void
Browser::CreateNewWebView(QString url) {
  WebView* wv = new WebView(web_window_.data());
  web_view_list_.append(wv);
  web_window_->SetActiveWebView(wv);
  wv->load(url);
}

void
Browser::CloseWebView(WebView* wv) {
  wv->setActive(false);
  web_view_list_.remove(wv);
  web_window_->SetActiveWebView(web_view_list_.get(0));
  delete wv;
}

void
Browser::SetActiveWebView(WebView* wv) {
  Q_ASSERT(web_view_list_.contains(wv));
  web_window_->SetActiveWebView(wv);
}

void
Browser::DumpMemoryInfo() {
  WebView* wv = web_window_->ActiveWebView();
  if (wv) {
    QString dir = QDir::currentPath() + QDir::separator() + "memdumps";
    if (!QDir().mkpath(dir)) {
      qCritical() << "Failed to create directory:" << dir;
      return;
    }
    QDateTime date = QDateTime::currentDateTime();
    QMap<QString, QVariant> qmap;
    qmap["fileName"] = dir + QDir::separator() + "qzilla-memory-dump-" +
        date.toString("yyyyMMddhhmmss") + ".gz";
    wv->sendAsyncMessage(kMemoryDumpMsg, QVariant(qmap));
    qDebug() << "Saving engine memory dump to:" << qmap["fileName"].toString();
  } else {
    qWarning() << "Can't dump engine memory info without active webview!";
  }
}

void
Browser::MemoryPressure() {
  QMozContext::GetInstance()->sendObserve(kMemoryPressureMsg, kMemoryPressureArg);
}

void
Browser::ThrottlePainting(bool throttle) {
  WebView* wv = web_window_->ActiveWebView();
  if (wv) {
    qDebug() << "Paint throttling" << (throttle ? "enable" : "disabled");
    wv->setThrottlePainting(throttle);
  }
}

void
Browser::EnablePaintFlashing(bool enable) {
  SetGraphicsPref("nglayout.debug.paint_flashing", QVariant(enable));
}

void
Browser::DrawLayerInfo(bool draw) {
  SetGraphicsPref("layers.draw-layer-info", QVariant(draw));
}

void
Browser::DrawLayerBorders(bool draw) {
  SetGraphicsPref("layers.draw-borders", QVariant(draw));
}

void
Browser::ShowFPS(bool show) {
  SetGraphicsPref("layers.acceleration.draw-fps", QVariant(show));
}

void
Browser::Rotate(Qt::ScreenOrientation orientation) {
  web_window_->MozWindow()->setContentOrientation(orientation);
}

void
Browser::Quit() {
  qDebug() << "Browser received terminaton request from UI.";
  should_terminate_ = true;
  toolbox_window_->hide();
  web_view_list_.clear();
}

void
Browser::OnMozContextInitialized() {
  qDebug() << "Mozilla context initialized";
  web_window_->Initialize();
  ApplyCustomSettings();
}

void
Browser::OnMozContextDestroyed() {
  qDebug() << "Mozilla context destroyed";
  Q_ASSERT(should_terminate_);
  qApp->exit();
}

void
Browser::OnWindowInitialized() {
  CreateNewWebView(initial_url_.isEmpty() ? kHomePageURL : initial_url_);
  web_window_->show();
}

void
Browser::OnWindowClosed() {
  Quit();
}

void
Browser::OnLastViewDestroyed() {
  if (should_terminate_) {
    qDebug() << "Last EmbedLiteView destroyed.";
    web_window_.reset();
  }
}

void
Browser::OnLastWindowDestroyed() {
  if (should_terminate_) {
    qDebug() << "Last EmbedLiteWindow destroyed.";
    context_.stopEmbedding();
  }
}

void
Browser::ApplyCustomSettings() {
  context_.setIsAccelerated(true);
  context_.setPixelRatio(1.5f);

  int size = sizeof(kCustomSettingsArray) / sizeof(*kCustomSettingsArray);
  for (int i = 0; i < size; ++i)
    context_.setPref(kCustomSettingsArray[i].key, kCustomSettingsArray[i].value);
}

void
Browser::SetGraphicsPref(const char* key, QVariant val) {
  context_.setPref(key, val);
  WebView* wv = web_window_->ActiveWebView();
  if (wv) {
    wv->update();
  }
}
