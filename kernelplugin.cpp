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

#include "kernelplugin.h"
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugin.h>
#include <project/projectmodel.h>
#include <KPluginFactory>
#include <KLocalizedString>
#include <KAboutData>
#include <QObject>

K_PLUGIN_FACTORY(KernelProjectFactory, registerPlugin<KernelPlugin>();)
K_EXPORT_PLUGIN(KernelProjectFactory(
    KAboutData("kdevkernel","kdevkernel",
        ki18n("Kernel Project"),
        "0.1",
        ki18n("Kernel Project Manager"),
        KAboutData::License_GPL,
        ki18n("Copyright (C) 2011 Alexandre Courbot <gnurou@gmail.com>"),
        KLocalizedString(),
        "",
        "gnurou@gmail.com"
    )
))

KernelPlugin::KernelPlugin(QObject *parent, const QVariantList &args)
    : KDevelop::AbstractFileManagerPlugin(KernelProjectFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IBuildSystemManager)
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IProjectFileManager)
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IBuildSystemManager)
}

KDevelop::IProjectBuilder *KernelPlugin::builder(KDevelop::ProjectFolderItem *item) const
{
    return 0;
}

KUrl::List KernelPlugin::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    return KUrl::List();
}

QHash<QString,QString> KernelPlugin::defines(KDevelop::ProjectBaseItem *item) const
{
    QHash<QString, QString> defines;
    defines.insert("__KERNEL__", "1");
    return defines;
}

KDevelop::ProjectTargetItem *KernelPlugin::createTarget(const QString& target, KDevelop::ProjectFolderItem *parent)
{
    return 0;
}

bool KernelPlugin::removeTarget(KDevelop::ProjectTargetItem *target)
{
    return false;
}

QList<KDevelop::ProjectTargetItem *> KernelPlugin::targets(KDevelop::ProjectFolderItem *item) const
{
    return QList<KDevelop::ProjectTargetItem *>();
}

bool KernelPlugin::addFilesToTarget(const QList<KDevelop::ProjectFileItem *> &files, KDevelop::ProjectTargetItem *target)
{
    return false;
}

bool KernelPlugin::removeFilesFromTargets(const QList<KDevelop::ProjectFileItem *> &files)
{
    return false;
}

KUrl KernelPlugin::buildDirectory(KDevelop::ProjectBaseItem *item) const
{
    KUrl buildDir(item->project()->projectItem()->url());
    return buildDir;
}

KJob *KernelPlugin::install(KDevelop::ProjectBaseItem *item)
{
    return 0;
}

KJob *KernelPlugin::build(KDevelop::ProjectBaseItem *item)
{
    return 0;
}

KJob *KernelPlugin::clean(KDevelop::ProjectBaseItem *item)
{
    return 0;
}

KJob *KernelPlugin::configure(KDevelop::IProject *project)
{
    return 0;
}

KJob *KernelPlugin::prune(KDevelop::IProject *project)
{
    return 0;
}

#include "kernelplugin.moc"
