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
#include "kdevkernelconfig.h"

#include "config/projectconfigpage.h"

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugin.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <project/projectconfigpage.h>

#include <KPluginFactory>
#include <KLocalizedString>
#include <KAboutData>
#include <KConfigGroup>
#include <KProcess>

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QtDebug>

K_PLUGIN_FACTORY_WITH_JSON(KernelProjectFactory, "kdevkernel.json", registerPlugin<KDevKernelPlugin>();)
/*K_EXPORT_PLUGIN(KernelProjectFactory(
                    KAboutData("kdevkernel", "kdevkernel",
                               ki18n("Linux Kernel"),
                               "0.1",
                               ki18n("Linux Kernel Project Manager"),
                               KAboutData::License_GPL,
                               ki18n("Copyright (C) 2011-2013 Alexandre Courbot <gnurou@gmail.com>"),
                               KLocalizedString(),
                               "",
                               "gnurou@gmail.com"
                              )
                ))*/

KDevKernelPlugin::KDevKernelPlugin(QObject *parent, const QVariantList &args)
    : KDevelop::AbstractFileManagerPlugin(QStringLiteral("kdevkernel"), parent, args)
{
    Q_UNUSED(args);
    /*KDEV_USE_EXTENSION_INTERFACE(KDevelop::IBuildSystemManager)
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IProjectFileManager)
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IProjectBuilder)*/

    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IMakeBuilder");
    if (i) {
        _builder = i->extension<IMakeBuilder>();
    }

    connect(core()->projectController(), SIGNAL(projectClosing(KDevelop::IProject *)), this, SLOT(projectClosing(KDevelop::IProject *)));
}

KDevelop::IProjectBuilder* KDevKernelPlugin::builder() const
{
    return (KDevelop::IProjectBuilder *)(this);
}

KDevelop::Path::List KDevKernelPlugin::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    return includeDirectories(item->project());
}

KDevelop::Path::List KDevKernelPlugin::includeDirectories(KDevelop::IProject *project) const
{
    KDevelop::Path::List ret;
    KDevelop::Path projectRoot = project->path();
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, KERN_KGROUP);
    KDevelop::Path bDir(cg.readEntry(KERN_BDIR, projectRoot.toUrl()));
//     bDir.adjustPath(QUrl::AddTrailingSlash);

    // TODO cache for better efficiency - this should be built when loading a project
    // or when config changes
    ret << KDevelop::Path(projectRoot, "/include");
    if (bDir != projectRoot)
        ret << KDevelop::Path(bDir, "include");

    if (cg.hasKey(KERN_ARCH)) {
        QString arch(cg.readEntry(KERN_ARCH));
        KDevelop::Path archUrl(projectRoot, "arch/");
        ret << KDevelop::Path(projectRoot, QString("arch/%1/include").arg(arch));
        foreach (const QString & machDir, _machDirs[project]) {
            ret << KDevelop::Path(projectRoot, QString("arch/%1/%2/include").arg(arch).arg(machDir));
        }

        // Build-specific generated includes
        ret << KDevelop::Path(bDir, QString("arch/%1/include/generated").arg(arch));
    }

    // TODO /usr/include and such should not be looked for

    return ret;
}

KDevelop::Path::List KDevKernelPlugin::frameworkDirectories(KDevelop::ProjectBaseItem *item) const
{
    Q_UNUSED(item)
    return {};
}

QHash<QString, QString> KDevKernelPlugin::defines(KDevelop::ProjectBaseItem *item) const
{
    return _defines[item->project()];
}

void KDevKernelPlugin::parseDotConfig(KDevelop::IProject *project, const KDevelop::Path &dotconfig, QHash<QString, QString> &_defs)
{
    QFile dfile(dotconfig.toLocalFile());
    static QRegExp def("(\\w+)=(\"?[^\\n]+\"?)\n?");

#ifdef DEBUG
    qDebug() << "kernel dotconfig" << dotconfig;
#endif

    // If the .config file does not exist, create it by invoking make.
    if (!dfile.exists()) {
        KSharedConfigPtr config = KSharedConfig::openConfig();
        KConfigGroup cg(config, KERN_KGROUP);
        QString conf(cg.readEntry(KERN_DEFCONFIG, ""));
        if (!conf.isEmpty()) {
            MakeVariables makeVars(makeVarsForProject(project));
            QStringList vars;
            MakeVariables::const_iterator it = makeVars.constBegin();
            while (it != makeVars.constEnd()) {
                vars += QString("%1=%2").arg(it->first).arg(it->second);
                ++it;
            }
            vars += conf + "_defconfig";
            KProcess *process = new KProcess();
            process->setWorkingDirectory(project->path().toLocalFile());
            process->setProgram("make", vars);
            process->execute();
            delete process;
        }
    }

    if (!dfile.open(QIODevice::ReadOnly)) return;

    while (1) {
        QString line(dfile.readLine());

        if (line.isEmpty()) break;

        if (!def.exactMatch(line)) continue;

        QString key(def.cap(1));
        QString val(def.cap(2));

        if (val == "y") val = "1";
        else if (val == "n") val = "0";
        else if (val.startsWith('"') && val.endsWith('"')) val = val.mid(1, val.size() - 2);

#ifdef DEBUG
        qDebug() << "kernel def:" << key << val;
#endif
        _defs[key] = val;
    }
}

// TODO This is crap. Valid files should be stored in the project directly, and every
// directory/file with includes should have a list of the valid files it added, and which
// get removed from the global list as we reparse the Makefile/source file.
void KDevKernelPlugin::parseMakefile(const KDevelop::Path &dir, KDevelop::IProject *project) const
{
    static QRegExp objy("([\\w-]+)-([^+:= \t]*)[\t ]*\\+?:?=([^\\\\]+)\\\\?\n");
    static QRegExp repl("\\$\\((\\w_+)\\)");
    static QRegExp spTab("\t| ");
    QFile makefile(KDevelop::Path(dir, "Makefile").toLocalFile());
    ValidFilesList &validFiles = _validFiles[project][dir];

    validFiles.lastUpdate = QDateTime::currentDateTime();
    //validFiles.validFiles.clear();

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

            if (y == "y" || y == "objs" || y == "") addFiles = true;

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

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, KERN_KGROUP);
    QString archDir(QString("arch/%1/").arg(cg.readEntry(KERN_ARCH)));
    foreach (QString file, files) {
        if (file.endsWith(".o")) file = file.mid(0, file.size() - 2) + ".c";
        else if (file.endsWith(".dtb")) file = file.mid(0, file.size() - 4) + ".dts";
        else if (file.endsWith("/")) file = file.left(file.size() - 1);
        // Some directories are specified from the source root in the arch dir
        if (dir.toLocalFile().endsWith(archDir) && file.startsWith(archDir))
            file = file.mid(archDir.size());

        // Sometimes files are referenced that are several directories below
        if (file.contains('/')) {
            KDevelop::Path nFile(KDevelop::Path(dir, file).toUrl());
            KDevelop::Path nDir(nFile.parent());
            // Add all the subdirectories
            KDevelop::Path nDir2(nDir.toUrl());
            while (nDir2 != dir) {
                QString f(nDir2.toUrl().fileName());
                KDevelop::Path d(nDir2.parent());
                _validFiles[project][d].validFiles << f;
                nDir2 = KDevelop::Path(d);
            }
//             nDir.adjustPath(KUrl::AddTrailingSlash);
            _validFiles[project][nDir].validFiles << nFile.toUrl().fileName();
        }
        validFiles.validFiles << file;
#ifdef DEBUG
        qDebug() << "VALID FILE" << dir << file;
#endif
    }
}

KDevelop::ProjectFolderItem *KDevKernelPlugin::import(KDevelop::IProject *project)
{
    KDevelop::Path projectRoot(project->path());
//     projectRoot.adjustPath(KUrl::AddTrailingSlash);
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, KERN_KGROUP);
    KDevelop::Path buildRoot(cg.readEntry(KERN_BDIR, projectRoot.toUrl()));
//     buildRoot.adjustPath(KUrl::AddTrailingSlash);

    // This effectively cleans up everything
    projectClosing(project);

    // Force language to "C"
    project->projectConfiguration()->group("Project").writeEntry("Language", "C");
    // Force disabling of make-based include path resolver
    project->projectConfiguration()->group("MakeBuilder").writeEntry("Resolve Using Make", false);

    // If no .config file in the build directory, force user to configure to choose one
    if (!QFile(KDevelop::Path(buildRoot, ".config").toLocalFile()).exists()) {
        // Populate a drop down list with all the _defconfig options, plus one "no change"
        // if there is already a .config file in the build dir. When dialog closes and a
        // defconfig is selected, run make to create it in the build dir.
        //KDevelop::ICore::self()->projectController()->configureProject(project);
    }

    // Standard definitions
    QHash<QString, QString> &_defs = _defines[project];
    _defs["__KERNEL__"] = "";

    if (cg.hasKey(KERN_BDIR))
        buildRoot = KDevelop::Path(cg.readEntry(KERN_BDIR, KDevelop::Path().toUrl()));
    else buildRoot = projectRoot;

//     buildRoot.adjustPath(KUrl::AddTrailingSlash);
    parseDotConfig(project, KDevelop::Path(buildRoot, ".config"), _defs);

    _validFiles[project].clear();

    ValidFilesList &rootFiles = _validFiles[project][projectRoot];
    rootFiles.lastUpdate = QDateTime::currentDateTime();

    if (cg.hasKey(KERN_ARCH)) {
        KDevelop::Path archUrl(projectRoot, "arch/");
        QString arch(cg.readEntry(KERN_ARCH, ""));
        KDevelop::Path archArchUrl(archUrl, arch);
// 	archArchUrl.adjustPath(KUrl::AddTrailingSlash);
        rootFiles.validFiles << "arch";
        _validFiles[project][archUrl].lastUpdate = QDateTime::currentDateTime();
        _validFiles[project][archUrl].validFiles << arch;
        _validFiles[project][archArchUrl].validFiles << "boot";
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

bool KDevKernelPlugin::hasBuildInfo(KDevelop::ProjectBaseItem *item) const
{
    Q_UNUSED(item)
    return false;
}

bool KDevKernelPlugin::isValid(const KDevelop::Path &url, const bool isFolder, KDevelop::IProject *project) const
{
    Q_UNUSED(isFolder)
    KDevelop::Path containingDir(url);
//     containingDir.adjustPath(KUrl::AddTrailingSlash);
    QString file(url.toUrl().fileName());
    const ValidFilesList &validFiles(_validFiles[project][containingDir]);
    bool valid = false;
    static QRegExp Kconf("/Kconfig($|\\.?)");
    QTime curTime(QTime::currentTime());
    QFileInfo mFile(KDevelop::Path(containingDir, "Makefile").toLocalFile());

    if (mFile.exists() && validFiles.lastUpdate <= mFile.lastModified()) {
        parseMakefile(containingDir, project);
    }

    QString lFile(url.toUrl().toLocalFile());
    // Files in include directories shall always be processed
    // TODO cache the include dirs list, this is inefficient
    KDevelop::Path::List includeDirs(includeDirectories(project));
    foreach (const KDevelop::Path & iUrl, includeDirs) {
        if (lFile.startsWith(iUrl.toLocalFile())) {
            valid = true;
            break;
        }
    }

    if (valid);
    // Documentation too
    else if (lFile.startsWith(KDevelop::Path(project->path(), "Documentation/").toLocalFile())) valid = true;
    // Same thing for .h files and Makefiles
    else if (lFile.endsWith(".h") || lFile.endsWith("/Makefile")) valid = true;
    // And KConfig files
    else if (lFile.contains(Kconf)) valid = true;
    else if (validFiles.validFiles.contains(file)) valid = true;
    // Last ressort, the user-list of hardcoded files to accept
    else {
        KSharedConfigPtr config = KSharedConfig::openConfig();
        KConfigGroup cg(config, KERN_KGROUP);
        QStringList vFiles(cg.readEntry(KERN_VALIDFILES, QStringList()));
        KDevelop::Path pRoot(project->path());
//         pRoot.adjustPath(KUrl::AddTrailingSlash);
        QString fPath(url.toLocalFile().mid(pRoot.toLocalFile().size()));
        if (vFiles.contains(fPath)) valid = true;
    }

#ifdef DEBUG
    qDebug() << "isValid" << containingDir << file << valid;
#endif
    return valid;
}

KDevelop::Path KDevKernelPlugin::buildDirectory(KDevelop::ProjectBaseItem *item) const
{
    KDevelop::Path buildDir(item->project()->projectItem()->path());
    return KDevelop::Path(buildDir);
}

QString KDevKernelPlugin::extraArguments(KDevelop::ProjectBaseItem *item) const
{
    Q_UNUSED(item)
    return {};
}

KJob *KDevKernelPlugin::install(KDevelop::ProjectBaseItem *item, const QUrl &specificPrefix)
{
    Q_UNUSED(item)
    Q_UNUSED(specificPrefix)
    return 0;
}

KJob *KDevKernelPlugin::build(KDevelop::ProjectBaseItem *item)
{
    KConfigGroup makeConfig(item->project()->projectConfiguration()->group("MakeBuilder"));

    // TODO instead of this, have all, vmlinux, etc... as targets in the files view.
    return jobForTarget(item->project(), makeConfig.readEntry("Default Target").split(" ", QString::SkipEmptyParts));
}

KJob *KDevKernelPlugin::clean(KDevelop::ProjectBaseItem *item)
{
    return jobForTarget(item->project(), QStringList("clean"));
}

KJob *KDevKernelPlugin::configure(KDevelop::IProject *project)
{
    return jobForTarget(project, QStringList("xconfig"));
}

KJob *KDevKernelPlugin::prune(KDevelop::IProject *project)
{
    return jobForTarget(project, QStringList("mrproper"));
}

KJob *KDevKernelPlugin::createDotConfig (KDevelop::IProject *project)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, KERN_KGROUP);
    QString defConfig(cg.readEntry(KERN_DEFCONFIG, ""));
    if (defConfig.isEmpty()) return 0;
    return jobForTarget(project, QStringList(defConfig + "_defconfig"));
}

MakeVariables KDevKernelPlugin::makeVarsForProject(KDevelop::IProject* project)
{
    Q_UNUSED(project)
    MakeVariables makeVars;
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, KERN_KGROUP);
    if (cg.hasKey(KERN_BDIR))
        makeVars << QPair<QString, QString>("O", KDevelop::Path(cg.readEntry(KERN_BDIR)).toLocalFile());
    if (cg.hasKey(KERN_ARCH))
        makeVars << QPair<QString, QString>("ARCH", cg.readEntry(KERN_ARCH));
    if (cg.hasKey(KERN_CROSS))
        makeVars << QPair<QString, QString>("CROSS_COMPILE", cg.readEntry(KERN_CROSS));

    return makeVars;
}

KJob *KDevKernelPlugin::jobForTarget(KDevelop::IProject *project, const QStringList &targets)
{
    if (_builder) {
        return _builder->executeMakeTargets(project->projectItem(),
                                            targets, makeVarsForProject(project));
    } else return 0;
}

QList<KDevelop::IProjectBuilder *> KDevKernelPlugin::additionalBuilderPlugins(KDevelop::IProject *project) const
{
    Q_UNUSED(project);

    QList<KDevelop::IProjectBuilder *> ret;
    ret << _builder;
    return ret;
}

int KDevKernelPlugin::perProjectConfigPages() const
{
    return 1; // TODO 1 or 0 ? It shows up with 1 when in a KDevKernel project, with 0 it doesn't
}

KDevelop::ConfigPage* KDevKernelPlugin::perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
{
    if (number != 0) {
        return nullptr;
    }

    return new KDevKernel::ProjectConfigPage(this, options.project, parent);
}

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "kdevkernelplugin.moc"
