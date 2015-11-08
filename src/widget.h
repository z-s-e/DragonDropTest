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

#ifndef WIDGET_H
#define WIDGET_H

#include "dragsource.h"
#include "droparea.h"

#include <QTemporaryFile>
#include <QWidget>

namespace Ui {
class Widget;
}
class DropDataModel;

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    void loadDragSourceConfig(const QUrl &configUrl);

private slots:
    void onDataDropped(int dropAction, const DnDAction &data);
    void updateUi();

    void dropSave();
    void dropOpen();
    void dropClip();

    void dragLoad();
    void dragEdit();
    void dragSave();

    void dropToDrag();

    void showError(const QString &message);

private:
    void loadDragSourceConfig(const QVariantList &data);
    QVariantList dragSourceConfigVariant() const;

    Ui::Widget *ui;
    DropDataModel mDropModel;
    DragSourceModel mDragModel;
    QScopedPointer<QTemporaryFile> mTmpFile;
};

#endif // WIDGET_H
