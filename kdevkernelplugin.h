/*
 *  Copyright (C) 2011, 2012 Alexandre Courbot <gnurou@gmail.com>
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
#include <QSet>
#include <QMap>
#include <QHash>
#include <QDateTime>

class KJob;
class IMakeBuilder;
namespace KDevelop
{
class ProjectBaseItem;
class ProjectTargetItem;
class ProjectFileItem;
class ProjectFolderItem;
class IProject;
}

struct ValidFilesList
{
    QDateTime lastUpdate;
    QSet<QString> validFiles;
};

class KDevKernelPlugin : public KDevelop::AbstractFileManagerPlugin, public KDevelop::IBuildSystemManager, public KDevelop::IProjectBuilder
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IProjectBuilder)
    Q_INTERFACES(KDevelop::IProjectFileManager)
    Q_INTERFACES(KDevelop::IBuildSystemManager)

private:
    IMakeBuilder *_builder;

    mutable QMap<KDevelop::IProject *, QMap<KUrl, ValidFilesList > > _validFiles;
    mutable QMap<KDevelop::IProject *, QStringList> _machDirs;
    mutable QMap<KDevelop::IProject *, QHash<QString, QString> > _defines;

    /**
     * Parse the given configuration file and set the kernel definitions accordingly.
     */
    void parseDotConfig(const KUrl &dotconfig, QHash<QString, QString> &_defs);
    /**
     * Parse the Makefiles and build the list of files we need to include according
     * to the definitions that have been parsed by parseDotConfig.
     */
    void parseMakefile(const KUrl &dir, KDevelop::IProject *project) const;

public:
    KDevKernelPlugin(QObject *parent, const QVariantList &args);

    // AbstractFileManagerPlugin interface
    virtual KDevelop::ProjectFolderItem *import(KDevelop::IProject *project);

    // IBuildSystemManager interface
    virtual KDevelop::IProjectBuilder *builder() const;
    virtual KUrl::List includeDirectories(KDevelop::ProjectBaseItem *item) const;
    virtual KUrl::List includeDirectories(KDevelop::IProject *project) const;
    virtual QHash<QString, QString> defines(KDevelop::ProjectBaseItem *item) const;
    virtual KDevelop::ProjectTargetItem *createTarget(const QString &target, KDevelop::ProjectFolderItem *parent);
    virtual bool removeTarget(KDevelop::ProjectTargetItem *target);
    virtual QList<KDevelop::ProjectTargetItem *> targets(KDevelop::ProjectFolderItem *item) const;
    virtual bool addFilesToTarget(const QList<KDevelop::ProjectFileItem *> &files, KDevelop::ProjectTargetItem *target);
    virtual bool removeFilesFromTargets(const QList<KDevelop::ProjectFileItem *> &files);
    /**
     * A file is valid if it belongs to the list of files that are enabled through the kernel configuration.
     * A directory is valid if it contains any file we are interested in.
     */
    virtual bool isValid(const KUrl &url, const bool isFolder, KDevelop::IProject *project) const;
    virtual KUrl buildDirectory(KDevelop::ProjectBaseItem *item) const;

    // IProjectBuilder interface
    virtual KJob *install(KDevelop::ProjectBaseItem *item);
    virtual KJob *build(KDevelop::ProjectBaseItem *item);
    virtual KJob *clean(KDevelop::ProjectBaseItem *item);
    virtual KJob *configure(KDevelop::IProject *item);
    virtual KJob *prune(KDevelop::IProject *item);

protected:
    virtual KJob *jobForTarget(KDevelop::IProject *item, const QString &target);

private slots:
    void projectClosing(KDevelop::IProject *project);
};

#endif
