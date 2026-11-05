/*
    SPDX-FileCopyrightText: 2026 Cesar
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class QuickNoteController : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.cesar.kquicknote")

    Q_PROPERTY(QVariantList notes READ notes NOTIFY notesChanged)
    Q_PROPERTY(QString currentNoteId READ currentNoteId NOTIFY currentNoteChanged)
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY currentNoteChanged)
    Q_PROPERTY(QString currentContent READ currentContent NOTIFY currentContentChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(bool pinned READ isPinned WRITE setPinned NOTIFY pinnedChanged)
    Q_PROPERTY(bool hotCornerEnabled READ isHotCornerEnabled WRITE setHotCornerEnabled NOTIFY hotCornerEnabledChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int characterCount READ characterCount NOTIFY currentContentChanged)
    Q_PROPERTY(int wordCount READ wordCount NOTIFY currentContentChanged)

public:
    explicit QuickNoteController(const QString &notesDir = QString(), QObject *parent = nullptr);

    QVariantList notes() const;
    QString currentNoteId() const;
    QString currentTitle() const;
    QString currentContent() const;
    bool isVisible() const;
    void setVisible(bool visible);
    bool isPinned() const;
    void setPinned(bool pinned);
    bool isHotCornerEnabled() const;
    void setHotCornerEnabled(bool enabled);
    QString statusMessage() const;
    int characterCount() const;
    int wordCount() const;

    QString notesDir() const;
    QString kiriKnowledgeDir() const;
    void setKiriKnowledgeDir(const QString &dir);

    static QString deriveTitle(const QString &content);
    static QString formatAsBlockquote(const QString &rawText);

    Q_INVOKABLE void createNote(const QString &initialContent = QString());
    Q_INVOKABLE void selectNote(const QString &id);
    Q_INVOKABLE void updateCurrentContent(const QString &content);
    Q_INVOKABLE void deleteNote(const QString &id);
    Q_INVOKABLE void insertClipboardQuote();
    Q_INVOKABLE void insertQuoteText(const QString &text);
    Q_INVOKABLE bool syncToKiriKnowledge();
    Q_INVOKABLE void toggleVisible();
    Q_INVOKABLE void showOverlay();
    Q_INVOKABLE void hideOverlay();

public Q_SLOTS:
    Q_SCRIPTABLE void Toggle();
    Q_SCRIPTABLE void Show();
    Q_SCRIPTABLE void Hide();
    Q_SCRIPTABLE void NewNoteWithSelection();

Q_SIGNALS:
    void notesChanged();
    void currentNoteChanged();
    void currentContentChanged();
    void visibleChanged();
    void pinnedChanged();
    void hotCornerEnabledChanged();
    void statusMessageChanged();

private:
    void loadNotesIndex();
    QString noteFilePath(const QString &id) const;
    bool writeNoteFile(const QString &id, const QString &content);
    void setStatusMessage(const QString &msg);
    QString readSelectionOrClipboard() const;

    QString m_notesDir;
    QString m_kiriKnowledgeDir;
    QVariantList m_notes;
    QString m_currentNoteId;
    QString m_currentTitle;
    QString m_currentContent;
    bool m_visible = true;
    bool m_pinned = true;
    bool m_hotCornerEnabled = true;
    QString m_statusMessage;
};
