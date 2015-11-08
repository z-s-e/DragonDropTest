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

#ifndef DRAGSOURCE_H
#define DRAGSOURCE_H

#include "dndaction.h"

#include "formgenwidgetsbase.h"

#include <QAbstractListModel>
#include <QLabel>
#include <QVector>


class QMimeData;
class QToolButton;


class DragSource : public QLabel {
    Q_OBJECT

public:
    explicit DragSource(QWidget *parent = 0);

    void setData(const DnDAction &action);

    QMimeData *toNewMimeData() const;

protected:
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

private:
    DnDAction mAction;
};


class DragSourceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ~DragSourceModel();

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    struct DragSourceEntry {
        QString name;
        DnDAction action;
    };

    QVector<DragSourceEntry>::const_iterator begin() const;
    QVector<DragSourceEntry>::const_iterator end() const;

    const DragSourceEntry &at(int idx) const;

    void clear();
    void append(const DragSourceEntry &entry);

signals:
    void rowCountChanged();

private:
    QVector<DragSourceEntry> mDragSources;

    friend QDataStream &operator<<(QDataStream &stream, const DragSourceModel &model);
    friend QDataStream &operator>>(QDataStream &stream, DragSourceModel &model);
};

QDataStream &operator<<(QDataStream &stream, const DragSourceModel &model);
QDataStream &operator>>(QDataStream &stream, DragSourceModel &model);


class FormGenByteArrayWidget : public FormGenUnframedBase {
    Q_OBJECT

public:
    explicit FormGenByteArrayWidget(ElementType type = Required, QWidget * parent = nullptr);

    QVariant defaultValue() const override;

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

    void updateInputWidgets() override;

private slots:
    void showFileChooser();

private:
    QString byteArraySummary(const QByteArray &array) const;

    QLabel * mInfo;
    QToolButton * mDataChooser;
    QByteArray mValue;

    friend class FormGenFileUrlList;
};

#endif // DRAGSOURCE_H
