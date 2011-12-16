/*
 *  Copyright (C) 2011 Alexandre Courbot <gnurou@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KDEVKERNELPLUGIN_H
#define KDEVKERNELPLUGIN_H

#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/abstractfilemanagerplugin.h>
#include <QVariant>

class KJob;
namespace KDevelop
{
class ProjectBaseItem;
class ProjectTargetItem;
class ProjectFileItem;
class ProjectFolderItem;
class IProject;
}

class KDevKernelPlugin : public KDevelop::AbstractFileManagerPlugin, public KDevelop::IBuildSystemManager, public KDevelop::IProjectBuilder
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IProjectBuilder)
    Q_INTERFACES(KDevelop::IProjectFileManager)
    Q_INTERFACES(KDevelop::IBuildSystemManager)
public:
    KDevKernelPlugin(QObject *parent, const QVariantList &args);

    // AbstractFileManagerPlugin interface

    // IBuildSystemManager interface
    virtual KDevelop::IProjectBuilder *builder(KDevelop::ProjectFolderItem *item) const;
    virtual KUrl::List includeDirectories(KDevelop::ProjectBaseItem *item) const;
    virtual QHash<QString,QString> defines(KDevelop::ProjectBaseItem *item) const;
    virtual KDevelop::ProjectTargetItem *createTarget(const QString& target, KDevelop::ProjectFolderItem *parent);
    virtual bool removeTarget(KDevelop::ProjectTargetItem *target);
    virtual QList<KDevelop::ProjectTargetItem *> targets(KDevelop::ProjectFolderItem *item) const;
    virtual bool addFilesToTarget(const QList<KDevelop::ProjectFileItem *> &files, KDevelop::ProjectTargetItem *target);
    virtual bool removeFilesFromTargets(const QList<KDevelop::ProjectFileItem *> &files);
    virtual KUrl buildDirectory(KDevelop::ProjectBaseItem *item) const;

    // IProjectBuilder interface
    virtual KJob *install(KDevelop::ProjectBaseItem* item);
    virtual KJob *build(KDevelop::ProjectBaseItem *item);
    virtual KJob *clean(KDevelop::ProjectBaseItem *item);
    virtual KJob *configure(KDevelop::IProject *item);
    virtual KJob *prune(KDevelop::IProject *item);
};

#endif
