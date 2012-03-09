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

#include <QFile>
#include <QtDebug>

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
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IProjectBuilder)
}

KDevelop::IProjectBuilder *KDevKernelPlugin::builder(KDevelop::ProjectFolderItem *item) const
{
    return (KDevelop::IProjectBuilder *)(this);
}

KUrl::List KDevKernelPlugin::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    KUrl::List ret;
    KUrl projectRoot = item->project()->folder();
    ret << KUrl(projectRoot, "include");
    ret << KUrl(projectRoot, "arch/arm/include");
    ret << KUrl(projectRoot, "arch/arm/mach-tegra/include");

    return ret;
}

QHash<QString,QString> KDevKernelPlugin::defines(KDevelop::ProjectBaseItem *item) const
{
    QHash<QString, QString> defines;
    defines.insert("__KERNEL__", "");
    defines.insert("CONFIG_PM", "1");
    return defines;
}

void KDevKernelPlugin::parseDotConfig(const QString &dotconfig)
{
}

void KDevKernelPlugin::parseMakefiles()
{
    QString root = "/home/gnurou/Work/Linux/linux/";
    parseMakefiles(root + "kernel/");
}

void KDevKernelPlugin::parseMakefiles(const QString &dir)
{
    QFile makefile(dir + "Makefile");
    static QRegExp objy("obj-y[\t ]*\\+?=([^\\\\]+)\\\\?\n");
    static QRegExp spTab("\t| ");

    if (!makefile.exists() || !makefile.open(QIODevice::ReadOnly)) return;

    QStringList files;
    while (1) {
        QString line(makefile.readLine());
	if (line.isEmpty()) break;
        if (objy.exactMatch(line)) {
		files += objy.capturedTexts()[1].split(spTab, QString::SkipEmptyParts);
		while (line.endsWith("\\\n")) {
			line = makefile.readLine();
			if (line.isEmpty()) break;
			QString line2(line);
			line2.remove("\\\n");
			files += line2.split(spTab, QString::SkipEmptyParts);
		}
	}
    }
    qDebug() << "kernel files for" << dir << ":" << files;
    foreach (const QString &file, files) {
	    if (file.endsWith('/')) parseMakefiles(dir + file);
    }
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

bool KDevKernelPlugin::isValid(const KUrl &url, const bool isFolder, KDevelop::IProject *project) const
{
    return true;
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
    parseMakefiles();
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
