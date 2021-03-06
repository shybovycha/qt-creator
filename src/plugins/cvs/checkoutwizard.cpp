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

#include "checkoutwizard.h"
#include "checkoutwizardpage.h"
#include "cvsclient.h"
#include "cvsplugin.h"

#include <coreplugin/iversioncontrol.h>
#include <vcsbase/vcscommand.h>
#include <vcsbase/vcsbaseconstants.h>
#include <vcsbase/wizard/vcsconfigurationpage.h>
#include <utils/qtcassert.h>

using namespace VcsBase;

namespace Cvs {
namespace Internal {

// --------------------------------------------------------------------
// CheckoutWizard:
// --------------------------------------------------------------------

CheckoutWizard::CheckoutWizard(const Utils::FileName &path, QWidget *parent) :
    BaseCheckoutWizard(path, parent)
{
    const Core::IVersionControl *vc = CvsPlugin::instance()->versionControl();
    if (!vc->isConfigured()) {
        auto configPage = new VcsConfigurationPage;
        configPage->setVersionControl(vc);
        addPage(configPage);
    }
    auto cwp = new CheckoutWizardPage;
    cwp->setPath(path.toString());
    addPage(cwp);
}

VcsCommand *CheckoutWizard::createCommand(Utils::FileName *checkoutDir)
{
    // Collect parameters for the checkout command.
    // CVS does not allow for checking out into a different directory.
    const CheckoutWizardPage *cwp = 0;
    foreach (int pageId, pageIds()) {
        if ((cwp = qobject_cast<const CheckoutWizardPage *>(page(pageId))))
            break;
    }

    QTC_ASSERT(cwp, return 0);
    const CvsSettings settings = CvsPlugin::instance()->client()->settings();
    const Utils::FileName binary = settings.binaryPath();
    QStringList args;

    const QString repository = cwp->repository();
    args << QLatin1String("checkout") << repository;
    const QString workingDirectory = cwp->path();
    *checkoutDir = Utils::FileName::fromString(workingDirectory + QLatin1Char('/') + repository);

    auto command = new VcsCommand(binary, workingDirectory,
                                  QProcessEnvironment::systemEnvironment());
    command->addJob(settings.addOptions(args), -1);
    return command;
}

} // namespace Internal
} // namespace Cvs
