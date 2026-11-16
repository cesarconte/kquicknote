/*
    SPDX-FileCopyrightText: 2026 Cesar
    SPDX-License-Identifier: GPL-3.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Dialogs as QtDialogs
import org.kde.kirigami as Kirigami
import CachyOS.UI

CachyWindow {
    id: root
    objectName: "quickNoteMainWindow"

    width: 720
    height: 540
    minimumWidth: 520
    minimumHeight: 380

    visible: typeof QuickNote !== "undefined" && QuickNote ? QuickNote.visible : true
    title: typeof QuickNote !== "undefined" && QuickNote && QuickNote.currentTitle.length > 0
        ? QuickNote.currentTitle
        : i18n("Notes — KQuickNote")

    showTrafficLights: true
    showTitleBarDivider: true
    showWindowFrame: true
    sidebarVisible: root.showSidebar
    sidebarWidth: 216
    cornerRadius: CachyTheme.radiusWindow

    property bool showSidebar: true
    property bool richPreviewMode: false

    onClosing: function(close) {
        if (typeof QuickNote !== "undefined" && QuickNote) {
            close.accepted = false;
            QuickNote.visible = false;
        }
    }

    // Golden Gate Notes Keyboard Shortcuts (15_notes.png)
    Shortcut {
        sequence: "Ctrl+N"
        onActivated: {
            if (typeof QuickNote !== "undefined" && QuickNote) {
                QuickNote.createNote("# Nueva nota\n\n");
                noteEditor.forceActiveFocus();
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+L"
        onActivated: {
            if (typeof QuickNote !== "undefined" && QuickNote) {
                QuickNote.insertHorizontalDivider();
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+D"
        onActivated: {
            if (typeof QuickNote !== "undefined" && QuickNote) {
                QuickNote.duplicateCurrentNote();
            }
        }
    }

    QtDialogs.FileDialog {
        id: importMdDialog
        title: i18n("Import Markdown…")
        fileMode: QtDialogs.FileDialog.OpenFile
        nameFilters: [i18n("Archivos Markdown (*.md *.markdown *.txt)"), i18n("Todos los archivos (*)")]
        onAccepted: {
            if (typeof QuickNote !== "undefined" && QuickNote) {
                QuickNote.importMarkdownFile(selectedFile.toString());
            }
        }
    }

    QtDialogs.FileDialog {
        id: exportMdDialog
        title: i18n("Export To Markdown…")
        fileMode: QtDialogs.FileDialog.SaveFile
        defaultSuffix: "md"
        nameFilters: [i18n("Markdown (*.md)")]
        onAccepted: {
            if (typeof QuickNote !== "undefined" && QuickNote) {
                QuickNote.exportToMarkdown(selectedFile.toString());
            }
        }
    }

    QtDialogs.FileDialog {
        id: exportPdfDialog
        title: i18n("Export To PDF…")
        fileMode: QtDialogs.FileDialog.SaveFile
        defaultSuffix: "pdf"
        nameFilters: [i18n("Documento PDF (*.pdf)")]
        onAccepted: {
            if (typeof QuickNote !== "undefined" && QuickNote) {
                QuickNote.exportToPdf(selectedFile.toString());
            }
        }
    }

    // =========================================================================
    // 1. GOLDEN GATE EDGE-TO-EDGE NOTES SIDEBAR
    // =========================================================================
    sidebarHeaderActions: [
        Row {
            spacing: 6
            anchors.verticalCenter: parent.verticalCenter

            CachyToolbarButton {
                text: "✎"
                tooltipText: i18n("Nueva nota (Ctrl+N)")
                onClicked: {
                    if (typeof QuickNote !== "undefined" && QuickNote) {
                        QuickNote.createNote("# Nueva nota\n\n");
                        noteEditor.forceActiveFocus();
                    }
                }
            }
        }
    ]

    sidebar: [
        ColumnLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            anchors.bottomMargin: 10
            spacing: 6

            Text {
                Layout.leftMargin: 8
                text: i18n("Notes")
                font.pixelSize: 11
                font.weight: Font.DemiBold
                color: CachyTheme.textMuted
            }

            QQC2.ScrollView {
                id: notesScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: availableWidth
                QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff
                clip: true

                ColumnLayout {
                    width: notesScroll.availableWidth
                    spacing: 6

                    Repeater {
                        model: typeof QuickNote !== "undefined" && QuickNote ? QuickNote.notes : []

                        Rectangle {
                            id: noteCard
                            required property var modelData
                            required property int index

                            readonly property bool isCurrent: typeof QuickNote !== "undefined" && QuickNote
                                && modelData.id === QuickNote.currentNoteId

                            Layout.fillWidth: true
                            Layout.maximumWidth: notesScroll.availableWidth
                            implicitHeight: noteCardCol.implicitHeight + 16
                            radius: 10
                            color: isCurrent
                                ? CachyTheme.withAlpha("#FFCC00", CachyTheme.isDark ? 0.22 : 0.26)
                                : (noteCardHover.hovered ? CachyTheme.cardBackgroundHover : "transparent")
                            border.width: isCurrent ? 1 : 0
                            border.color: isCurrent ? "#FFCC00" : "transparent"

                            HoverHandler { id: noteCardHover; cursorShape: Qt.PointingHandCursor }
                            TapHandler {
                                onTapped: {
                                    if (typeof QuickNote !== "undefined" && QuickNote) {
                                        QuickNote.selectNote(noteCard.modelData.id);
                                    }
                                }
                            }

                            ColumnLayout {
                                id: noteCardCol
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 2

                                Text {
                                    Layout.fillWidth: true
                                    text: noteCard.modelData.title || i18n("Nota sin título")
                                    font.pixelSize: 13
                                    font.weight: noteCard.isCurrent ? Font.Bold : Font.DemiBold
                                    color: CachyTheme.textPrimary
                                    elide: Text.ElideRight
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6

                                    Text {
                                        text: noteCard.modelData.updatedAt || ""
                                        font.pixelSize: 10
                                        color: CachyTheme.textMuted
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        text: noteCard.modelData.preview || ""
                                        font.pixelSize: 11
                                        color: CachyTheme.textSecondary
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    ]

    // =========================================================================
    // 2. HEADER TOOLBAR (Replicating 15_notes.png File -> Import/Export Markdown/PDF)
    // =========================================================================
    headerLeading: [
        RowLayout {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            CachyToolbarButton {
                text: "☰"
                tooltipText: i18n("Mostrar/ocultar barra lateral de notas")
                onClicked: root.showSidebar = !root.showSidebar
            }

            // Golden Gate "File" Menu (Exact replica of 15_notes.png — "Less iconic" clean menu)
            CachyToolbarButton {
                id: fileMenuBtn
                text: i18n("File ▾")
                implicitWidth: 54
                tooltipText: i18n("Menú Archivo (Importar/Exportar Markdown y PDF)")
                onClicked: goldenGateFileMenu.popup(0, fileMenuBtn.height + 4)

                QQC2.Menu {
                    id: goldenGateFileMenu

                    QQC2.MenuItem {
                        text: i18n("New Note\t⌘N")
                        onTriggered: {
                            if (typeof QuickNote !== "undefined" && QuickNote) {
                                QuickNote.createNote("# Nueva nota\n\n");
                                noteEditor.forceActiveFocus();
                            }
                        }
                    }

                    QQC2.MenuSeparator {}

                    QQC2.MenuItem {
                        text: i18n("Import Markdown…")
                        onTriggered: importMdDialog.open()
                    }

                    QQC2.Menu {
                        title: i18n("Export To")

                        QQC2.MenuItem {
                            text: i18n("PDF")
                            onTriggered: exportPdfDialog.open()
                        }

                        QQC2.MenuItem {
                            text: i18n("Markdown")
                            onTriggered: exportMdDialog.open()
                        }
                    }

                    QQC2.MenuSeparator {}

                    QQC2.MenuItem {
                        text: typeof QuickNote !== "undefined" && QuickNote && QuickNote.pinned
                            ? i18n("Unpin Note")
                            : i18n("Pin Note")
                        onTriggered: {
                            if (typeof QuickNote !== "undefined" && QuickNote) {
                                QuickNote.pinned = !QuickNote.pinned;
                            }
                        }
                    }

                    QQC2.MenuItem {
                        text: i18n("Duplicate Note\t⌘D")
                        onTriggered: {
                            if (typeof QuickNote !== "undefined" && QuickNote) {
                                QuickNote.duplicateCurrentNote();
                            }
                        }
                    }

                    QQC2.MenuSeparator {}

                    QQC2.MenuItem {
                        text: i18n("Delete Note")
                        onTriggered: {
                            if (typeof QuickNote !== "undefined" && QuickNote) {
                                QuickNote.deleteNote(QuickNote.currentNoteId);
                            }
                        }
                    }
                }
            }

            // Golden Gate Horizontal Divider Line (Ctrl+L)
            CachyToolbarButton {
                text: "―"
                tooltipText: i18n("Insertar línea divisoria horizontal (Ctrl+L)")
                onClicked: {
                    if (typeof QuickNote !== "undefined" && QuickNote) {
                        QuickNote.insertHorizontalDivider();
                    }
                }
            }

            // Insert Clipboard Quote
            CachyToolbarButton {
                text: "❝"
                tooltipText: i18n("Insertar selección o portapapeles como cita (> ...)")
                onClicked: {
                    if (typeof QuickNote !== "undefined" && QuickNote) {
                        QuickNote.insertClipboardQuote();
                    }
                }
            }
        }
    ]

    headerActions: [
        RowLayout {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            // Toggle Rich Formatted View vs Markdown Source
            CachyToolbarButton {
                text: root.richPreviewMode ? "MD" : "👁"
                checkable: true
                checked: root.richPreviewMode
                tooltipText: root.richPreviewMode
                    ? i18n("Volver al editor Markdown")
                    : i18n("Vista previa con formato enriquecido y líneas divisorias")
                onClicked: root.richPreviewMode = !root.richPreviewMode
            }

            // Sync to Kity RAG Knowledge Base
            CachyToolbarButton {
                text: "✦"
                tooltipText: i18n("Sincronizar nota con la base de conocimiento de Kity AI")
                onClicked: {
                    if (typeof QuickNote !== "undefined" && QuickNote) {
                        QuickNote.syncToKityKnowledge();
                    }
                }
            }

            // Pin Window
            CachyToolbarButton {
                text: "📌"
                checkable: true
                checked: typeof QuickNote !== "undefined" && QuickNote ? QuickNote.pinned : true
                tooltipText: i18n("Fijar nota en primer plano")
                onClicked: {
                    if (typeof QuickNote !== "undefined" && QuickNote) {
                        QuickNote.pinned = !QuickNote.pinned;
                    }
                }
            }
        }
    ]

    // =========================================================================
    // 3. MAIN NOTE EDITOR & FORMATTED VIEW (Strictly Zero Horizontal Overflow)
    // =========================================================================
    contentData: [
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 10

            // Editor / Rich Preview Surface
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 14
                color: CachyTheme.cardBackground
                border.width: 1
                border.color: CachyTheme.specularBorder
                clip: true

                // Markdown Editor Mode
                QQC2.ScrollView {
                    id: editorScroll
                    anchors.fill: parent
                    anchors.margins: 10
                    visible: !root.richPreviewMode
                    contentWidth: availableWidth
                    QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff
                    clip: true

                    QQC2.TextArea {
                        id: noteEditor
                        width: editorScroll.availableWidth
                        placeholderText: i18n("Escribe tus notas aquí (pulsa Ctrl+L para insertar un divisor horizontal)…")
                        text: typeof QuickNote !== "undefined" && QuickNote ? QuickNote.currentContent : ""
                        wrapMode: TextEdit.Wrap
                        selectByMouse: true
                        color: CachyTheme.textPrimary
                        placeholderTextColor: CachyTheme.textMuted
                        font.pixelSize: 14
                        background: null

                        onTextChanged: {
                            if (typeof QuickNote !== "undefined" && QuickNote && text !== QuickNote.currentContent) {
                                QuickNote.updateCurrentContent(text);
                            }
                        }
                    }
                }

                // Rich Formatted Markdown View Mode (Renders headers, lists, quotes, and Ctrl+L horizontal dividers)
                Flickable {
                    id: previewFlick
                    anchors.fill: parent
                    anchors.margins: 14
                    visible: root.richPreviewMode
                    contentWidth: width
                    contentHeight: richPreviewText.implicitHeight + 12
                    flickableDirection: Flickable.VerticalFlick
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true

                    QQC2.ScrollBar.vertical: QQC2.ScrollBar {
                        policy: QQC2.ScrollBar.AsNeeded
                    }

                    TextEdit {
                        id: richPreviewText
                        width: Math.max(100, previewFlick.width - 12)
                        readOnly: true
                        selectByMouse: true
                        wrapMode: TextEdit.Wrap
                        textFormat: TextEdit.MarkdownText
                        color: CachyTheme.textPrimary
                        font.pixelSize: 14
                        text: typeof QuickNote !== "undefined" && QuickNote ? QuickNote.currentContent : ""
                    }
                }
            }

            // Bottom Status Bar: Auto-save status + Word/Char counter
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: "#30D158"
                }

                Text {
                    Layout.fillWidth: true
                    text: typeof QuickNote !== "undefined" && QuickNote && QuickNote.statusMessage.length > 0
                        ? QuickNote.statusMessage
                        : i18n("Guardado automático · Markdown & PDF (macOS Golden Gate)")
                    color: CachyTheme.textSecondary
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }

                Text {
                    text: {
                        const words = typeof QuickNote !== "undefined" && QuickNote ? QuickNote.wordCount : 0;
                        const chars = typeof QuickNote !== "undefined" && QuickNote ? QuickNote.characterCount : 0;
                        return words + " pal · " + chars + " car";
                    }
                    color: CachyTheme.textMuted
                    font.pixelSize: 11
                }
            }
        }
    ]
}
