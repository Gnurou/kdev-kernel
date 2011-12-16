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

#include "kdevkernelplugin.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugin.h>
#include <project/projectmodel.h>
#include <KPluginFactory>
#include <KLocalizedString>
#include <KAboutData>
#include <QObject>

K_PLUGIN_FACTORY(KernelProjectFactory, registerPlugin<KDevKernelPlugin>();)
K_EXPORT_PLUGIN(KernelProjectFactory(
    KAboutData("kdevkernel","kdevkernel",
        ki18n("Kernel Project"),
        "0.1",
        ki18n("Linux Kernel Project Manager"),
        KAboutData::License_GPL,
        ki18n("Copyright (C) 2011 Alexandre Courbot <gnurou@gmail.com>"),
        KLocalizedString(),
        "",
        "gnurou@gmail.com"
    )
))

KDevKernelPlugin::KDevKernelPlugin(QObject *parent, const QVariantList &args)
    : KDevelop::AbstractFileManagerPlugin(KernelProjectFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IBuildSystemManager)
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IProjectFileManager)
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IBuildSystemManager)
}

KDevelop::IProjectBuilder *KDevKernelPlugin::builder(KDevelop::ProjectFolderItem *item) const
{
    return 0;
}

KUrl::List KDevKernelPlugin::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    return KUrl::List();
}

QHash<QString,QString> KDevKernelPlugin::defines(KDevelop::ProjectBaseItem *item) const
{
    QHash<QString, QString> defines;
    defines.insert("__KERNEL__", "1");
    defines.insert("CONFIG_PM", "1");
    return defines;
}

KDevelop::ProjectTargetItem *KDevKernelPlugin::createTarget(const QString& target, KDevelop::ProjectFolderItem *parent)
{
    return 0;
}

bool KDevKernelPlugin::removeTarget(KDevelop::ProjectTargetItem *target)
{
    return false;
}

QList<KDevelop::ProjectTargetItem *> KDevKernelPlugin::targets(KDevelop::ProjectFolderItem *item) const
{
    return QList<KDevelop::ProjectTargetItem *>();
}

bool KDevKernelPlugin::addFilesToTarget(const QList<KDevelop::ProjectFileItem *> &files, KDevelop::ProjectTargetItem *target)
{
    return false;
}

bool KDevKernelPlugin::removeFilesFromTargets(const QList<KDevelop::ProjectFileItem *> &files)
{
    return false;
}

KUrl KDevKernelPlugin::buildDirectory(KDevelop::ProjectBaseItem *item) const
{
    KUrl buildDir(item->project()->projectItem()->url());
    return buildDir;
}

KJob *KDevKernelPlugin::install(KDevelop::ProjectBaseItem *item)
{
    return 0;
}

KJob *KDevKernelPlugin::build(KDevelop::ProjectBaseItem *item)
{
    return 0;
}

KJob *KDevKernelPlugin::clean(KDevelop::ProjectBaseItem *item)
{
    return 0;
}

KJob *KDevKernelPlugin::configure(KDevelop::IProject *project)
{
    return 0;
}

KJob *KDevKernelPlugin::prune(KDevelop::IProject *project)
{
    return 0;
}

#include "kdevkernelplugin.moc"
