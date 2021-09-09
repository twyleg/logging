import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Styles 1.4

Item {

	id: logView

	property color foregroundColor: "white"
	property color backgroundColor: "white"
	property real backgroundOpacity: 0.2
	property int fontSizePixel: 10
	property alias scrollLock: scrollLockCheckbox.checked

	Rectangle {
		id: backgroundRectangle

		border.color: backgroundColor
		border.width: 1
		anchors.fill: parent
		radius: 20
		color: logView.backgroundColor
		opacity: logView.backgroundOpacity
	}

	ColumnLayout {

		anchors.fill: parent
		spacing: 2

		Row {
			Layout.alignment: Qt.AlignHCenter
			Layout.topMargin: 2
			Layout.bottomMargin: 2

			spacing: 2

			Button {
				text: "+"
				onClicked: fontSizePixel += 5
				width: height
			}

			Button {
				text: "-"
				onClicked: fontSizePixel = Math.max(2, fontSizePixel-5)
				width: height
			}

			CheckBox {
				id: scrollLockCheckbox
				text: "Lock"
				checked: true
			}
		}

		ListView {

			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.topMargin: backgroundRectangle.radius/2
			Layout.bottomMargin: backgroundRectangle.radius/2
			Layout.leftMargin: backgroundRectangle.radius/2
			Layout.rightMargin: backgroundRectangle.radius/2

			clip: true
			flickableDirection: Flickable.HorizontalAndVerticalFlick

			onMovementStarted: scrollLock = false
			onMovementEnded: if (atYEnd) { scrollLock = true }

			onContentHeightChanged: {
				if (scrollLock) {
					contentY = contentHeight - height
				}
			}

			model: logMessages



			delegate: Text {
				font.pixelSize: fontSizePixel
				text: display
				color: logView.foregroundColor
			}

			ScrollBar.vertical: ScrollBar {
				active: true;
			}

		}

	}

}
