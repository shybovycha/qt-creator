/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**************************************************************************/
#include "linuxdevicefactoryselectiondialog.h"
#include "ui_linuxdevicefactoryselectiondialog.h"

#include "linuxdeviceconfiguration.h"

#include <extensionsystem/pluginmanager.h>
#include <utils/qtcassert.h>

#include <QtGui/QPushButton>

namespace RemoteLinux {
namespace Internal {

LinuxDeviceFactorySelectionDialog::LinuxDeviceFactorySelectionDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::LinuxDeviceFactorySelectionDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Start Wizard"));

    const QList<ILinuxDeviceConfigurationFactory *> &factories
        = ExtensionSystem::PluginManager::instance()->getObjects<ILinuxDeviceConfigurationFactory>();
    foreach (const ILinuxDeviceConfigurationFactory * const factory, factories) {
        m_factories << factory;
        ui->listWidget->addItem(factory->displayName());
    }

    connect(ui->listWidget, SIGNAL(itemSelectionChanged()), SLOT(handleItemSelectionChanged()));
    handleItemSelectionChanged();
}

LinuxDeviceFactorySelectionDialog::~LinuxDeviceFactorySelectionDialog()
{
    delete ui;
}

void LinuxDeviceFactorySelectionDialog::handleItemSelectionChanged()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)
        ->setEnabled(!ui->listWidget->selectedItems().isEmpty());
}

const ILinuxDeviceConfigurationFactory *LinuxDeviceFactorySelectionDialog::selectedFactory() const
{
    return m_factories.at(ui->listWidget->row(ui->listWidget->selectedItems().first()));
}

} // namespace Internal
} // namespace RemoteLinux
