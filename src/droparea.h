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

#ifndef DROPAREA_H
#define DROPAREA_H

#include <QAbstractItemModel>
#include <QLabel>

#include "dndaction.h"


class DropArea : public QLabel {
    Q_OBJECT

public:
    DropArea(QWidget *parent = 0);

    void setDropActionString(const QString &dropAction);

signals:
    void dataDropped(int dropAction, const DnDAction &data);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *) override;
    void dropEvent(QDropEvent *event) override;

    void mousePressEvent(QMouseEvent *ev) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

private slots:
    void fromClipboard();

private:
    void clear();
    void acceptDropEvent(QDropEvent *event);
    static Qt::DropAction overrideAction(Qt::KeyboardModifiers modifiers);
};


class DropDataModel : public QAbstractItemModel {
    Q_OBJECT

public:
    DropDataModel(QObject * parent = 0);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int	columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    int	rowCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QString dropMimeType(int row) const;
    QString dropFileExtension(int row) const;
    QByteArray dropData(int row) const;
    QString dropActioString() const;
    DnDAction dropActionData() const;

public slots:
    void setDropData(int dropAction, const DnDAction &data);

private:
    int mDropAction;
    DnDAction mDrop;
};

#endif // DROPAREA_H
