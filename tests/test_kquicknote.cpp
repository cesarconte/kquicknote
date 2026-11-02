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
        engine.rootContext()->setContextProperty(QStringLiteral("QuickNote"), &ctrl);

        QQmlComponent comp(&engine, QUrl(QStringLiteral("qrc:/org/cesar/kquicknote/qml/main.qml")));
        QVERIFY2(!comp.isError(), qPrintable(comp.errorString()));
    }
};

QTEST_MAIN(TestKQuickNote)
#include "test_kquicknote.moc"
