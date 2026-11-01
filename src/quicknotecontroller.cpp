/*
    SPDX-FileCopyrightText: 2026 Cesar
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "quicknotecontroller.h"

#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUuid>
#include <QVariantMap>

static void initQuickNoteResources()
{
    Q_INIT_RESOURCE(resources);
}

QuickNoteController::QuickNoteController(const QString &notesDir, QObject *parent)
    : QObject(parent)
{
    initQuickNoteResources();

    if (notesDir.trimmed().isEmpty()) {
        const QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        m_notesDir = docs + QStringLiteral("/QuickNotes");
    } else {
        m_notesDir = notesDir;
    }

    const QString genericData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    m_kiriKnowledgeDir = genericData + QStringLiteral("/kiri/knowledge");

    QDir().mkpath(m_notesDir);
    loadNotesIndex();

    if (m_notes.isEmpty()) {
        createNote(QStringLiteral("# Nota rápida\n\nEscribe tus ideas en Markdown aquí..."));
    } else {
        const QVariantMap first = m_notes.first().toMap();
        selectNote(first.value(QStringLiteral("id")).toString());
    }
}

QVariantList QuickNoteController::notes() const
{
    return m_notes;
}

QString QuickNoteController::currentNoteId() const
{
    return m_currentNoteId;
}

QString QuickNoteController::currentTitle() const
{
    return m_currentTitle;
}

QString QuickNoteController::currentContent() const
{
    return m_currentContent;
}

bool QuickNoteController::isVisible() const
{
    return m_visible;
}

void QuickNoteController::setVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }
    m_visible = visible;
    Q_EMIT visibleChanged();
}

bool QuickNoteController::isPinned() const
{
    return m_pinned;
}

void QuickNoteController::setPinned(bool pinned)
{
    if (m_pinned == pinned) {
        return;
    }
    m_pinned = pinned;
    setStatusMessage(m_pinned ? QStringLiteral("Nota fijada en esquina")
                              : QStringLiteral("Nota desfijada"));
    Q_EMIT pinnedChanged();
}

QString QuickNoteController::statusMessage() const
{
    return m_statusMessage;
}

int QuickNoteController::characterCount() const
{
    return m_currentContent.length();
}

int QuickNoteController::wordCount() const
{
    const QString trimmed = m_currentContent.trimmed();
    if (trimmed.isEmpty()) {
        return 0;
    }
    static const QRegularExpression wsRe(QStringLiteral("\\s+"));
    return trimmed.split(wsRe, Qt::SkipEmptyParts).size();
}

QString QuickNoteController::notesDir() const
{
    return m_notesDir;
}

QString QuickNoteController::kiriKnowledgeDir() const
{
    return m_kiriKnowledgeDir;
}

void QuickNoteController::setKiriKnowledgeDir(const QString &dir)
{
    m_kiriKnowledgeDir = dir;
}

QString QuickNoteController::deriveTitle(const QString &content)
{
    const QStringList lines = content.split(QLatin1Char('\n'));
    for (const QString &rawLine : lines) {
        QString line = rawLine.trimmed();
        if (line.isEmpty()) {
            continue;
        }
        while (line.startsWith(QLatin1Char('#'))) {
            line.remove(0, 1);
        }
        line = line.trimmed();
        if (!line.isEmpty()) {
            if (line.length() > 60) {
                return line.left(57) + QStringLiteral("...");
            }
            return line;
        }
    }
    return QStringLiteral("Nota sin título");
}

QString QuickNoteController::formatAsBlockquote(const QString &rawText)
{
    const QString trimmed = rawText.trimmed();
    if (trimmed.isEmpty()) {
        return QString();
    }
    const QStringList lines = trimmed.split(QLatin1Char('\n'));
    QStringList quotedLines;
    quotedLines.reserve(lines.size());
    for (const QString &line : lines) {
        quotedLines.append(QStringLiteral("> ") + line);
    }
    return QStringLiteral("\n") + quotedLines.join(QLatin1Char('\n')) + QStringLiteral("\n\n");
}

void QuickNoteController::createNote(const QString &initialContent)
{
    QDir().mkpath(m_notesDir);

    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    const QString shortUuid = QUuid::createUuid().toString(QUuid::Id128).left(6);
    const QString id = QStringLiteral("%1-%2").arg(timestamp, shortUuid);

    if (!writeNoteFile(id, initialContent)) {
        setStatusMessage(QStringLiteral("Error al crear la nota en disco"));
        return;
    }

    m_currentNoteId = id;
    m_currentContent = initialContent;
    m_currentTitle = deriveTitle(initialContent);

    loadNotesIndex();
    setStatusMessage(QStringLiteral("Nueva nota creada"));
    Q_EMIT currentNoteChanged();
    Q_EMIT currentContentChanged();
}

void QuickNoteController::selectNote(const QString &id)
{
    if (id.trimmed().isEmpty()) {
        return;
    }

    const QString path = noteFilePath(id);
    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setStatusMessage(QStringLiteral("No se pudo abrir la nota seleccionada"));
        return;
    }

    const QString content = QString::fromUtf8(file.readAll());
    file.close();

    m_currentNoteId = id;
    m_currentContent = content;
    m_currentTitle = deriveTitle(content);

    setStatusMessage(QStringLiteral("Nota cargada"));
    Q_EMIT currentNoteChanged();
    Q_EMIT currentContentChanged();
}

void QuickNoteController::updateCurrentContent(const QString &content)
{
    if (m_currentNoteId.isEmpty()) {
        createNote(content);
        return;
    }

    if (m_currentContent == content && QFileInfo::exists(noteFilePath(m_currentNoteId))) {
        return;
    }

    if (!writeNoteFile(m_currentNoteId, content)) {
        setStatusMessage(QStringLiteral("Error al guardar la nota"));
        return;
    }

    const QString newTitle = deriveTitle(content);
    const bool titleChanged = (newTitle != m_currentTitle);

    m_currentContent = content;
    m_currentTitle = newTitle;

    loadNotesIndex();
    setStatusMessage(QStringLiteral("Guardado automáticamente"));

    if (titleChanged) {
        Q_EMIT currentNoteChanged();
    }
    Q_EMIT currentContentChanged();
}

void QuickNoteController::deleteNote(const QString &id)
{
    if (id.trimmed().isEmpty()) {
        return;
    }

    const QString path = noteFilePath(id);
    QFile::remove(path);

    loadNotesIndex();

    if (m_notes.isEmpty()) {
        createNote(QString());
        setStatusMessage(QStringLiteral("Nota eliminada"));
        return;
    }

    if (m_currentNoteId == id) {
        const QVariantMap first = m_notes.first().toMap();
        selectNote(first.value(QStringLiteral("id")).toString());
    }
    setStatusMessage(QStringLiteral("Nota eliminada"));
}

void QuickNoteController::insertClipboardQuote()
{
    const QString captured = readSelectionOrClipboard();
    if (captured.trimmed().isEmpty()) {
        setStatusMessage(QStringLiteral("No hay texto en la selección ni en el portapapeles"));
        return;
    }
    insertQuoteText(captured);
}

void QuickNoteController::insertQuoteText(const QString &text)
{
    const QString formatted = formatAsBlockquote(text);
    if (formatted.isEmpty()) {
        setStatusMessage(QStringLiteral("Texto de cita vacío"));
        return;
    }

    QString updated = m_currentContent;
    if (!updated.isEmpty() && !updated.endsWith(QLatin1Char('\n'))) {
        updated.append(QLatin1Char('\n'));
    }
    updated.append(formatted);
    updateCurrentContent(updated);
    setStatusMessage(QStringLiteral("Cita insertada en la nota"));
}

bool QuickNoteController::syncToKiriKnowledge()
{
    if (m_currentNoteId.isEmpty()) {
        setStatusMessage(QStringLiteral("No hay ninguna nota activa para sincronizar"));
        return false;
    }

    if (!QDir().mkpath(m_kiriKnowledgeDir)) {
        setStatusMessage(QStringLiteral("No se pudo crear el directorio de Kiri RAG"));
        return false;
    }

    const QString targetPath = QDir(m_kiriKnowledgeDir)
                                   .filePath(QStringLiteral("quicknote-%1.md").arg(m_currentNoteId));
    QFile outFile(targetPath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        setStatusMessage(QStringLiteral("Error al exportar a Kiri RAG"));
        return false;
    }

    outFile.write(m_currentContent.toUtf8());
    outFile.close();

    setStatusMessage(QStringLiteral("Sincronizado con Kiri RAG"));
    return true;
}

void QuickNoteController::toggleVisible()
{
    setVisible(!m_visible);
}

void QuickNoteController::showOverlay()
{
    setVisible(true);
}

void QuickNoteController::hideOverlay()
{
    setVisible(false);
}

void QuickNoteController::Toggle()
{
    toggleVisible();
}

void QuickNoteController::Show()
{
    showOverlay();
}

void QuickNoteController::Hide()
{
    hideOverlay();
}

void QuickNoteController::NewNoteWithSelection()
{
    showOverlay();
    createNote(QStringLiteral("# Captura rápida\n"));
    insertClipboardQuote();
}

void QuickNoteController::loadNotesIndex()
{
    QDir dir(m_notesDir);
    const QFileInfoList files = dir.entryInfoList(
        QStringList{QStringLiteral("*.md")},
        QDir::Files | QDir::Readable,
        QDir::Time | QDir::Name);

    QVariantList list;
    list.reserve(files.size());

    for (const QFileInfo &fi : files) {
        QFile file(fi.absoluteFilePath());
        QString content;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            content = QString::fromUtf8(file.readAll());
            file.close();
        }

        const QString id = fi.completeBaseName();
        const QString title = deriveTitle(content);

        QString preview = content.trimmed();
        preview.replace(QLatin1Char('\n'), QLatin1Char(' '));
        if (preview.length() > 80) {
            preview = preview.left(77) + QStringLiteral("...");
        }

        QVariantMap item;
        item.insert(QStringLiteral("id"), id);
        item.insert(QStringLiteral("title"), title);
        item.insert(QStringLiteral("filePath"), fi.absoluteFilePath());
        item.insert(QStringLiteral("updatedAt"),
                    fi.lastModified().toString(QStringLiteral("dd/MM HH:mm")));
        item.insert(QStringLiteral("preview"), preview);
        list.append(item);
    }

    m_notes = list;
    Q_EMIT notesChanged();
}

QString QuickNoteController::noteFilePath(const QString &id) const
{
    return QDir(m_notesDir).filePath(id + QStringLiteral(".md"));
}

bool QuickNoteController::writeNoteFile(const QString &id, const QString &content)
{
    QDir().mkpath(m_notesDir);
    QFile file(noteFilePath(id));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }
    file.write(content.toUtf8());
    file.close();
    return true;
}

void QuickNoteController::setStatusMessage(const QString &msg)
{
    if (m_statusMessage == msg) {
        return;
    }
    m_statusMessage = msg;
    Q_EMIT statusMessageChanged();
}

QString QuickNoteController::readSelectionOrClipboard() const
{
    if (const QClipboard *cb = QGuiApplication::clipboard()) {
        const QString primary = cb->text(QClipboard::Selection).trimmed();
        if (!primary.isEmpty()) {
            return primary;
        }
        const QString clip = cb->text(QClipboard::Clipboard).trimmed();
        if (!clip.isEmpty()) {
            return clip;
        }
    }

    const QString wlPaste = QStandardPaths::findExecutable(QStringLiteral("wl-paste"));
    if (!wlPaste.isEmpty()) {
        for (const QStringList &args : {
                 QStringList{QStringLiteral("--primary"), QStringLiteral("--no-newline")},
                 QStringList{QStringLiteral("--no-newline")}}) {
            QProcess proc;
            proc.start(wlPaste, args);
            if (proc.waitForFinished(1500) && proc.exitCode() == 0) {
                const QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
                if (!out.isEmpty()) {
                    return out;
                }
            }
        }
    }

    return QString();
}
