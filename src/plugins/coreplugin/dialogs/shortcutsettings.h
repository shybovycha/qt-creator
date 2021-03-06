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

#ifndef SHORTCUTSETTINGS_H
#define SHORTCUTSETTINGS_H

#include <coreplugin/actionmanager/commandmappings.h>

#include <QKeySequence>

QT_BEGIN_NAMESPACE
class QKeyEvent;
QT_END_NAMESPACE

namespace Core {

class Command;

namespace Internal {

class ActionManagerPrivate;
class MainWindow;

struct ShortcutItem
{
    Command *m_cmd;
    QKeySequence m_key;
    QTreeWidgetItem *m_item;
};


class ShortcutSettings : public CommandMappings
{
    Q_OBJECT

public:
    ShortcutSettings(QObject *parent = 0);

    QWidget *widget();
    void apply();
    void finish();

protected:
    bool eventFilter(QObject *o, QEvent *e);

private slots:
    void commandChanged(QTreeWidgetItem *current);
    void targetIdentifierChanged();
    void resetTargetIdentifier();
    void removeTargetIdentifier();
    void importAction();
    void exportAction();
    void defaultAction();
    void initialize();

private:
    void setKeySequence(const QKeySequence &key);
    void clear();

    void handleKeyEvent(QKeyEvent *e);
    int translateModifiers(Qt::KeyboardModifiers state, const QString &text);

    void markCollisions(ShortcutItem *);
    bool hasConflicts() const;

    QList<ShortcutItem *> m_scitems;
    int m_key[4], m_keyNum;

    bool m_initialized;
};

} // namespace Internal
} // namespace Core

#endif // SHORTCUTSETTINGS_H
