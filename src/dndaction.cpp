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

#include "dndaction.h"

#include <QCryptographicHash>
#include <QMimeDatabase>


QString DnDAction::actionsToString(Qt::DropActions actions)
{
    QStringList tmp;

    if( actions & Qt::CopyAction )
        tmp.append("copy");
    if( actions & Qt::MoveAction )
        tmp.append("move");
    if( actions & Qt::LinkAction )
        tmp.append("link");

    QString res = tmp.join(" | ");
    return res.isEmpty() ? "ignore" : res;
}

DnDAction::DataEntry::DataEntry()
{
}

DnDAction::DataEntry::DataEntry(const QString &mime, const QByteArray &data)
    : mMime(mime), mBytes(data), mFileExtension("?")
{
}

QString DnDAction::DataEntry::mime() const
{
    return mMime;
}

QByteArray DnDAction::DataEntry::bytes() const
{
    return mBytes;
}

QString DnDAction::DataEntry::fileExtension() const
{
    static QMimeDatabase mimeDb;

    if( mFileExtension.size() == 1 )
        mFileExtension = mimeDb.mimeTypeForName(mime()).preferredSuffix();

    return mFileExtension;
}

qint64 DnDAction::DataEntry::fileSize() const
{
    return mBytes.size();
}

QString DnDAction::DataEntry::sha1() const
{
    if( mSha1.isEmpty() ) {
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(mBytes);
        mSha1 = QString::fromLatin1(hash.result().toHex());
    }

    return mSha1;
}


QDataStream &operator<<(QDataStream &stream, const DnDAction::DataEntry &entry)
{
    return stream << entry.mime() << entry.bytes();
}


QDataStream &operator>>(QDataStream &stream, DnDAction::DataEntry &entry)
{
    QString mime;
    QByteArray bytes;
    stream >> mime >> bytes;
    entry = DnDAction::DataEntry(mime, bytes);
    return stream;
}


QDataStream &operator<<(QDataStream &stream, const DnDAction &action)
{
    return stream << int(action.supportedActions) << int(action.defaultAction) << action.data;
}


QDataStream &operator>>(QDataStream &stream, DnDAction &action)
{
    qint32 sup, def;
    stream >> sup >> def;
    action.supportedActions = static_cast<Qt::DropActions>(sup);
    action.defaultAction = static_cast<Qt::DropAction>(def);
    return stream >> action.data;
}
