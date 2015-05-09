// Copyright 2015 Jolla Mobile. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

ApplicationWindow {
  width: 600
  height: 700
  visible: true
  title: "QZilla Toolbox"

  readonly property string kHomePage: "https://www.google.com"
  property alias activeWebView: listView.activeWebView

  toolBar: ToolBar {
    RowLayout {
      anchors.fill: parent

      ToolButton {
        iconSource: "qrc:/icons/window-new.png"
        onClicked: Browser.CreateNewWebView(kHomePage)
      }
      ToolButton {
        enabled: activeWebView ? activeWebView.canGoBack : false
        iconSource: "qrc:/icons/back.png"
        onClicked: if (activeWebView) activeWebView.goBack()
      }
      ToolButton {
        enabled: activeWebView ? activeWebView.canGoForward : false
        iconSource: "qrc:/icons/forward.png"
        onClicked: if (activeWebView) activeWebView.goForward()
      }
      ToolButton {
        readonly property bool pageLoading: activeWebView ? activeWebView.loading : false
        iconSource: pageLoading ? "qrc:/icons/stop.png" : "qrc:/icons/reload.png";
        onClicked: {
          if (activeWebView) {
            pageLoading ? activeWebView.stop() : activeWebView.reload()
          }
        }
      }
      TextField {
        Layout.fillWidth: true
        text: activeWebView ? activeWebView.url : ""
        horizontalAlignment: TextInput.AlightRight
        onAccepted: {
          console.log("Loading page: " + text)
          console.log("ActiveWebView: " + activeWebView)
          if (activeWebView)
            activeWebView.load(text)
        }
      }
    }
  }

  ScrollView{
    anchors.fill: parent

    ListView {
      id: listView
      anchors.fill: parent
      spacing : 2

      property QtObject activeWebView: null

      model: WebViewList

      delegate: Rectangle {
        width: parent.width
        height: childrenRect.height
        color: WebView.active ? "steelblue" : "lightsteelblue"
        radius: 8

        Component.onCompleted: {
          if (WebView.active && !listView.activeWebView)
            listView.activeWebView = WebView
        }

        Connections {
          target: WebView
          onActiveChanged: {
            if (WebView.active) {
              listView.currentIndex = index
              listView.activeWebView = WebView
            }
          }
        }

        MouseArea {
          anchors.fill: parent
          onClicked: WebView.active = true
        }

        RowLayout {
          width: parent.width
          height: 40
          Text {
            Layout.fillWidth: true
            width: parent.width
            text: WebView.title
            font.pixelSize: 16
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
          }
          Item {
            width: 40
            height: 40
            ToolButton {
              anchors.centerIn: parent
              iconSource: "qrc:/icons/window-close.png"
              onClicked: console.log("TODO: handle tab closing")
            }
          }
        }
      }
    } // ListView
  } // ScrollView

  statusBar: StatusBar {
    ProgressBar {
      anchors.fill: parent
      visible: activeWebView ? activeWebView.loading : false
      minimumValue: 0
      maximumValue: 100
      value: activeWebView ? activeWebView.loadProgress : 0
    }
  }
}
