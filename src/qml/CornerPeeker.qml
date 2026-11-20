/*
    SPDX-FileCopyrightText: 2026 Cesar
    SPDX-License-Identifier: GPL-3.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.layershell as LayerShell
import CachyOS.UI

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
            color: CachyTheme.withAlpha(CachyTheme.accentColor, 0.15)
            border.width: 1.5
            border.color: peekerWindow.isExpanded ? CachyTheme.accentColor : CachyPalette.specularBorder

            // Inner Note Icon
            CachyIcon {
                anchors.centerIn: parent
                name: "document-edit-symbolic"
                fallback: "document-edit"
                size: 22
                color: peekerWindow.isExpanded ? CachyTheme.accentColor : CachyPalette.text
            }

            // Dog-ear folded corner indicator (macOS Notes style)
            Rectangle {
                anchors.top: parent.top
                anchors.right: parent.right
                width: 10
                height: 10
                color: CachyTheme.withAlpha(CachyPalette.text, 0.2)
                radius: 2
                visible: peekerWindow.isExpanded
            }
        }
    }
}
