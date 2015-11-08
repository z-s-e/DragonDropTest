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

#include "dragsource.h"

#include <QApplication>
#include <QClipboard>
#include <QCryptographicHash>
#include <QDrag>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QToolButton>

#include <QDebug>

DragSource::DragSource(QWidget *parent)
    : QLabel(parent)
{
    setToolTip(tr("Drag data from here\nRight click: copy to clipboard"));
}

void DragSource::setData(const DnDAction &action)
{
    mAction = action;
}

QMimeData *DragSource::toNewMimeData() const
{
    QMimeData *mimeData = new QMimeData;

    for( const auto &e : mAction.data)
        mimeData->setData(e.mime(), e.bytes());

    return mimeData;
}

void DragSource::mousePressEvent(QMouseEvent *ev)
{
    if( ev->button() == Qt::LeftButton) {
        QDrag *drag = new QDrag(this);
        drag->setMimeData(toNewMimeData());

        Qt::DropAction action = drag->exec(mAction.supportedActions, mAction.defaultAction);
        QString actionString;
        switch(action) {
        case Qt::CopyAction:
            actionString = tr("copy");
            break;
        case Qt::MoveAction:
            actionString = tr("move");
            break;
        case Qt::LinkAction:
            actionString = tr("link");
            break;
        default:
            actionString = tr("no");
        }
        setText(tr("<drag content>\n(%1 action)").arg(actionString));

        ev->accept();
    } else if( ev->button() == Qt::RightButton ) {
        QApplication::clipboard()->setMimeData(toNewMimeData());
        setText(tr("<drag content>\n(to clipboard)"));

        ev->accept();
    }
}

void DragSource::mouseDoubleClickEvent(QMouseEvent *)
{
}

void DragSource::mouseMoveEvent(QMouseEvent *)
{
}

void DragSource::mouseReleaseEvent(QMouseEvent *)
{
}



DragSourceModel::~DragSourceModel()
{
}


int DragSourceModel::rowCount(const QModelIndex &parent) const
{
    if( parent.isValid() )
        return 0;

    return mDragSources.size();
}

QVariant DragSourceModel::data(const QModelIndex &index, int role) const
{
    if( index.row() < 0 || index.row() >= mDragSources.size() || ! index.isValid() )
        return {};

    switch( role ) {
    case Qt::DisplayRole:
        return mDragSources.at(index.row()).name;
    }

    return {};
}

QVector<DragSourceModel::DragSourceEntry>::const_iterator DragSourceModel::begin() const
{
    return mDragSources.begin();
}

QVector<DragSourceModel::DragSourceEntry>::const_iterator DragSourceModel::end() const
{
    return mDragSources.end();
}

const DragSourceModel::DragSourceEntry &DragSourceModel::at(int idx) const
{
    return mDragSources.at(idx);
}

void DragSourceModel::clear()
{
    beginResetModel();
    mDragSources.resize(0);
    endResetModel();
    emit rowCountChanged();
}

void DragSourceModel::append(const DragSourceEntry &entry)
{
    const int row = mDragSources.size();
    beginInsertRows(QModelIndex(), row, row);
    mDragSources.append(entry);
    endInsertRows();
    emit rowCountChanged();
}


QDataStream &operator<<(QDataStream &stream, const DragSourceModel::DragSourceEntry &entry)
{
    return stream << entry.name << entry.action;
}

QDataStream &operator>>(QDataStream &stream, DragSourceModel::DragSourceEntry &entry)
{
    return stream >> entry.name >> entry.action;
}


QDataStream &operator<<(QDataStream &stream, const DragSourceModel &model)
{
    return stream << model.mDragSources;
}


QDataStream &operator>>(QDataStream &stream, DragSourceModel &model)
{
    return stream >> model.mDragSources;
}



FormGenByteArrayWidget::FormGenByteArrayWidget(FormGenElement::ElementType type, QWidget *parent)
    : FormGenUnframedBase(type, parent)
    , mInfo(new QLabel)
    , mDataChooser(new QToolButton)
{
    hboxLayout()->addWidget(mInfo, 1);

    mDataChooser->setText(tr("..."));
    connect(mDataChooser, &QToolButton::clicked, this, &FormGenByteArrayWidget::showFileChooser);
    layout()->addWidget(mDataChooser);

    mInfo->setText(byteArraySummary(mValue));
    updateInputWidgets();
}

QVariant FormGenByteArrayWidget::defaultValue() const
{
    return QByteArray();
}

QVariant FormGenByteArrayWidget::valueImpl() const
{
    return mValue;
}

QString FormGenByteArrayWidget::valueStringImpl() const
{
    return mInfo->text();
}

FormGenAcceptResult FormGenByteArrayWidget::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) == QMetaType::QByteArray )
        return FormGenAcceptResult::accept(val, byteArraySummary(val.toByteArray()));

    return FormGenAcceptResult::reject({}, val);
}

void FormGenByteArrayWidget::setVaidatedValueImpl(const QVariant &val)
{
    const QByteArray array = val.toByteArray();

    if( mValue != array ) {
        mValue = array;
        mInfo->setText(byteArraySummary(mValue));
        emit valueChanged();
    }
}

void FormGenByteArrayWidget::updateInputWidgets()
{
    mDataChooser->setEnabled(isValueSet());
}

void FormGenByteArrayWidget::showFileChooser()
{
    QFileDialog dialog(this, tr("Select file"));
    dialog.setFileMode(QFileDialog::AnyFile);

    if( dialog.exec() != QDialog::Accepted )
        return;
    if( dialog.selectedFiles().size() < 1 )
        return;

    QFile f(dialog.selectedFiles().first());
    if( ! f.open(QIODevice::ReadOnly) ) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot open file %s"), qPrintable(dialog.selectedFiles().first()));
        return;
    }

    setValue(f.readAll());
}

QString FormGenByteArrayWidget::byteArraySummary(const QByteArray &array) const
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(array);
    return tr("<BLOB %1 bytes, %2 SHA-1>").arg(QString::number(array.size()),
                                               QString::fromLatin1(hash.result().toHex()));
}
