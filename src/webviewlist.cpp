// Copyright (c) 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webviewlist.h"

#include <QDebug>

WebViewList::WebViewList(QObject* parent)
    : QAbstractListModel(parent) {
  role_names_[Qt::UserRole] = "currentWebView";
}

void
WebViewList::append(WebView* obj) {
  beginInsertRows(QModelIndex(), data_.count(), data_.count());
  data_.append(obj);
  endInsertRows();
}

void
WebViewList::remove(WebView* wv) {
  int idx = data_.indexOf(wv);
  Q_ASSERT(idx >= 0 && idx < data_.size());
  beginRemoveRows(QModelIndex(), idx, idx);
  data_.removeAt(idx);
  endRemoveRows();
}

WebView*
WebViewList::get(int index) const {
  if (index < 0 || index >= data_.size())
    return NULL;
  return data_.at(index);
}

void
WebViewList::clear() {
  for (WebView* wv : data_) {
    delete wv;
  }
  data_.clear();
}

QVariant
WebViewList::data(const QModelIndex &index, int role) const {
  if (index.row() < 0 || index.row() >= data_.size())
      return QVariant();

  QByteArray roleName = role_names_.value(role);
  if (roleName.isEmpty()) {
      qWarning() << "No item for role " << role << " found!";
      return QVariant();
  }

  return QVariant::fromValue(data_.at(index.row()));
}

int
WebViewList::rowCount(const QModelIndex&) const {
  return data_.size();
}

QHash<int, QByteArray>
WebViewList::roleNames() const {
  return role_names_;
}
