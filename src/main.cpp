/*
    SPDX-FileCopyrightText: 2026 Cesar
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "quicknotecontroller.h"

#include <KGlobalAccel>
#include <KLocalizedQmlContext>
#include <KLocalizedString>

#include <QAction>
#include <QApplication>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QIcon>
#include <QKeySequence>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("kquicknote");
    app.setApplicationName(QStringLiteral("kquicknote"));
    app.setApplicationDisplayName(QStringLiteral("KQuickNote"));
    app.setOrganizationDomain(QStringLiteral("cesar.org"));
    app.setQuitOnLastWindowClosed(false);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("accessories-text-editor"), QIcon::fromTheme(QStringLiteral("document-edit"))));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("KQuickNote — Nota Rápida con Esquina Activa para KDE Plasma 6 Wayland"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption daemonOpt(QStringList{QStringLiteral("d"), QStringLiteral("daemon")},
                                 QStringLiteral("Iniciar en segundo plano con la esquina activa habilitada"));
    parser.addOption(daemonOpt);

    QCommandLineOption toggleOpt(QStringList{QStringLiteral("t"), QStringLiteral("toggle")},
                                 QStringLiteral("Mostrar u ocultar la ventana de Nota Rápida"));
    parser.addOption(toggleOpt);

    QCommandLineOption newOpt(QStringList{QStringLiteral("n"), QStringLiteral("new-note")},
                              QStringLiteral("Crear una nueva nota capturando la selección actual"));
    parser.addOption(newOpt);

    parser.process(app);

    const QString serviceName = QStringLiteral("org.cesar.kquicknote");
    const QString objectPath = QStringLiteral("/QuickNote");
    const QString interfaceName = QStringLiteral("org.cesar.kquicknote");

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (bus.isConnected() && !bus.registerService(serviceName)) {
        // Otra instancia ya se está ejecutando: reenviar comando por D-Bus y salir
        QDBusInterface iface(serviceName, objectPath, interfaceName, bus);
        if (iface.isValid()) {
            if (parser.isSet(newOpt)) {
                iface.call(QStringLiteral("NewNoteWithSelection"));
            } else {
                iface.call(QStringLiteral("Toggle"));
            }
            return 0;
        }
    }

    QuickNoteController controller;

    if (bus.isConnected()) {
        bus.registerObject(objectPath, &controller, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportAllProperties);
    }

    // Atajos globales vía KF6 GlobalAccel
    auto *toggleAction = new QAction(&app);
    toggleAction->setObjectName(QStringLiteral("toggle_quick_note"));
    toggleAction->setText(QStringLiteral("Mostrar/Ocultar Nota Rápida"));
    KGlobalAccel::setGlobalShortcut(toggleAction, QKeySequence(Qt::META | Qt::ALT | Qt::Key_N));
    QObject::connect(toggleAction, &QAction::triggered, &controller, &QuickNoteController::toggleVisible);

    auto *captureAction = new QAction(&app);
    captureAction->setObjectName(QStringLiteral("capture_selection_quick_note"));
    captureAction->setText(QStringLiteral("Nueva Nota Rápida con Selección"));
    KGlobalAccel::setGlobalShortcut(captureAction, QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_N));
    QObject::connect(captureAction, &QAction::triggered, &controller, &QuickNoteController::NewNoteWithSelection);

    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);
    engine.rootContext()->setContextProperty(QStringLiteral("QuickNote"), &controller);

    // Cargar ventana principal de edición
    engine.load(QUrl(QStringLiteral("qrc:/org/cesar/kquicknote/qml/main.qml")));

    // Cargar peeker de la esquina activa Wayland (LayerShell)
    engine.load(QUrl(QStringLiteral("qrc:/org/cesar/kquicknote/qml/CornerPeeker.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    for (auto *obj : engine.rootObjects()) {
        if (auto *win = qobject_cast<QQuickWindow *>(obj)) {
            if (win->objectName() == QStringLiteral("quickNoteMainWindow")) {
                QObject::connect(&controller, &QuickNoteController::visibleChanged, win, [win, &controller]() {
                    if (controller.isVisible()) {
                        win->show();
                        win->raise();
                        win->requestActivate();
                    } else {
                        win->hide();
                    }
                });

                if (parser.isSet(daemonOpt)) {
                    controller.setVisible(false);
                    win->hide();
                } else {
                    controller.setVisible(true);
                    win->show();
                    win->raise();
                    win->requestActivate();
                }
            }
        }
    }

    if (parser.isSet(newOpt)) {
        controller.NewNoteWithSelection();
    }

    return app.exec();
}
