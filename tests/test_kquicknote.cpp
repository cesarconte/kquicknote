/*
    SPDX-FileCopyrightText: 2026 Cesar
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "quicknotecontroller.h"

#include <QDir>
#include <QFile>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

class TestKQuickNote : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCreateUpdateAndTitleDerivation()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QuickNoteController ctrl(tempDir.path());
        // Constructor creates an initial note when directory is empty
        QCOMPARE(ctrl.notes().size(), 1);
        QVERIFY(!ctrl.currentNoteId().isEmpty());

        ctrl.createNote(QStringLiteral("# Arquitectura Plasma 6.8\nNotas sobre integración nativa Breeze."));
        QCOMPARE(ctrl.notes().size(), 2);
        QCOMPARE(ctrl.currentTitle(), QStringLiteral("Arquitectura Plasma 6.8"));
        QVERIFY(ctrl.wordCount() >= 6);

        ctrl.updateCurrentContent(QStringLiteral("### Idea de Hot Corner\nActivar en esquina inferior derecha."));
        QCOMPARE(ctrl.currentTitle(), QStringLiteral("Idea de Hot Corner"));

        // Verify empty content fallback title
        QCOMPARE(QuickNoteController::deriveTitle(QStringLiteral("   \n  \n")),
                 QStringLiteral("Nota sin título"));
    }

    void testPersistenceAcrossControllerInstances()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString savedId;
        {
            QuickNoteController writer(tempDir.path());
            writer.updateCurrentContent(QStringLiteral("# Nota Persistente\nGuardada automáticamente en disco."));
            savedId = writer.currentNoteId();
            QVERIFY(QFile::exists(QDir(tempDir.path()).filePath(savedId + QStringLiteral(".md"))));
        }

        {
            QuickNoteController reader(tempDir.path());
            QCOMPARE(reader.notes().size(), 1);
            reader.selectNote(savedId);
            QCOMPARE(reader.currentNoteId(), savedId);
            QCOMPARE(reader.currentTitle(), QStringLiteral("Nota Persistente"));
            QVERIFY(reader.currentContent().contains(QStringLiteral("Guardada automáticamente en disco.")));
        }
    }

    void testQuoteInsertionAndDeleteNote()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QuickNoteController ctrl(tempDir.path());
        ctrl.updateCurrentContent(QStringLiteral("# Investigación\n"));
        const QString firstId = ctrl.currentNoteId();

        ctrl.insertQuoteText(QStringLiteral("Primera línea citada\nSegunda línea citada"));
        QVERIFY(ctrl.currentContent().contains(QStringLiteral("> Primera línea citada\n> Segunda línea citada")));

        ctrl.createNote(QStringLiteral("# Segunda nota"));
        const QString secondId = ctrl.currentNoteId();
        QCOMPARE(ctrl.notes().size(), 2);

        ctrl.deleteNote(secondId);
        QCOMPARE(ctrl.notes().size(), 1);
        QCOMPARE(ctrl.currentNoteId(), firstId);

        // Deleting the last note creates a fresh blank note so editor is never orphaned
        ctrl.deleteNote(firstId);
        QCOMPARE(ctrl.notes().size(), 1);
        QVERIFY(ctrl.currentNoteId() != firstId);
    }

    void testSyncToKiriKnowledge()
    {
        QTemporaryDir notesTempDir;
        QTemporaryDir kiriTempDir;
        QVERIFY(notesTempDir.isValid());
        QVERIFY(kiriTempDir.isValid());

        QuickNoteController ctrl(notesTempDir.path());
        ctrl.setKiriKnowledgeDir(kiriTempDir.path());
        ctrl.updateCurrentContent(QStringLiteral("# Resumen RAG\nSincronización directa con Kiri local knowledge."));

        QVERIFY(ctrl.syncToKiriKnowledge());

        const QString expectedFile = QDir(kiriTempDir.path())
                                         .filePath(QStringLiteral("quicknote-%1.md").arg(ctrl.currentNoteId()));
        QVERIFY(QFile::exists(expectedFile));

        QFile exported(expectedFile);
        QVERIFY(exported.open(QIODevice::ReadOnly | QIODevice::Text));
        const QString exportedText = QString::fromUtf8(exported.readAll());
        QVERIFY(exportedText.contains(QStringLiteral("Sincronización directa con Kiri local knowledge.")));
    }

    void testVisibilityAndDBusSlots()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QuickNoteController ctrl(tempDir.path());
        QVERIFY(ctrl.isVisible());

        ctrl.Hide();
        QVERIFY(!ctrl.isVisible());

        ctrl.Toggle();
        QVERIFY(ctrl.isVisible());

        ctrl.Show();
        QVERIFY(ctrl.isVisible());
    }

    void testQmlLoadsWithoutErrors()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        QuickNoteController ctrl(tempDir.path());

        QQmlEngine engine;
        engine.addImportPath(QStringLiteral("/home/cesar/.local/lib/qt6/qml"));
        engine.rootContext()->setContextProperty(QStringLiteral("QuickNote"), &ctrl);

        QQmlComponent comp(&engine, QUrl(QStringLiteral("qrc:/org/cesar/kquicknote/qml/main.qml")));
        QVERIFY2(!comp.isError(), qPrintable(comp.errorString()));

        QQmlComponent peekerComp(&engine, QUrl(QStringLiteral("qrc:/org/cesar/kquicknote/qml/CornerPeeker.qml")));
        QVERIFY2(!peekerComp.isError(), qPrintable(peekerComp.errorString()));
    }

    void testGoldenGateFeatures()
    {
        QTemporaryDir tempDir;
        QTemporaryDir exportDir;
        QVERIFY(tempDir.isValid());
        QVERIFY(exportDir.isValid());

        QuickNoteController ctrl(tempDir.path());
        ctrl.updateCurrentContent(QStringLiteral("# macOS 27 Golden Gate Notes\nSección 1"));

        // Horizontal divider insertion
        ctrl.insertHorizontalDivider();
        QVERIFY(ctrl.currentContent().contains(QStringLiteral("\n\n---\n\n")));

        // Duplicate note
        const int beforeCount = ctrl.notes().size();
        ctrl.duplicateCurrentNote();
        QCOMPARE(ctrl.notes().size(), beforeCount + 1);
        QVERIFY(ctrl.currentContent().contains(QStringLiteral("macOS 27 Golden Gate Notes")));
        QCOMPARE(ctrl.statusMessage(), QStringLiteral("Nota duplicada"));

        // Export to Markdown
        const QString mdExportPath = QDir(exportDir.path()).filePath(QStringLiteral("exported.md"));
        QVERIFY(ctrl.exportToMarkdown(mdExportPath));
        QVERIFY(QFile::exists(mdExportPath));

        // Import Markdown file
        QVERIFY(ctrl.importMarkdownFile(mdExportPath));
        QCOMPARE(ctrl.notes().size(), beforeCount + 2);

        // Export to PDF
        const QString pdfExportPath = QDir(exportDir.path()).filePath(QStringLiteral("exported.pdf"));
        QVERIFY(ctrl.exportToPdf(pdfExportPath));
        QVERIFY(QFile::exists(pdfExportPath));
        QVERIFY(QFileInfo(pdfExportPath).size() > 0);
    }

    void testHotCornerState()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        QuickNoteController ctrl(tempDir.path());

        QVERIFY(ctrl.isHotCornerEnabled());
        QSignalSpy spy(&ctrl, &QuickNoteController::hotCornerEnabledChanged);

        ctrl.setHotCornerEnabled(false);
        QCOMPARE(spy.count(), 1);
        QVERIFY(!ctrl.isHotCornerEnabled());

        ctrl.setHotCornerEnabled(true);
        QCOMPARE(spy.count(), 2);
        QVERIFY(ctrl.isHotCornerEnabled());
    }
};

QTEST_MAIN(TestKQuickNote)
#include "test_kquicknote.moc"
