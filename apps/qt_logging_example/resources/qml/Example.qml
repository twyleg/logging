// Copyright (C) 2021 twyleg
import QtQuick 2.15
import QtQuick.Window 2.1
import QtQuick.Controls 2.15
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4
import QtQuick.Controls.Material 2.12

ApplicationWindow {
	id: applicationWindow

	visible: true
	color: "black"
	title: qsTr("Qt log example")

	width: 640
	height: 480

	LogView {

		anchors.fill: parent
		anchors.leftMargin: 5
		anchors.rightMargin: 5
		anchors.topMargin: 5
		anchors.bottomMargin: 5
	}
}
