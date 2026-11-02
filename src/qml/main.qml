/*
    SPDX-FileCopyrightText: 2026 Cesar
    SPDX-License-Identifier: GPL-3.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root
    width: 440
    height: 520
    minimumWidth: 320
    minimumHeight: 300
    visible: typeof QuickNote !== "undefined" && QuickNote ? QuickNote.visible : true
    title: typeof QuickNote !== "undefined" && QuickNote && QuickNote.currentTitle.length > 0
        ? (QuickNote.currentTitle + " — KQuickNote")
        : i18n("KQuickNote — Nota Rápida")

    onClosing: function(close) {
        if (typeof QuickNote !== "undefined" && QuickNote) {
            close.accepted = false;
            QuickNote.visible = false;
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.mediumSpacing

        // Action Toolbar: Note Selector + Quick Actions
        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            QQC2.ComboBox {
                id: noteSelector
                Layout.fillWidth: true
                Layout.minimumWidth: 140
                model: typeof QuickNote !== "undefined" && QuickNote ? QuickNote.notes : []
                textRole: "title"
                valueRole: "id"
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18n("Cambiar entre notas rápidas recientes")

                onActivated: function(index) {
                    if (typeof QuickNote !== "undefined" && QuickNote && index >= 0 && index < QuickNote.notes.length) {
                        const selected = QuickNote.notes[index];
                        if (selected && selected.id) {
                            QuickNote.selectNote(selected.id);
                        }
                    }
                }

                Connections {
                    target: typeof QuickNote !== "undefined" ? QuickNote : null
                    function onCurrentNoteChanged() {
                        if (!QuickNote) {
                            return;
                        }
                        for (let i = 0; i < QuickNote.notes.length; ++i) {
                            if (QuickNote.notes[i].id === QuickNote.currentNoteId) {
                                noteSelector.currentIndex = i;
                                break;
                            }
                        }
                    }
                }
            }

            QQC2.ToolButton {
                icon.name: "list-add"
                display: QQC2.AbstractButton.IconOnly
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18n("Nueva nota rápida")
                onClicked: {
                    if (typeof QuickNote !== "undefined" && QuickNote) {
                        QuickNote.createNote("# Nueva nota\n\n");
                        noteEditor.forceActiveFocus();
                    }
                }
            }

            QQC2.ToolButton {
                icon.name: "edit-paste"
                display: QQC2.AbstractButton.IconOnly
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18n("Insertar selección/portapapeles como cita (> ...)")
                onClicked: {
                    if (typeof QuickNote !== "undefined" && QuickNote) {
                        QuickNote.insertClipboardQuote();
                    }
                }
            }

            QQC2.ToolButton {
                icon.name: "document-export"
                display: QQC2.AbstractButton.IconOnly
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18n("Sincronizar con Kiri RAG (~/.local/share/kiri/knowledge/)")
                onClicked: {
                    if (typeof QuickNote !== "undefined" && QuickNote) {
                        QuickNote.syncToKiriKnowledge();
                    }
                }
            }

            QQC2.ToolButton {
                icon.name: "edit-delete"
                display: QQC2.AbstractButton.IconOnly
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18n("Eliminar nota actual")
                onClicked: {
                    if (typeof QuickNote !== "undefined" && QuickNote) {
                        QuickNote.deleteNote(QuickNote.currentNoteId);
                    }
                }
            }
        }

        // Native Breeze Framed Markdown Editor
        QQC2.Frame {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 0

            QQC2.ScrollView {
                anchors.fill: parent
                clip: true

                QQC2.TextArea {
                    id: noteEditor
                    placeholderText: i18n("Escribe una nota rápida en Markdown...")
                    text: typeof QuickNote !== "undefined" && QuickNote ? QuickNote.currentContent : ""
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                    color: Kirigami.Theme.textColor
                    placeholderTextColor: Kirigami.Theme.disabledTextColor
                    font.pixelSize: 13
                    background: null

                    onTextChanged: {
                        if (typeof QuickNote !== "undefined" && QuickNote && text !== QuickNote.currentContent) {
                            QuickNote.updateCurrentContent(text);
                        }
                    }
                }
            }
        }

        // Bottom Status Bar: Auto-save status + Word/Char counter
        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: Kirigami.Theme.positiveTextColor
            }

            QQC2.Label {
                Layout.fillWidth: true
                text: typeof QuickNote !== "undefined" && QuickNote && QuickNote.statusMessage.length > 0
                    ? QuickNote.statusMessage
                    : i18n("Guardado automático en Markdown")
                color: Kirigami.Theme.disabledTextColor
                font.pixelSize: 11
                elide: Text.ElideRight
            }

            QQC2.Label {
                text: {
                    const words = typeof QuickNote !== "undefined" && QuickNote ? QuickNote.wordCount : 0;
                    const chars = typeof QuickNote !== "undefined" && QuickNote ? QuickNote.characterCount : 0;
                    return words + " pal · " + chars + " car";
                }
                color: Kirigami.Theme.disabledTextColor
                font.pixelSize: 11
            }
        }
    }
}
