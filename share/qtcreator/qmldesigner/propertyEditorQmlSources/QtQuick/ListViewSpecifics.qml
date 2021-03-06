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

import QtQuick 2.1
import HelperWidgets 2.0
import QtQuick.Layouts 1.0

Column {
    anchors.left: parent.left
    anchors.right: parent.right

    FlickableSection {
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Section {
        anchors.left: parent.left
        anchors.right: parent.right
        caption: qsTr("List View")

        SectionLayout {

            Label {
                text: qsTr("Cache")
                tooltip: qsTr("Cache buffer")
            }

            SectionLayout {
                SpinBox {
                    backendValue: backendValues.cacheBuffer
                    minimumValue: 0;
                    maximumValue: 1000;
                    decimals: 0
                }

                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Navigation wraps")
                tooltip: qsTr("Determines whether the grid wraps key navigation.")
            }

            SectionLayout {
                CheckBox {
                    backendValue: backendValues.keyNavigationWraps
                }

                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Orientation")
                tooltip: qsTr("Orientation of the list.")
            }

            SecondColumnLayout {
                ComboBox {
                    model: ["Horizontal", "Vertical"]
                    backendValue: backendValues.orientation
                    Layout.fillWidth: true
                    scope: "ListView"
                }

            }

            Label {
                text: qsTr("Layout Direction")
            }

            SecondColumnLayout {
                ComboBox {
                    model: ["LeftToRight", "RightToLeft"]
                    backendValue: backendValues.layoutDirection
                    scope: "Qt"
                }

                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Snap mode")
                tooltip: qsTr("Determines how the view scrolling will settle following a drag or flick.")
            }

            SecondColumnLayout {
                ComboBox {
                    model: ["NoSnap", "SnapToItem", "SnapOneItem"]
                    backendValue: backendValues.snapMode
                    scope: "ListView"
                }

                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Spacing")
                tooltip: qsTr("Spacing between items.")
            }

            SectionLayout {
                SpinBox {
                    backendValue: backendValues.spacing
                    minimumValue: 0;
                    maximumValue: 1000;
                    decimals: 0
                }

                ExpandingSpacer {

                }
            }

        }
    }

    Section {
        anchors.left: parent.left
        anchors.right: parent.right
        caption: qsTr("List View Highlight")

        SectionLayout {

            Label {
                text: qsTr("Range")
                tooltip: qsTr("Highlight range")
            }

            SecondColumnLayout {
                ComboBox {
                    model: ["NoHighlightRange", "ApplyRange", "StrictlyEnforceRange"]
                    backendValue: backendValues.highlightRangeMode
                    Layout.fillWidth: true
                    scope: "ListView"
                }

                ExpandingSpacer {

                }
            }


            Label {
                text: qsTr("Move duration")
                tooltip: qsTr("Move animation duration of the highlight delegate.")
            }

            SectionLayout {
                SpinBox {
                    backendValue: backendValues.highlightMoveDuration
                    minimumValue: 0;
                    maximumValue: 1000;
                    decimals: 0
                }

                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Move speed")
                tooltip: qsTr("Move animation speed of the highlight delegate.")
            }

            SectionLayout {
                SpinBox {
                    backendValue: backendValues.highlightMoveSpeed
                    minimumValue: 0;
                    maximumValue: 1000;
                    decimals: 0
                }

                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Resize duration")
                tooltip: qsTr("Resize animation duration of the highlight delegate.")
            }

            SectionLayout {
                SpinBox {
                    backendValue: backendValues.highlightResizeDuration
                    minimumValue: 0;
                    maximumValue: 1000;
                    decimals: 0
                }

                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Preferred begin")
                tooltip: qsTr("Preferred highlight begin - must be smaller than Preferred end.")
            }

            SectionLayout {
                SpinBox {
                    backendValue: backendValues.preferredHighlightBegin
                    minimumValue: 0;
                    maximumValue: 1000;
                    decimals: 0
                }

                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Preferred end")
                tooltip: qsTr("Preferred highlight end - must be larger than Preferred begin.")
            }

            SectionLayout {
                SpinBox {
                    backendValue: backendValues.preferredHighlightEnd
                    minimumValue: 0;
                    maximumValue: 1000;
                    decimals: 0
                }

                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Follows current")
                tooltip: qsTr("Determines whether the highlight is managed by the view.")
            }

            SectionLayout {
                CheckBox {
                    backendValue: backendValues.highlightFollowsCurrentItem
                }

                ExpandingSpacer {

                }
            }

        }
    }
}
