/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "qbsprojectmanager.h"

#include "defaultpropertyprovider.h"
#include "qbslogsink.h"
#include "qbsproject.h"
#include "qbsprojectmanagerconstants.h"
#include "qbsprojectmanagerplugin.h"

#include <coreplugin/icore.h>
#include <coreplugin/messagemanager.h>
#include <extensionsystem/pluginmanager.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/projectexplorer.h>
#include <qmljstools/qmljstoolsconstants.h>
#include <qtsupport/baseqtversion.h>
#include <qtsupport/qtkitinformation.h>

#include <QVariantMap>

#include <qbs.h>
#include <qtprofilesetup.h>

const QChar sep = QLatin1Char('.');

static QString qtcProfileGroup() { return QLatin1String("preferences.qtcreator.kit"); }
static QString qtcProfilePrefix() { return qtcProfileGroup() + sep; }

namespace QbsProjectManager {
namespace Internal {

qbs::Settings *QbsManager::m_settings = 0;
Internal::QbsLogSink *QbsManager::m_logSink = 0;
QbsManager *QbsManager::m_instance = 0;

QbsManager::QbsManager() :
    m_defaultPropertyProvider(new DefaultPropertyProvider)
{
    m_settings = new qbs::Settings(Core::ICore::userResourcePath());
    m_instance = this;

    setObjectName(QLatin1String("QbsProjectManager"));
    connect(ProjectExplorer::KitManager::instance(), &ProjectExplorer::KitManager::kitsLoaded, this,
            [this]() { m_kitsToBeSetupForQbs = ProjectExplorer::KitManager::kits(); } );
    connect(ProjectExplorer::KitManager::instance(), &ProjectExplorer::KitManager::kitAdded, this,
            &QbsManager::addProfileFromKit);
    connect(ProjectExplorer::KitManager::instance(), &ProjectExplorer::KitManager::kitUpdated, this,
            &QbsManager::handleKitUpdate);
    connect(ProjectExplorer::KitManager::instance(), &ProjectExplorer::KitManager::kitRemoved, this,
            &QbsManager::handleKitRemoval);

    m_logSink = new QbsLogSink(this);
    int level = qbs::LoggerWarning;
    const QString levelEnv = QString::fromLocal8Bit(qgetenv("QBS_LOG_LEVEL"));
    if (!levelEnv.isEmpty()) {
        bool ok = false;
        int tmp = levelEnv.toInt(&ok);
        if (ok) {
            if (tmp < static_cast<int>(qbs::LoggerMinLevel))
                tmp = static_cast<int>(qbs::LoggerMinLevel);
            if (tmp > static_cast<int>(qbs::LoggerMaxLevel))
                tmp = static_cast<int>(qbs::LoggerMaxLevel);
            level = tmp;
        }
    }
    m_logSink->setLogLevel(static_cast<qbs::LoggerLevel>(level));
}

QbsManager::~QbsManager()
{
    delete m_defaultPropertyProvider;
    delete m_settings;
    m_instance = 0;
}

QString QbsManager::mimeType() const
{
    return QLatin1String(QmlJSTools::Constants::QBS_MIMETYPE);
}

ProjectExplorer::Project *QbsManager::openProject(const QString &fileName, QString *errorString)
{
    if (!QFileInfo(fileName).isFile()) {
        if (errorString)
            *errorString = tr("Failed opening project \"%1\": Project is not a file.")
                .arg(fileName);
        return 0;
    }

    return new QbsProject(this, fileName);
}

QString QbsManager::profileForKit(const ProjectExplorer::Kit *k)
{
    if (!k)
        return QString();
    updateProfileIfNecessary(k);
    return m_settings->value(qtcProfilePrefix() + k->id().toString()).toString();
}

void QbsManager::setProfileForKit(const QString &name, const ProjectExplorer::Kit *k)
{
    m_settings->setValue(qtcProfilePrefix() + k->id().toString(), name);
}

void QbsManager::updateProfileIfNecessary(const ProjectExplorer::Kit *kit)
{
    // kit in list <=> profile update is necessary
    // Note that the const_cast is safe, as we do not call any non-const methods on the object.
    if (m_kitsToBeSetupForQbs.removeOne(const_cast<ProjectExplorer::Kit *>(kit)))
        addProfileFromKit(kit);
}

void QbsManager::addProfile(const QString &name, const QVariantMap &data)
{
    qbs::Profile profile(name, settings());
    const QVariantMap::ConstIterator cend = data.constEnd();
    for (QVariantMap::ConstIterator it = data.constBegin(); it != cend; ++it)
        profile.setValue(it.key(), it.value());
}

void QbsManager::addQtProfileFromKit(const QString &profileName, const ProjectExplorer::Kit *k)
{
    const QtSupport::BaseQtVersion * const qt = QtSupport::QtKitInformation::qtVersion(k);
    if (!qt)
        return;

    qbs::QtEnvironment qtEnv;
    const QList<ProjectExplorer::Abi> abi = qt->qtAbis();
    if (!abi.empty()) {
        qtEnv.architecture = ProjectExplorer::Abi::toString(abi.first().architecture());
        if (abi.first().wordWidth() == 64)
            qtEnv.architecture.append(QLatin1String("_64"));
    }
    qtEnv.binaryPath = qt->binPath().toString();
    qtEnv.documentationPath = qt->docsPath().toString();
    qtEnv.includePath = qt->headerPath().toString();
    qtEnv.libraryPath = qt->libraryPath().toString();
    qtEnv.pluginPath = qt->pluginPath().toString();
    qtEnv.mkspecBasePath = qt->mkspecsPath().toString();
    qtEnv.mkspecName = qt->mkspec().toString();
    qtEnv.mkspecPath = qt->mkspecPath().toString();
    qtEnv.qtNameSpace = qt->qtNamespace();
    qtEnv.qtLibInfix = qt->qtLibInfix();
    qtEnv.qtVersion = qt->qtVersionString();
    qtEnv.qtMajorVersion = qt->qtVersion().majorVersion;
    qtEnv.qtMinorVersion = qt->qtVersion().minorVersion;
    qtEnv.qtPatchVersion = qt->qtVersion().patchVersion;
    qtEnv.frameworkBuild = qt->isFrameworkBuild();
    qtEnv.configItems = qt->configValues();
    qtEnv.qtConfigItems = qt->qtConfigValues();
    foreach (const QString &buildVariant,
            QStringList() << QLatin1String("debug") << QLatin1String("release")) {
        if (qtEnv.qtConfigItems.contains(buildVariant))
            qtEnv.buildVariant << buildVariant;
    }
    const qbs::ErrorInfo errorInfo = qbs::setupQtProfile(profileName, settings(), qtEnv);
    if (errorInfo.hasError()) {
        Core::MessageManager::write(tr("Failed to set up kit for Qbs: %1")
                .arg(errorInfo.toString()), Core::MessageManager::ModeSwitch);
    }
}

void QbsManager::addProfileFromKit(const ProjectExplorer::Kit *k)
{
    const QString name = QString::fromLatin1("qtc_%1_%2").arg(k->fileSystemFriendlyName().left(8),
            QString::number(k->id().uniqueIdentifier(), 16));
    qbs::Profile(name, settings()).removeProfile();
    setProfileForKit(name, k);
    addQtProfileFromKit(name, k);

    // set up properties:
    QVariantMap data = m_defaultPropertyProvider->properties(k, QVariantMap());
    QList<PropertyProvider *> providerList = ExtensionSystem::PluginManager::getObjects<PropertyProvider>();
    foreach (PropertyProvider *provider, providerList) {
        if (provider->canHandle(k))
            data = provider->properties(k, data);
    }

    addProfile(name, data);
}

void QbsManager::handleKitUpdate(ProjectExplorer::Kit *kit)
{
    m_kitsToBeSetupForQbs.removeOne(kit);
    addProfileFromKit(kit);
}

void QbsManager::handleKitRemoval(ProjectExplorer::Kit *kit)
{
    m_kitsToBeSetupForQbs.removeOne(kit);
    const QString key = qtcProfilePrefix() + kit->id().toString();
    const QString profileName = m_settings->value(key).toString();
    m_settings->remove(key);
    qbs::Profile(profileName, m_settings).removeProfile();
}

} // namespace Internal
} // namespace QbsProjectManager
