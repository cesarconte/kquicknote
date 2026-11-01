/*
    SPDX-FileCopyrightText: 2026 Cesar
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "quicknotecontroller.h"

#include <KGlobalAccel>
#include <LayerShellQt/Window>

#include <QAction>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QKeySequence>
#include <QMargins>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kquicknote"));
    app.setApplicationDisplayName(QStringLiteral("KQuickNote"));
    app.setOrganizationDomain(QStringLiteral("cesar.org"));
    app.setQuitOnLastWindowClosed(false);

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (bus.isConnected()) {
        if (!bus.registerService(QStringLiteral("org.cesar.kquicknote"))) {
            // Existing instance running: toggle it via D-Bus and exit
            QDBusInterface iface(
                QStringLiteral("org.cesar.kquicknote"),
                QStringLiteral("/QuickNote"),
                QStringLiteral("org.cesar.kquicknote"),
                bus);
            if (iface.isValid()) {
                iface.call(QStringLiteral("Toggle"));
                return 0;
            }
        }
    }

    QuickNoteController controller;

    if (bus.isConnected()) {
        bus.registerObject(
            QStringLiteral("/QuickNote"),
            &controller,
            QDBusConnection::ExportScriptableSlots);
    }

    // Register global shortcut Meta+Alt+N via KF6 GlobalAccel
    auto *toggleAction = new QAction(&app);
    toggleAction->setObjectName(QStringLiteral("toggle_quick_note"));
    toggleAction->setText(QStringLiteral("Mostrar/Ocultar Nota Rápida en Esquina"));
    KGlobalAccel::setGlobalShortcut(
        toggleAction,
        QKeySequence(Qt::META | Qt::ALT | Qt::Key_N));
    QObject::connect(toggleAction, &QAction::triggered, &controller, &QuickNoteController::toggleVisible);

    auto *captureAction = new QAction(&app);
    captureAction->setObjectName(QStringLiteral("capture_selection_quick_note"));
    captureAction->setText(QStringLiteral("Nueva Nota Rápida con Selección"));
    KGlobalAccel::setGlobalShortcut(
        captureAction,
        QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_N));
    QObject::connect(captureAction, &QAction::triggered, &controller, &QuickNoteController::NewNoteWithSelection);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("QuickNote"), &controller);
    engine.load(QUrl(QStringLiteral("qrc:/org/cesar/kquicknote/qml/main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (auto *win = qobject_cast<QQuickWindow *>(engine.rootObjects().first())) {
        if (auto *layerWin = LayerShellQt::Window::get(win)) {
            layerWin->setLayer(LayerShellQt::Window::LayerOverlay);
            layerWin->setAnchors(LayerShellQt::Window::Anchors(
                LayerShellQt::Window::AnchorBottom | LayerShellQt::Window::AnchorRight));
            layerWin->setMargins(QMargins(0, 0, 24, 24));
            layerWin->setExclusiveZone(0);
            layerWin->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
            layerWin->setScope(QStringLiteral("kquicknote"));

            QObject::connect(&controller, &QuickNoteController::pinnedChanged, win, [layerWin, &controller]() {
                layerWin->setLayer(controller.isPinned()
                                       ? LayerShellQt::Window::LayerOverlay
                                       : LayerShellQt::Window::LayerTop);
            });
        }
    }

    return app.exec();
}
