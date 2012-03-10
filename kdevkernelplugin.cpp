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
    Q_UNUSED(args);
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IBuildSystemManager)
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IProjectFileManager)
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IProjectBuilder)
}

KDevelop::IProjectBuilder *KDevKernelPlugin::builder(KDevelop::ProjectFolderItem *item) const
{
    Q_UNUSED(item);
    return (KDevelop::IProjectBuilder *)(this);
}

KUrl::List KDevKernelPlugin::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    KUrl::List ret;
    KUrl projectRoot = item->project()->folder();
    ret << KUrl(projectRoot, "include");
    ret << KUrl(projectRoot, "arch/x86");
    // TODO also include the mach include directory if it exists
    // TODO /usr/include and such should not be looked for

    return ret;
}

QHash<QString,QString> KDevKernelPlugin::defines(KDevelop::ProjectBaseItem *item) const
{
    return _defines[item->project()];
}

void KDevKernelPlugin::parseDotConfig(const KUrl &dotconfig, QHash<QString, QString> &_defs)
{
    QFile dfile(dotconfig.toLocalFile());
    static QRegExp def("(\\w+)=(\"?[^\\n]+\"?)\n?");

    qDebug() << "kernel dotconfig" << dotconfig;
    if (!dfile.exists() || !dfile.open(QIODevice::ReadOnly)) return;

    while (1) {
        QString line(dfile.readLine());
	if (line.isEmpty()) break;
	if (!def.exactMatch(line)) continue;
	QString key(def.cap(1));
	QString val(def.cap(2));
	if (val == "y") val = "1";
	else if (val == "n") val = "0";
	else if (val.startsWith('"') && val.endsWith('"')) val = val.mid(1, val.size() - 2);
	qDebug() << "kernel def:" << key << val;
	_defs[key] = val;
    }
}

void KDevKernelPlugin::parseMakefiles(const KUrl &dir, KDevelop::IProject *project)
{
    static QRegExp objy("obj-([\\$\\(\\)\\w]+)[\t ]*\\+?=([^\\\\]+)\\\\?\n");
    static QRegExp repl("\\$\\((\\w_+)\\)");
    static QRegExp spTab("\t| ");
    QFile makefile(KUrl(dir, "Makefile").toLocalFile());
    QSet<KUrl> &_files = _validFiles[project];

    if (!makefile.exists() || !makefile.open(QIODevice::ReadOnly)) return;

    QStringList files;
    while (1) {
        QString line(makefile.readLine());
	if (line.isEmpty()) break;
        if (objy.exactMatch(line)) {
		bool addFiles = false;
		QString y(objy.cap(1));
		if (y.startsWith("$(") && y.endsWith(")")) {
			QString def(_defines[project][y.mid(2, y.size() - 3)]);
			if (def == "1") y = "y";
		}
		if (y == "y") addFiles = true;
		// Get multi-line definitions
		if (addFiles) files += objy.cap(2).split(spTab, QString::SkipEmptyParts);
		while (line.endsWith("\\\n")) {
			line = makefile.readLine();
			if (line.isEmpty()) break;
			if (addFiles) {
				QString line2(line);
				line2.remove("\\\n");
				line2.remove("\n");
				files += line2.split(spTab, QString::SkipEmptyParts);
			}
		}
	}
    }
    foreach (QString file, files) {
	    if (file.endsWith(".o")) file = file.mid(0, file.size() - 2) + ".c";
	    KUrl f(dir, file);
	    if (file.endsWith('/')) parseMakefiles(f, project);
	    else _files << f;
    }
    if (!files.isEmpty()) _files << dir;
}

// TODO cleanup everything when the project closes!
KDevelop::ProjectFolderItem *KDevKernelPlugin::import(KDevelop::IProject *project)
{
    KUrl projectRoot(project->folder());
    QHash<QString, QString> &_defs = _defines[project];
    _defs.clear();
    // Standard definitions
    _defs["__KERNEL__"] = "";
    parseDotConfig(KUrl(projectRoot, ".config"), _defs);
    _validFiles[project].clear();
    parseMakefiles(KUrl(projectRoot, "init/"), project);
    parseMakefiles(KUrl(projectRoot, "sound/"), project);
    parseMakefiles(KUrl(projectRoot, "net/"), project);
    parseMakefiles(KUrl(projectRoot, "lib/"), project);
    parseMakefiles(KUrl(projectRoot, "usr/"), project);
    parseMakefiles(KUrl(projectRoot, "kernel/"), project);
    parseMakefiles(KUrl(projectRoot, "mm/"), project);
    parseMakefiles(KUrl(projectRoot, "fs/"), project);
    parseMakefiles(KUrl(projectRoot, "ipc/"), project);
    parseMakefiles(KUrl(projectRoot, "security/"), project);
    parseMakefiles(KUrl(projectRoot, "crypto/"), project);
    parseMakefiles(KUrl(projectRoot, "block/"), project);

    return AbstractFileManagerPlugin::import(project);
}

KDevelop::ProjectTargetItem *KDevKernelPlugin::createTarget(const QString& target, KDevelop::ProjectFolderItem *parent)
{
    Q_UNUSED(target);
    Q_UNUSED(parent);
    return 0;
}

bool KDevKernelPlugin::removeTarget(KDevelop::ProjectTargetItem *target)
{
    Q_UNUSED(target);
    return false;
}

QList<KDevelop::ProjectTargetItem *> KDevKernelPlugin::targets(KDevelop::ProjectFolderItem *item) const
{
    Q_UNUSED(item);
    return QList<KDevelop::ProjectTargetItem *>();
}

bool KDevKernelPlugin::addFilesToTarget(const QList<KDevelop::ProjectFileItem *> &files, KDevelop::ProjectTargetItem *target)
{
    Q_UNUSED(files);
    Q_UNUSED(target);
    return false;
}

bool KDevKernelPlugin::removeFilesFromTargets(const QList<KDevelop::ProjectFileItem *> &files)
{
    Q_UNUSED(files);
    return false;
}

bool KDevKernelPlugin::isValid(const KUrl &url, const bool isFolder, KDevelop::IProject *project) const
{
    Q_UNUSED(isFolder)
    static QRegExp Kconf("/Kconfig($|\\.?)");
    qDebug() << "isValid" << url;
    QString lFile(url.toLocalFile());
    // Files in include/ shall always be processed
    if (lFile.startsWith(KUrl(project->folder(), "include/").toLocalFile())) return true;
    // Documentation too
    if (lFile.startsWith(KUrl(project->folder(), "Documentation/").toLocalFile())) return true;
    // Same thing for .h files and Makefiles
    if (lFile.endsWith(".h") || lFile.endsWith("/Makefile")) return true;
    // And KConfig files
    if (lFile.contains(Kconf)) return true;
    return _validFiles[project].contains(url);
}

KUrl KDevKernelPlugin::buildDirectory(KDevelop::ProjectBaseItem *item) const
{
    KUrl buildDir(item->project()->projectItem()->url());
    return buildDir;
}

KJob *KDevKernelPlugin::install(KDevelop::ProjectBaseItem *item)
{
    Q_UNUSED(item)
    return 0;
}

KJob *KDevKernelPlugin::build(KDevelop::ProjectBaseItem *item)
{
    Q_UNUSED(item)
    return 0;
}

KJob *KDevKernelPlugin::clean(KDevelop::ProjectBaseItem *item)
{
    Q_UNUSED(item)
    return 0;
}

KJob *KDevKernelPlugin::configure(KDevelop::IProject *project)
{
    Q_UNUSED(project)
    return 0;
}

KJob *KDevKernelPlugin::prune(KDevelop::IProject *project)
{
    Q_UNUSED(project)
    return 0;
}

#include "kdevkernelplugin.moc"
