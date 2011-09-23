/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (info@qt.nokia.com)
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
** Nokia at info@qt.nokia.com.
**
**************************************************************************/

#include "codenavigator.h"
#include "liveunitsmanager.h"
#include "reuse.h"
#include "indexedsymbolinfo.h"
#include "indexer.h"

#include <QtCore/QtConcurrentRun>

using namespace Clang;
using namespace Internal;

namespace {

Unit parseUnit(const QString &fileName,
               const QStringList &compileOptions,
               unsigned managmentOptions)
{
    Unit unit(fileName);
    unit.setCompilationOptions(compileOptions);
    unit.setManagementOptions(managmentOptions);
    unit.parse();
    return unit;
}

} // Anonymous

CodeNavigator::CodeNavigator()
{}

CodeNavigator::~CodeNavigator()
{
    if (!m_unit.isNull())
        LiveUnitsManager::instance()->remove(m_unit.fileName());
}

void CodeNavigator::setup(const QString &fileName, const Indexer *indexer)
{
    m_indexer = indexer;
    m_unit = LiveUnitsManager::instance()->find(fileName);
    if (m_unit.isNull()) {
        QFuture<Unit> future = QtConcurrent::run(parseUnit,
                                                 fileName,
                                                 indexer->compilationOptions(fileName),
                                                 CXTranslationUnit_None);
        m_unitWatcher.setFuture(future);
        connect(&m_unitWatcher, SIGNAL(finished()), this, SLOT(unitReady()));
    }
}

void CodeNavigator::unitReady()
{
    const Unit &unit = m_unitWatcher.result();
    if (!unit.isNull()) {
        m_unit = unit;
        // Share this TU so it's available for anyone else while the navigator exists.
        LiveUnitsManager::instance()->insert(m_unit);
    }
}

SourceLocation CodeNavigator::findDefinition(unsigned line, unsigned column) const
{
    // @TODO: Cover includes, macros, etc...

    if (m_unit.isNull()) {
        m_unit.parse();
        if (m_unit.isNull())
            return SourceLocation();
    }

    const CXFile &file = m_unit.getFile();
    const CXSourceLocation &location = m_unit.getLocation(file, line, column);
    if (clang_equalLocations(location, clang_getNullLocation()))
        return SourceLocation();

    CXCursor cursor = m_unit.getCursor(location);
    if (clang_equalCursors(cursor, clang_getNullCursor()))
        return SourceLocation();

    CXCursor cursorDefinition = clang_getNullCursor();
    if (clang_isCursorDefinition(cursor))
        cursorDefinition = cursor;
    else
        cursorDefinition = clang_getCursorDefinition(cursor);

    if (!clang_equalCursors(cursorDefinition, clang_getNullCursor()))
        return Internal::getInstantiationLocation(clang_getCursorLocation(cursorDefinition));

    // Definition is not in the unit, use indexed data to look for it.
    CXCursorKind cursorKind = clang_getCursorKind(cursor);
    if (clang_isDeclaration(cursorKind)
            || clang_isReference(cursorKind)) {
        QList<IndexedSymbolInfo> indexedInfo;
        if (cursorKind == CXCursor_ClassDecl
                || cursorKind == CXCursor_StructDecl
                || cursorKind == CXCursor_UnionDecl) {
            indexedInfo = m_indexer->allClasses();
        } else if (cursorKind == CXCursor_FunctionDecl
                   || cursorKind == CXCursor_FunctionTemplate
                   || cursorKind == CXCursor_CXXMethod) {
            indexedInfo.append(m_indexer->allFunctions());
            indexedInfo.append(m_indexer->allMethods());
        } else if (cursorKind == CXCursor_Constructor) {
            indexedInfo = m_indexer->allConstructors();
        } else if (cursorKind == CXCursor_Destructor) {
            indexedInfo = m_indexer->allDestructors();
        }

        if (!indexedInfo.isEmpty()) {
            const QString &spelling = Internal::getQString(clang_getCursorSpelling(cursor));
            // @TODO: Take qualification into consideration.
            foreach (const IndexedSymbolInfo &info, indexedInfo) {
                if (info.m_name == spelling) {
                    return info.m_location;
                }
            }
        }
    }

    return SourceLocation();
}
