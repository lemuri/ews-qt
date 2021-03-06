/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "syncfolderhierarchyreply_p.h"
#include "folder_p.h"

#include <QDebug>

using namespace Ews;

SyncFolderHierarchyReply::SyncFolderHierarchyReply(SyncFolderHierarchyReplyPrivate *priv) :
    Reply(priv)
{
}

QString SyncFolderHierarchyReply::syncState() const
{
    Q_D(const SyncFolderHierarchyReply);
    return d->syncState;
}

bool SyncFolderHierarchyReply::includesLastFolderInRange() const
{
    Q_D(const SyncFolderHierarchyReply);
    return d->includesLastFolderInRange;
}

QList<Folder> SyncFolderHierarchyReply::createFolders() const
{
    Q_D(const SyncFolderHierarchyReply);
    return d->createFolders;
}

QList<Folder> SyncFolderHierarchyReply::updateFolders() const
{
    Q_D(const SyncFolderHierarchyReply);
    return d->updateFolders;
}

QStringList SyncFolderHierarchyReply::deleteFolders() const
{
    Q_D(const SyncFolderHierarchyReply);
    return d->deleteFolders;
}

SyncFolderHierarchyReplyPrivate::SyncFolderHierarchyReplyPrivate(KDSoapJob *job) :
    ReplyPrivate(job, this),
    includesLastFolderInRange(false)
{

}

void SyncFolderHierarchyReplyPrivate::processJob(KDSoapJob *job)
{
    SyncFolderHierarchyJob *syncJob = qobject_cast<SyncFolderHierarchyJob*>(job);
    const TNS__SyncFolderHierarchyResponseType &response = syncJob->syncFolderHierarchyResult();

    TNS__ArrayOfResponseMessagesType messages = response.responseMessages();

    QList<TNS__SyncFolderHierarchyResponseMessageType> responseMsgs;
    responseMsgs = messages.syncFolderHierarchyResponseMessage();

    foreach (const TNS__SyncFolderHierarchyResponseMessageType &msg, responseMsgs) {
        setResponseMessage(msg);

        syncState = msg.syncState();
        includesLastFolderInRange = msg.includesLastFolderInRange();

        T__SyncFolderHierarchyChangesType changes = msg.changes();
        foreach (const T__SyncFolderHierarchyCreateOrUpdateType &create, changes.create()) {
            if (!create.folder().folderId().id().isNull()) {
                FolderPrivate *priv = new FolderPrivate(create.folder());
                createFolders << Folder(*priv);
            } else {
                qWarning() << Q_FUNC_INFO << "Unknown create type";
            }
        }

        foreach (const T__SyncFolderHierarchyCreateOrUpdateType &update, changes.update()) {
            if (!update.folder().folderId().id().isNull()) {
                FolderPrivate *priv = new FolderPrivate(update.folder());
                updateFolders << Folder(*priv);
            } else {
                qWarning() << Q_FUNC_INFO << "Unknown update type";
            }
        }

        foreach (const T__SyncFolderHierarchyDeleteType &deleteFolder, changes.delete_()) {
            deleteFolders << deleteFolder.folderId().id();
        }
    }
}
