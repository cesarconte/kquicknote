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
    width: 380
    height: 440
    visible: typeof QuickNote !== "undefined" && QuickNote ? QuickNote.visible : true
    title: i18n("KQuickNote — Nota Rápida")

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
                    QQC2.ToolTip.text: "Cambiar entre notas rápidas recientes"

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
                    QQC2.ToolTip.text: "Nueva nota rápida"
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
                    QQC2.ToolTip.text: "Insertar selección/portapapeles como cita (> ...)"
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
                    QQC2.ToolTip.text: "Sincronizar con Kiri RAG (~/.local/share/kiri/knowledge/)"
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
                    QQC2.ToolTip.text: "Eliminar nota actual"
                    onClicked: {
                        if (typeof QuickNote !== "undefined" && QuickNote) {
                            QuickNote.deleteNote(QuickNote.currentNoteId);
                        }
                    }
                }
            }

            // Recessed Markdown Editor Stage
            LiquidGlassCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                glassStyle: "recessed"

                QQC2.ScrollView {
                    anchors.fill: parent
                    anchors.margins: Kirigami.Units.smallSpacing
                    clip: true

                    QQC2.TextArea {
                        id: noteEditor
                        placeholderText: "Escribe una nota rápida en Markdown..."
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
                    width: 7
                    height: 7
                    radius: 3.5
                    color: Kirigami.Theme.positiveTextColor
                }

                QQC2.Label {
                    Layout.fillWidth: true
                    text: typeof QuickNote !== "undefined" && QuickNote && QuickNote.statusMessage.length > 0
                        ? QuickNote.statusMessage
                        : "Guardado automático en Markdown"
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
