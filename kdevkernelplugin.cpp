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

#include "kdevkernelplugin.h"
#include "kdevkernelconfigwidget.h"
#include "kdevkernelconfig.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugin.h>
#include <interfaces/iplugincontroller.h>
#include <make/imakebuilder.h>
#include <project/projectmodel.h>
#include <KPluginFactory>
#include <KLocalizedString>
#include <KAboutData>
#include <QObject>

#include <QFile>
#include <QFileInfo>
#include <QtDebug>

K_PLUGIN_FACTORY(KernelProjectFactory, registerPlugin<KDevKernelPlugin>();)
K_EXPORT_PLUGIN(KernelProjectFactory(
                    KAboutData("kdevkernel", "kdevkernel",
                               ki18n("Linux Kernel"),
                               "0.1",
                               ki18n("Linux Kernel Project Manager"),
                               KAboutData::License_GPL,
                               ki18n("Copyright (C) 2011/2012 Alexandre Courbot <gnurou@gmail.com>"),
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

    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IMakeBuilder");
    if (i)
    {
        _builder = i->extension<IMakeBuilder>();
    }

    connect(core()->projectController(), SIGNAL(projectClosing(KDevelop::IProject *)), this, SLOT(projectClosing(KDevelop::IProject *)));
}

KDevelop::IProjectBuilder *KDevKernelPlugin::builder() const
{
    return (KDevelop::IProjectBuilder *)(this);
}

KUrl::List KDevKernelPlugin::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    return includeDirectories(item->project());
}

KUrl::List KDevKernelPlugin::includeDirectories(KDevelop::IProject *project) const
{
    KUrl::List ret;
    KUrl projectRoot = project->folder();
    KConfigGroup config(project->projectConfiguration()->group(KERN_KGROUP));

    // TODO cache for better efficiency - this should be built when loading a project
    // or when config changes
    ret << KUrl(projectRoot, "include");

    if (config.hasKey(KERN_ARCH)) {
        QString arch(config.readEntry(KERN_ARCH));
        KUrl archUrl(projectRoot, "arch/");
        ret << KUrl(projectRoot, QString("arch/%1/include").arg(arch));
        foreach (const QString & machDir, _machDirs[project]) {
            ret << KUrl(projectRoot, QString("arch/%1/%2/include").arg(arch).arg(machDir));
        }
    }

    // TODO /usr/include and such should not be looked for

    return ret;
}

QHash<QString, QString> KDevKernelPlugin::defines(KDevelop::ProjectBaseItem *item) const
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

void KDevKernelPlugin::parseMakefile(const KUrl &dir, KDevelop::IProject *project) const
{
    static QRegExp objy("([\\w-]+)-([^+:= \t]+)[\t ]*\\+?:?=([^\\\\]+)\\\\?\n");
    static QRegExp repl("\\$\\((\\w_+)\\)");
    static QRegExp spTab("\t| ");
    QFile makefile(KUrl(dir, "Makefile").toLocalFile());
    ValidFilesList &validFiles = _validFiles[project][dir];

    validFiles.lastUpdate = QDateTime::currentDateTime();
    validFiles.validFiles.clear();

    if (!makefile.exists() || !makefile.open(QIODevice::ReadOnly)) return;

    QStringList files;

    while (1) {
        QString line(makefile.readLine());

        if (line.isEmpty()) break;

        if (objy.exactMatch(line)) {
            bool addFiles = false;
            QString y(objy.cap(2));
            y.replace("${", "$(");
            y.replace("}", ")");

            if (y.startsWith("$(") && y.endsWith(")")) {
                QString def(_defines[project][y.mid(2, y.size() - 3)]);

                if (def == "1") y = "y";
            }

            if (y == "y" || y == "objs") addFiles = true;

            // Special handling for machine and plat cases
            // TODO merge common actions
            if (addFiles && (objy.cap(1) == "machine" || objy.cap(1) == "plat")) {
                QStringList pFiles(objy.cap(3).split(spTab, QString::SkipEmptyParts));
                foreach (const QString & pFile, pFiles) {
                    QString pDir((objy.cap(1) == "machine" ? "mach-" : "plat-") + pFile);
                    files += pDir + "/";
                    _machDirs[project] << pDir;
                }
            } else {
                // Get multi-line definitions
                if (addFiles) files += objy.cap(3).split(spTab, QString::SkipEmptyParts);

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
    }

    KConfigGroup config(project->projectConfiguration()->group(KERN_KGROUP));
    QString archDir(QString("arch/%1/").arg(config.readEntry(KERN_ARCH)));
    foreach (QString file, files) {
        if (file.endsWith(".o")) file = file.mid(0, file.size() - 2) + ".c";
        else if (file.endsWith("/")) file = file.left(file.size() - 1);
        // Some directories are specified from the source root in the arch dir
        if (dir.toLocalFile().endsWith(archDir) && file.startsWith(archDir))
             file = file.mid(archDir.size());

        validFiles.validFiles << file;
        qDebug() << "VALID FILE" << dir << file;
    }
}

// TODO cleanup everything when the project closes!
KDevelop::ProjectFolderItem *KDevKernelPlugin::import(KDevelop::IProject *project)
{
    KUrl projectRoot(project->folder());
    KUrl buildRoot;

    // This effectively cleans up everything
    projectClosing(project);

    // Standard definitions
    QHash<QString, QString> &_defs = _defines[project];
    _defs["__KERNEL__"] = "";

    KConfigGroup config(project->projectConfiguration()->group(KERN_KGROUP));

    if (config.hasKey(KERN_BDIR))
        buildRoot = config.readEntry(KERN_BDIR, KUrl());
    else buildRoot = projectRoot;

    buildRoot.adjustPath(KUrl::AddTrailingSlash);
    parseDotConfig(KUrl(buildRoot, ".config"), _defs);

    _validFiles[project].clear();

    ValidFilesList &rootFiles = _validFiles[project][projectRoot];
    rootFiles.lastUpdate = QDateTime::currentDateTime();

    if (config.hasKey(KERN_ARCH)) {
        KUrl archUrl(projectRoot, "arch/");
        rootFiles.validFiles << "arch";
        _validFiles[project][archUrl].lastUpdate = QDateTime::currentDateTime();
        _validFiles[project][archUrl].validFiles << config.readEntry(KERN_ARCH, "");
    }

    /*
     * TODO can't we parse the root Makefile for that?
     */
    rootFiles.validFiles << "init";
    rootFiles.validFiles << "sound";
    rootFiles.validFiles << "net";
    rootFiles.validFiles << "lib";
    rootFiles.validFiles << "usr";
    rootFiles.validFiles << "kernel";
    rootFiles.validFiles << "mm";
    rootFiles.validFiles << "fs";
    rootFiles.validFiles << "ipc";
    rootFiles.validFiles << "security";
    rootFiles.validFiles << "crypto";
    rootFiles.validFiles << "block";
    rootFiles.validFiles << "drivers";

    return AbstractFileManagerPlugin::import(project);
}

void KDevKernelPlugin::projectClosing (KDevelop::IProject *project)
{
    _validFiles.remove(project);
    _machDirs.remove(project);
    _defines.remove(project);
}

KDevelop::ProjectTargetItem *KDevKernelPlugin::createTarget(const QString &target, KDevelop::ProjectFolderItem *parent)
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
    KUrl containingDir(url.directory());
    containingDir.adjustPath(KUrl::AddTrailingSlash);
    QString file(url.fileName());
    const ValidFilesList &validFiles(_validFiles[project][containingDir]);
    bool valid = false;
    static QRegExp Kconf("/Kconfig($|\\.?)");
    QTime curTime(QTime::currentTime());
    QFileInfo mFile(KUrl(containingDir, "Makefile").toLocalFile());
    
    if (mFile.exists() && validFiles.lastUpdate <= mFile.lastModified()) {
        parseMakefile(containingDir, project);
    }

    QString lFile(url.toLocalFile());
    // Files in include directories shall always be processed
    // TODO cache the include dirs list, this is inefficient
    KUrl::List includeDirs(includeDirectories(project));
    foreach (const KUrl & iUrl, includeDirs) {
        if (lFile.startsWith(iUrl.toLocalFile())) {
            valid = true;
            break;
        }
    }

    if (valid);
    // Documentation too
    else if (lFile.startsWith(KUrl(project->folder(), "Documentation/").toLocalFile())) valid = true;
    // Same thing for .h files and Makefiles
    else if (lFile.endsWith(".h") || lFile.endsWith("/Makefile")) valid = true;
    // And KConfig files
    else if (lFile.contains(Kconf)) valid = true;
    else if (validFiles.validFiles.contains(file)) valid = true;

    qDebug() << "isValid" << containingDir << file << valid;
    return valid;
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
    // TODO instead of this, have all, vmlinux, etc... as targets in the files view.
    return jobForTarget(item->project(), "all");
}

KJob *KDevKernelPlugin::clean(KDevelop::ProjectBaseItem *item)
{
    if (_builder) {
        return _builder->clean(item);
    }
    else return 0;
}

KJob *KDevKernelPlugin::configure(KDevelop::IProject *project)
{
    return jobForTarget(project, "xconfig");
}

KJob *KDevKernelPlugin::prune(KDevelop::IProject *project)
{
    if (_builder) {
        return _builder->executeMakeTarget(project->projectItem(), "mrproper");
    }
    else return 0;
}

KJob *KDevKernelPlugin::jobForTarget(KDevelop::IProject *project, const QString &target)
{
    if (_builder) {
        MakeVariables makeVars;
        KConfigGroup config(project->projectConfiguration()->group(KERN_KGROUP));
        if (config.hasKey(KERN_BDIR))
            makeVars << QPair<QString, QString>("O", KUrl(config.readEntry(KERN_BDIR)).toLocalFile());
        if (config.hasKey(KERN_ARCH))
            makeVars << QPair<QString, QString>("ARCH", config.readEntry(KERN_ARCH));
        if (config.hasKey(KERN_CROSS))
            makeVars << QPair<QString, QString>("CROSS_COMPILE", config.readEntry(KERN_CROSS));
        return _builder->executeMakeTargets(project->projectItem(), QStringList(target), makeVars);
    }
    else return 0;
}


#include "kdevkernelplugin.moc"
