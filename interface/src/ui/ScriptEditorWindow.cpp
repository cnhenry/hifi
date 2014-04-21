//
//  ScriptEditorWindow.cpp
//  interface/src/ui
//
//  Created by Thijs Wenker on 4/14/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ui_scriptEditorWindow.h"
#include "ScriptEditorWindow.h"
#include "ScriptEditorWidget.h"

#include <QGridLayout>
#include <QFrame>
#include <QLayoutItem>
#include <QMainWindow>
#include <QMessageBox>
#include <QPalette>
#include <QScrollBar>
#include <QShortcut>
#include <QSizePolicy>
#include <QTimer>
#include <QWidget>

#include "Application.h"
#include "FlowLayout.h"

ScriptEditorWindow::ScriptEditorWindow() :
    _ScriptEditorWindowUI(new Ui::ScriptEditorWindow),
    _loadMenu(new QMenu),
    _saveMenu(new QMenu)
{
    _ScriptEditorWindowUI->setupUi(this);
    show();
    addScriptEditorWidget("New script");
    connect(_loadMenu, SIGNAL(aboutToShow()), this, SLOT(loadMenuAboutToShow()));
    _ScriptEditorWindowUI->loadButton->setMenu(_loadMenu);

    _saveMenu->addAction("Save as..", this, SLOT(saveScriptAsClicked()), Qt::CTRL | Qt::SHIFT | Qt::Key_S);

    _ScriptEditorWindowUI->saveButton->setMenu(_saveMenu);

    connect(new QShortcut(QKeySequence("Ctrl+N"), this), SIGNAL(activated()), this, SLOT(newScriptClicked()));
    connect(new QShortcut(QKeySequence("Ctrl+S"), this), SIGNAL(activated()), this, SLOT(saveScriptClicked()));
    connect(new QShortcut(QKeySequence("Ctrl+O"), this), SIGNAL(activated()), this, SLOT(loadScriptClicked()));
    connect(new QShortcut(QKeySequence("F5"), this), SIGNAL(activated()), this, SLOT(toggleRunScriptClicked()));
}

ScriptEditorWindow::~ScriptEditorWindow() {
    delete _ScriptEditorWindowUI;
}

void ScriptEditorWindow::setRunningState(bool run) {
    if (_ScriptEditorWindowUI->tabWidget->currentIndex() != -1) {
        static_cast<ScriptEditorWidget*>(_ScriptEditorWindowUI->tabWidget->currentWidget())->setRunning(run);
    }
    this->updateButtons();
}

void ScriptEditorWindow::updateButtons() {
    _ScriptEditorWindowUI->toggleRunButton->setEnabled(_ScriptEditorWindowUI->tabWidget->currentIndex() != -1);
    _ScriptEditorWindowUI->toggleRunButton->setIcon(_ScriptEditorWindowUI->tabWidget->currentIndex() != -1 &&
        static_cast<ScriptEditorWidget*>(_ScriptEditorWindowUI->tabWidget->currentWidget())->isRunning() ?
        QIcon("../resources/icons/stop-script.svg") : QIcon("../resources/icons/start-script.svg"));
}

void ScriptEditorWindow::loadScriptMenu(const QString& scriptName) {
    addScriptEditorWidget("loading...")->loadFile(scriptName);
    updateButtons();
}

void ScriptEditorWindow::loadScriptClicked() {
    QString scriptName = QFileDialog::getOpenFileName(this, tr("Interface"), QString(), tr("Javascript (*.js)"));
    if (!scriptName.isEmpty()) {
        addScriptEditorWidget("loading...")->loadFile(scriptName);
        updateButtons();
    }
}

void ScriptEditorWindow::loadMenuAboutToShow() {
    _loadMenu->clear();
    QStringList runningScripts = Application::getInstance()->getRunningScripts();
    if (runningScripts.count() > 0) {
        QSignalMapper* signalMapper = new QSignalMapper(this);
        foreach (const QString& runningScript, runningScripts) {
            QAction* runningScriptAction = new QAction(runningScript, _loadMenu);
            connect(runningScriptAction, SIGNAL(triggered()), signalMapper, SLOT(map()));
            signalMapper->setMapping(runningScriptAction, runningScript);
            _loadMenu->addAction(runningScriptAction);
        }
        connect(signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(loadScriptMenu(const QString&)));
    } else {
        QAction* naAction = new QAction("(no running scripts)", _loadMenu);
        naAction->setDisabled(true);
        _loadMenu->addAction(naAction);
    }
}

void ScriptEditorWindow::newScriptClicked() {
    addScriptEditorWidget(QString("New script"));
}

void ScriptEditorWindow::toggleRunScriptClicked() {
    this->setRunningState(!(_ScriptEditorWindowUI->tabWidget->currentIndex() !=-1
        && static_cast<ScriptEditorWidget*>(_ScriptEditorWindowUI->tabWidget->currentWidget())->isRunning()));
}

void ScriptEditorWindow::saveScriptClicked() {
    if (_ScriptEditorWindowUI->tabWidget->currentIndex() != -1) {
        ScriptEditorWidget* currentScriptWidget = static_cast<ScriptEditorWidget*>(_ScriptEditorWindowUI->tabWidget
                                                                                   ->currentWidget());
        currentScriptWidget->save();
    }
}

void ScriptEditorWindow::saveScriptAsClicked() {
    if (_ScriptEditorWindowUI->tabWidget->currentIndex() != -1) {
        ScriptEditorWidget* currentScriptWidget = static_cast<ScriptEditorWidget*>(_ScriptEditorWindowUI->tabWidget
                                                                                   ->currentWidget());
        currentScriptWidget->saveAs();
    }
}

ScriptEditorWidget* ScriptEditorWindow::addScriptEditorWidget(QString title) {
    ScriptEditorWidget* newScriptEditorWidget = new ScriptEditorWidget();
    connect(newScriptEditorWidget, SIGNAL(scriptnameChanged()), this, SLOT(updateScriptNameOrStatus()));
    connect(newScriptEditorWidget, SIGNAL(scriptModified()), this, SLOT(updateScriptNameOrStatus()));
    connect(newScriptEditorWidget, SIGNAL(runningStateChanged()), this, SLOT(updateButtons()));
    _ScriptEditorWindowUI->tabWidget->addTab(newScriptEditorWidget, title);
    _ScriptEditorWindowUI->tabWidget->setCurrentWidget(newScriptEditorWidget);
    newScriptEditorWidget->setUpdatesEnabled(true);
    newScriptEditorWidget->adjustSize();
    return newScriptEditorWidget;
}

void ScriptEditorWindow::tabSwitched(int tabIndex) {
    this->updateButtons();
    if (_ScriptEditorWindowUI->tabWidget->currentIndex() != -1) {
        ScriptEditorWidget* currentScriptWidget = static_cast<ScriptEditorWidget*>(_ScriptEditorWindowUI->tabWidget
                                                                                   ->currentWidget());
        QString modifiedStar = (currentScriptWidget->isModified() ? "*" : "");
        if (currentScriptWidget->getScriptName().length() > 0) {
            this->setWindowTitle("Script Editor [" + currentScriptWidget->getScriptName() + modifiedStar + "]");
        } else {
            this->setWindowTitle("Script Editor [New script" + modifiedStar + "]");
        }
    } else {
        this->setWindowTitle("Script Editor");
    }
}

void ScriptEditorWindow::tabCloseRequested(int tabIndex) {
    ScriptEditorWidget* closingScriptWidget = static_cast<ScriptEditorWidget*>(_ScriptEditorWindowUI->tabWidget
                                                                               ->widget(tabIndex));
    if(closingScriptWidget->questionSave()) {
        _ScriptEditorWindowUI->tabWidget->removeTab(tabIndex);
    }
}

void ScriptEditorWindow::closeEvent(QCloseEvent *event) {
    bool unsaved_docs_warning = false;
    for (int i = 0; i < _ScriptEditorWindowUI->tabWidget->count(); i++){
        if(static_cast<ScriptEditorWidget*>(_ScriptEditorWindowUI->tabWidget->widget(i))->isModified()){
            unsaved_docs_warning = true;
            break;
        }
    }

    if (!unsaved_docs_warning || QMessageBox::warning(this, tr("Interface"),
        tr("There are some unsaved scripts, are you sure you want to close the editor? Changes will be lost!"),
        QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Discard) {
        event->accept();
    } else {
        event->ignore();
    }
}

void ScriptEditorWindow::updateScriptNameOrStatus() {
    ScriptEditorWidget* source = static_cast<ScriptEditorWidget*>(QObject::sender());
    QString modifiedStar = (source->isModified()? "*" : "");
    if (source->getScriptName().length() > 0) {
        for (int i = 0; i < _ScriptEditorWindowUI->tabWidget->count(); i++){
            if (_ScriptEditorWindowUI->tabWidget->widget(i) == source) {
                _ScriptEditorWindowUI->tabWidget->setTabText(i, modifiedStar + QFileInfo(source->getScriptName()).fileName());
                _ScriptEditorWindowUI->tabWidget->setTabToolTip(i, source->getScriptName());
            }
        }
    }

    if (_ScriptEditorWindowUI->tabWidget->currentWidget() == source) {
        if (source->getScriptName().length() > 0) {
            this->setWindowTitle("Script Editor [" + source->getScriptName() + modifiedStar + "]");
        } else {
            this->setWindowTitle("Script Editor [New script" + modifiedStar + "]");
        }
    }
}