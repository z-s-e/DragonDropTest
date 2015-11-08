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

#include "widget.h"
#include "ui_widget.h"

#include "dragsource.h"

#include "formgenwidgets-qt.h"

#include <QApplication>
#include <QClipboard>
#include <QCompleter>
#include <QDataStream>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QMimeDatabase>
#include <QUrl>

static const QString s_dragDataName = "name";
static const QString s_dragDataSupCopy = "supportCopy";
static const QString s_dragDataSupMove = "supportMove";
static const QString s_dragDataSupLink = "supportLink";
static const QString s_dragDataDefault = "defaultAction";
static const QString s_dragDataIgnore = "ignore";
static const QString s_dragDataCopy = "copy";
static const QString s_dragDataMove = "move";
static const QString s_dragDataLink = "link";
static const QString s_dragDataData = "data";
static const QString s_dragDataBytes = "bytes";
static const QString s_dragDataMime = "mime";

static const char s_dragSourceConfigHead[] = "DragonDropTest.dragSourceConfig.0.1";

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->listDrop->setModel(&mDropModel);
    ui->listDrag->setModel(&mDragModel);

    connect(ui->labelDrop, SIGNAL(dataDropped(int,DnDAction)), this, SLOT(onDataDropped(int,DnDAction)));
    onDataDropped(-2, {});

    connect(&mDragModel, SIGNAL(rowCountChanged()), this, SLOT(updateUi()));
    connect(ui->listDrag->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(updateUi()));
    connect(&mDropModel, SIGNAL(modelReset()), this, SLOT(updateUi()));
    connect(ui->listDrop->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(updateUi()));

    connect(ui->buttonDragEdit, SIGNAL(clicked()), this, SLOT(dragEdit()));
    connect(ui->buttonDragLoad, SIGNAL(clicked()), this, SLOT(dragLoad()));
    connect(ui->buttonDragSave, SIGNAL(clicked()), this, SLOT(dragSave()));
    connect(ui->buttonDropClip, SIGNAL(clicked()), this, SLOT(dropClip()));
    connect(ui->buttonDropOpen, SIGNAL(clicked()), this, SLOT(dropOpen()));
    connect(ui->buttonDropSave, SIGNAL(clicked()), this, SLOT(dropSave()));
    connect(ui->buttonDropToDrag, SIGNAL(clicked()), this, SLOT(dropToDrag()));

    updateUi();
}

Widget::~Widget()
{
    delete ui;
}


void Widget::updateUi()
{
    const bool dropDataSelected = ui->listDrop->selectionModel()->currentIndex().row() >= 0 && mDropModel.rowCount() > 0;
    ui->buttonDropClip->setEnabled(dropDataSelected);
    ui->buttonDropOpen->setEnabled(dropDataSelected);
    ui->buttonDropSave->setEnabled(dropDataSelected);

    ui->buttonDragSave->setEnabled(mDragModel.rowCount() > 0);
    const int dragRow = ui->listDrag->selectionModel()->currentIndex().row();
    if( dragRow < 0 ) {
        ui->labelDrag->setData({});
        ui->labelDragSupported->setText({});
        ui->labelDragDefault->setText({});
    } else {
        const auto &entry = mDragModel.at(dragRow);
        ui->labelDrag->setData(entry.action);
        ui->labelDragSupported->setText(tr("Supported actions:  %1").arg(DnDAction::actionsToString(entry.action.supportedActions)));
        ui->labelDragDefault->setText(tr("Default action:  %1").arg(DnDAction::actionsToString(entry.action.defaultAction)));
    }
}

void Widget::dropSave()
{
    const int row = ui->listDrop->selectionModel()->currentIndex().row();

    QString fileExt = mDropModel.dropFileExtension(row);
    QString fileName = QFileDialog::getSaveFileName(0, tr("Save drop data"), QString(),
                                                    fileExt.isEmpty() ? QString()
                                                                      : QString("*.%1").arg(fileExt));
    if( fileName.isEmpty() )
        return;

    QFile f(fileName);
    if( ! f.open(QIODevice::WriteOnly) ) {
        showError(tr("Save drop: cannot open file to save"));
        return;
    }

    f.write(mDropModel.dropData(row));
    f.close();
}

void Widget::dropOpen()
{
    const int row = ui->listDrop->selectionModel()->currentIndex().row();

    mTmpFile.reset(new QTemporaryFile);
    QString fileExt = mDropModel.dropFileExtension(row);
    if( ! fileExt.isEmpty() )
        mTmpFile->setFileTemplate(QString("%1/DragonDropTest-XXXXXX.%2").arg(QDir::tempPath(), fileExt));

    if( ! mTmpFile->open() ) {
        showError(tr("Open drop: could not create temporary file"));
        return;
    }

    mTmpFile->write(mDropModel.dropData(row));
    mTmpFile->close();

    if( ! QDesktopServices::openUrl(QUrl::fromLocalFile(mTmpFile->fileName())) )
        showError(tr("Open drop: could not open temporary file %1").arg(mTmpFile->fileName()));
}

void Widget::dropClip()
{
    const int row = ui->listDrop->selectionModel()->currentIndex().row();

    auto *clipboardData = new QMimeData;
    clipboardData->setData(mDropModel.dropMimeType(row),
                           mDropModel.dropData(row));

    QApplication::clipboard()->setMimeData(clipboardData);
}


void Widget::loadDragSourceConfig(const QUrl &configUrl)
{
    mDragModel.clear();

    QFile file(configUrl.toLocalFile());
    if ( ! file.open(QIODevice::ReadOnly)) {
        showError(tr("Cannot open config file %1").arg(configUrl.toString()));
        return;
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_2);
    {
        char headerTmp[sizeof(s_dragSourceConfigHead)];
        stream.readRawData(headerTmp, sizeof(s_dragSourceConfigHead));
        if( qstrcmp(headerTmp, s_dragSourceConfigHead) != 0 ) {
            showError(tr("Invalid config file"));
            return;
        }
    }

    stream >> mDragModel;
}

void Widget::onDataDropped(int dropAction, const DnDAction &data)
{
    mDropModel.setDropData(dropAction, data);
    ui->labelDrop->setDropActionString(mDropModel.dropActioString());
    ui->labelDropPossible->setText(tr("Possible actions:  %1").arg(DnDAction::actionsToString(data.supportedActions)));
    ui->labelDropSuggested->setText(tr("Suggested action:  %1").arg(DnDAction::actionsToString(data.defaultAction)));
}


void Widget::dragLoad()
{
    QFileDialog dialog(this, tr("Select drag source config file"), {}, "*.dndtest");
    dialog.setFileMode(QFileDialog::ExistingFile);
    if( dialog.exec() != QDialog::Accepted )
        return;

    loadDragSourceConfig(dialog.selectedUrls().first());
}

void Widget::dragEdit()
{
    QDialog dialog;
    dialog.setWindowTitle(tr("Edit drag sources"));
    auto *layout = new QVBoxLayout;

    auto *dragSourceList = new FormGenListBagComposition(FormGenListBagComposition::ListMode);
    dragSourceList->frameWidget()->setTitle(dialog.windowTitle());
    {
        auto * dragSource = new FormGenRecordComposition;
        dragSource->addElement(s_dragDataName, new FormGenTextWidget, tr("Name"));
        dragSource->addElement(s_dragDataSupCopy, new FormGenBoolWidget, tr("Support copy action"));
        dragSource->addElement(s_dragDataSupMove, new FormGenBoolWidget, tr("Support move action"));
        dragSource->addElement(s_dragDataSupLink, new FormGenBoolWidget, tr("Support link action"));

        auto * defaultDrag = new FormGenEnumWidget;
        defaultDrag->addEnumValue(s_dragDataIgnore);
        defaultDrag->addEnumValue(s_dragDataCopy);
        defaultDrag->addEnumValue(s_dragDataMove);
        defaultDrag->addEnumValue(s_dragDataLink);
        dragSource->addElement(s_dragDataDefault, defaultDrag, tr("Default action"));

        auto * dataElement = new FormGenRecordComposition;
        auto *mimeEntry = new FormGenTextWidget;
        {
            QStringList mime;
            const QList<QMimeType> db = QMimeDatabase().allMimeTypes();
            for( auto it = db.cbegin(); it != db.cend(); ++it )
                mime << it->name();
            qSort(mime);

            auto *c = new QCompleter(mime, mimeEntry);
            mimeEntry->findChild<QLineEdit *>()->setCompleter(c);
        }
        dataElement->addElement(s_dragDataMime, mimeEntry, tr("MIME"));
        auto * bytes = new FormGenByteArrayWidget;
        dataElement->addElement(s_dragDataBytes, bytes, tr("Bytes"));
        auto * dataList = new FormGenListBagComposition(FormGenListBagComposition::BagMode);
        dataList->setContentElement(dataElement);
        dragSource->addElement(s_dragDataData, dataList, tr("Data"));

        dragSourceList->setContentElement(dragSource, tr("Entry"));
    }
    layout->addWidget(dragSourceList);

    auto *buttonBox = new QDialogButtonBox;
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    buttonBox->connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    buttonBox->connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);

    dialog.setLayout(layout);

    Q_ASSERT(dragSourceList->acceptsValue(dragSourceConfigVariant()).acceptable);
    dragSourceList->setValue(dragSourceConfigVariant());

    if( dialog.exec() != QDialog::Accepted )
        return;

    loadDragSourceConfig(dragSourceList->value().toList());
}

void Widget::dragSave()
{
    QString saveFile = QFileDialog::getSaveFileName(this,
                                                    tr("Save drag source config file"),
                                                    {}, "*.dndtest");

    if( saveFile.isEmpty() )
        return;
    if( ! saveFile.contains('.') )
        saveFile.append(".dndtest");

    QFile file(saveFile);
    if( ! file.open(QIODevice::WriteOnly) ) {
        showError(tr("Save drag config: cannot open file"));
        return;
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_2);
    stream.writeRawData(s_dragSourceConfigHead, sizeof(s_dragSourceConfigHead));
    stream << mDragModel;
}

void Widget::dropToDrag()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("Drag source name"),
                                         tr("Name:"), QLineEdit::Normal,
                                         {}, &ok);
    if (ok && !name.isEmpty()) {
        DragSourceModel::DragSourceEntry e;
        e.name = name;
        e.action = mDropModel.dropActionData();
        mDragModel.append(e);
    }
}

void Widget::showError(const QString &message)
{
    qWarning("%s", qPrintable(message));
    QMessageBox::critical(this, tr("Error"), message);
}

static Qt::DropAction defaultActionFromVariant(const QVariant &var)
{
    QString s = var.toHash().cbegin().key();
    if( s == s_dragDataCopy )
        return Qt::CopyAction;
    else if( s == s_dragDataMove )
        return Qt::MoveAction;
    else if( s == s_dragDataLink )
        return Qt::LinkAction;
    return Qt::IgnoreAction;
}

static QVariant defaultActionVariant(Qt::DropAction action)
{
    QVariantHash v;
    switch( action ) {
    case Qt::IgnoreAction:
        v[s_dragDataIgnore] = FormGenVoidWidget::voidValue();
        break;
    case Qt::CopyAction:
        v[s_dragDataCopy] = FormGenVoidWidget::voidValue();
        break;
    case Qt::MoveAction:
        v[s_dragDataMove] = FormGenVoidWidget::voidValue();
        break;
    case Qt::LinkAction:
        v[s_dragDataLink] = FormGenVoidWidget::voidValue();
        break;
    default:
        qFatal("Unexpected action %d", action);
    }
    return v;
}

void Widget::loadDragSourceConfig(const QVariantList &data)
{
    mDragModel.clear();

    for( const auto & entry : data ) {
        QVariantHash e = entry.toHash();
        DragSourceModel::DragSourceEntry o;

        o.name = e.value(s_dragDataName).toString();
        o.action.supportedActions = Qt::IgnoreAction;
        o.action.supportedActions |= e.value(s_dragDataSupCopy).toBool() ? Qt::CopyAction : Qt::IgnoreAction;
        o.action.supportedActions |= e.value(s_dragDataSupMove).toBool() ? Qt::MoveAction : Qt::IgnoreAction;
        o.action.supportedActions |= e.value(s_dragDataSupLink).toBool() ? Qt::LinkAction : Qt::IgnoreAction;
        o.action.defaultAction = defaultActionFromVariant(e.value(s_dragDataDefault));

        QVariantList entryData = e.value(s_dragDataData).toList();
        for( int i = 0; i < entryData.size(); ++i ) {
            QVariantHash ed = entryData.at(i).toHash();
            o.action.data.append(DnDAction::DataEntry(ed.value(s_dragDataMime).toString(),
                                                      ed.value(s_dragDataBytes).toByteArray()));
        }

        mDragModel.append(o);
    }
}

QVariantList Widget::dragSourceConfigVariant() const
{
    QVariantList data;

    for( const auto &entry : mDragModel ) {
        QVariantHash e;
        e[s_dragDataName] = entry.name;
        e[s_dragDataSupCopy] = bool(entry.action.supportedActions & Qt::CopyAction);
        e[s_dragDataSupMove] = bool(entry.action.supportedActions & Qt::MoveAction);
        e[s_dragDataSupLink] = bool(entry.action.supportedActions & Qt::LinkAction);
        e[s_dragDataDefault] = defaultActionVariant(entry.action.defaultAction);

        QVariantList eData;
        for( const auto &e : entry.action.data ) {
            QVariantHash ed;
            ed[s_dragDataMime] = e.mime();
            ed[s_dragDataBytes] = e.bytes();
            eData.append(ed);
        }
        e[s_dragDataData] = eData;

        data.append(e);
    }

    return data;
}
