// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_VIEW_LIST_H_
#define WEB_VIEW_LIST_H_

#include <QAbstractListModel>

#include "webview.h"

class WebViewList : public QAbstractListModel {
  Q_OBJECT;
 public:
  explicit WebViewList(QObject* parent = 0);

  void append(WebView*);
  Q_INVOKABLE WebView* get(int index) const;

  // QAbstractListModel overrides
  QVariant data(const QModelIndex &index, int role) const override;
  int rowCount(const QModelIndex&) const override;
  QHash<int, QByteArray> roleNames() const override;

 private:
  QHash<int, QByteArray> role_names_;
  QList<WebView*> data_;

  Q_DISABLE_COPY(WebViewList)
};

#endif // WEB_VIEW_LIST_H_

