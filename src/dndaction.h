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

#ifndef DNDACTION_H
#define DNDACTION_H

#include <QDataStream>
#include <QString>
#include <QVector>
#include <QWidget>

class DnDAction
{
public:
    class DataEntry {
    public:
        DataEntry();
        DataEntry(const QString &mime, const QByteArray &data);

        QString mime() const;
        QByteArray bytes() const;
        QString fileExtension() const;
        qint64 fileSize() const;
        QString sha1() const;

    private:
        QString mMime;
        QByteArray mBytes;
        mutable QString mFileExtension;
        mutable QString mSha1;
    };

    Qt::DropActions supportedActions = Qt::IgnoreAction;
    Qt::DropAction defaultAction = Qt::IgnoreAction;
    QVector<DataEntry> data;

    static QString actionsToString(Qt::DropActions actions);
};

QDataStream &operator<<(QDataStream &stream, const DnDAction &action);
QDataStream &operator>>(QDataStream &stream, DnDAction &action);

Q_DECLARE_METATYPE(DnDAction)

#endif // DNDACTION_H
