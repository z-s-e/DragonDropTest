/* Copyright 2014, 2015 Zeno Sebastian Endemann <zeno.endemann@googlemail.com>
 *
 * This file is part of DragonDropTest.
 *
 * DragonDropTest is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DragonDropTest is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DragonDropTest.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "droparea.h"

#include <QApplication>
#include <QClipboard>
#include <QCryptographicHash>
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMouseEvent>
#include <QTimer>

static const int s_FromClipboardAction = -1;

DropArea::DropArea(QWidget *parent)
    : QLabel(parent)
{
    setAcceptDrops(true);
    setAutoFillBackground(true);
    setToolTip(tr("Use shift, ctrl or shift+ctrl to select link, copy or move action\nRight click: get clipboard data"));
    clear();
}

void DropArea::setDropActionString(const QString &dropAction)
{
    setText(tr("<drop content>\n(%1)").arg(dropAction));
}

void DropArea::dragEnterEvent(QDragEnterEvent *event)
{
    setBackgroundRole(QPalette::Highlight);

    acceptDropEvent(event);
}

void DropArea::dragMoveEvent(QDragMoveEvent *event)
{
    acceptDropEvent(event);
}

void DropArea::dragLeaveEvent(QDragLeaveEvent *)
{
    clear();
}

void DropArea::dropEvent(QDropEvent *event)
{
    DnDAction act;
    act.defaultAction = event->proposedAction();
    act.supportedActions = event->possibleActions();

    acceptDropEvent(event);

    const QMimeData *mimeData = event->mimeData();
    const QStringList formats = mimeData->formats();
    act.data.reserve(formats.size());

    qDebug() << "Drop (" << event->dropAction() << "):";
    for( const auto &f : formats) {
        qDebug() << " format: " << f;
        act.data.append(DnDAction::DataEntry(f, mimeData->data(f)));
    }

    emit dataDropped(event->dropAction(), act);

    clear();
}

void DropArea::mousePressEvent(QMouseEvent *ev)
{
    if( ev->button() == Qt::RightButton ) {
        setBackgroundRole(QPalette::Highlight);
        QTimer::singleShot(1, this, SLOT(fromClipboard()));
    }
}

void DropArea::mouseDoubleClickEvent(QMouseEvent *)
{
}

void DropArea::mouseMoveEvent(QMouseEvent *)
{
}

void DropArea::mouseReleaseEvent(QMouseEvent *)
{
}

void DropArea::fromClipboard()
{
    DnDAction act;
    act.defaultAction = Qt::CopyAction;
    act.supportedActions = Qt::CopyAction;

    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    const QStringList formats = mimeData->formats();
    act.data.reserve(formats.size());

    for( const auto &f : formats)
        act.data.append(DnDAction::DataEntry(f, mimeData->data(f)));

    emit dataDropped(s_FromClipboardAction, act);

    clear();
}

void DropArea::acceptDropEvent(QDropEvent *event)
{
    const auto override = overrideAction(event->keyboardModifiers());

    switch (override) {
    case Qt::CopyAction:
    case Qt::MoveAction:
    case Qt::LinkAction:
        if( event->possibleActions() & override ) {
            event->setDropAction(override);
            event->accept();
        } else {
            event->ignore();
        }
        break;
    default:
        event->acceptProposedAction();
    }
}

Qt::DropAction DropArea::overrideAction(Qt::KeyboardModifiers modifiers)
{
    if( modifiers & Qt::ShiftModifier ) {
        if( modifiers & Qt::ControlModifier ) {
            return Qt::MoveAction;
        } else {
            return Qt::LinkAction;
        }
    } else if( modifiers & Qt::ControlModifier ) {
        return Qt::CopyAction;
    } else {
        return Qt::ActionMask;
    }
}

void DropArea::clear()
{
    setBackgroundRole(QPalette::Dark);
}



DropDataModel::DropDataModel(QObject *parent)
    : QAbstractItemModel(parent)
    , mDropAction(-2)
{
}

int DropDataModel::columnCount(const QModelIndex &) const
{
    return 4;
}

QVariant DropDataModel::data(const QModelIndex &index, int role) const
{
    if( ! index.isValid() )
        return {};

    if( role != Qt::DisplayRole )
        return QVariant();

    switch( index.column() ) {
    case 0:
        return dropMimeType(index.row());
    case 1:
        return dropFileExtension(index.row());
    case 2:
        return mDrop.data.at(index.row()).fileSize();
    case 3:
        return mDrop.data.at(index.row()).sha1();
    }

    return {};
}

int DropDataModel::rowCount(const QModelIndex &parent) const
{
    if( parent.isValid() )
        return 0;

    return mDrop.data.size();
}

QVariant DropDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return QVariant();

    switch (section) {
    case 0:
        return tr("MIME");
    case 1:
        return tr("File extension");
    case 2:
        return tr("Bytes");
    case 3:
        return tr("SHA-1");
    }

    return {};
}

QString DropDataModel::dropMimeType(int row) const
{
    return mDrop.data.at(row).mime();
}

QString DropDataModel::dropFileExtension(int row) const
{
    return mDrop.data.at(row).fileExtension();
}

QByteArray DropDataModel::dropData(int row) const
{
    return mDrop.data.at(row).bytes();
}

QString DropDataModel::dropActioString() const
{
    switch( mDropAction ) {
    case Qt::CopyAction: return tr("copy drop");
    case Qt::MoveAction: return tr("move drop");
    case Qt::LinkAction: return tr("link drop");
    case s_FromClipboardAction: return tr("from clipboard");
    }
    return tr("nothing dropped");
}

DnDAction DropDataModel::dropActionData() const
{
    return mDrop;
}

void DropDataModel::setDropData(int dropAction, const DnDAction &data)
{
    beginResetModel();
    mDropAction = dropAction;
    mDrop = data;
    endResetModel();
}

QModelIndex DropDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if( parent.isValid() )
        return {};

    return createIndex(row, column);
}

QModelIndex DropDataModel::parent(const QModelIndex &) const
{
    return {};
}
