/*
    SPDX-FileCopyrightText: 2026 Cesar
    SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.layershell as LayerShell
import org.kde.kirigami as Kirigami

Window {
    id: peekerWindow
    objectName: "quickNoteCornerPeeker"

    width: 68
    height: 68
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowDoesNotAcceptFocus

    visible: typeof QuickNote !== "undefined" && QuickNote ? (QuickNote.hotCornerEnabled && !QuickNote.visible) : true

    LayerShell.Window.anchors: LayerShell.Window.AnchorBottom | LayerShell.Window.AnchorRight
    LayerShell.Window.layer: LayerShell.Window.LayerOverlay
    LayerShell.Window.keyboardInteractivity: LayerShell.Window.KeyboardInteractivityNone
    LayerShell.Window.exclusionZone: 0

    property bool isExpanded: false

    Timer {
        id: autoHideTimer
        interval: 1200
        repeat: false
        onTriggered: {
            if (!mouseZone.containsMouse) {
                peekerWindow.isExpanded = false;
            }
        }
    }

    Item {
        anchors.fill: parent

        // Corner Hitbox Mouse Area
        MouseArea {
            id: mouseZone
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onEntered: {
                autoHideTimer.stop();
                peekerWindow.isExpanded = true;
            }

            onExited: {
                autoHideTimer.restart();
            }

            onClicked: {
                peekerWindow.isExpanded = false;
                if (typeof QuickNote !== "undefined" && QuickNote) {
                    QuickNote.Show();
                }
            }
        }

        // Animated macOS Quick Note Peek Thumbnail
        Rectangle {
            id: noteCard
            width: 48
            height: 48
            radius: 12

            // Hidden position: tucked into bottom-right corner with 6px peeking
            // Expanded position: popped out 12px from both edges
            x: peekerWindow.isExpanded ? (parent.width - width - 10) : (parent.width - 8)
            y: peekerWindow.isExpanded ? (parent.height - height - 10) : (parent.height - 8)
            opacity: peekerWindow.isExpanded ? 1.0 : 0.45

            Behavior on x {
                NumberAnimation { duration: 250; easing.type: Easing.OutBack; easing.overshoot: 1.2 }
            }
            Behavior on y {
                NumberAnimation { duration: 250; easing.type: Easing.OutBack; easing.overshoot: 1.2 }
            }
            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }

            // Liquid Glass styling
            color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.15)
            border.width: 1.5
            border.color: peekerWindow.isExpanded ? Kirigami.Theme.highlightColor : Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.25)

            // Inner Note Icon
            Kirigami.Icon {
                anchors.centerIn: parent
                source: "document-edit-symbolic"
                implicitWidth: 22
                implicitHeight: 22
                color: peekerWindow.isExpanded ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
            }

            // Dog-ear folded corner indicator (macOS Notes style)
            Rectangle {
                anchors.top: parent.top
                anchors.right: parent.right
                width: 10
                height: 10
                color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.2)
                radius: 2
                visible: peekerWindow.isExpanded
            }
        }
    }
}
