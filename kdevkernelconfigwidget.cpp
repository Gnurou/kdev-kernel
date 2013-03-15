/*
 *  Copyright (C) 2012 Alexandre Courbot <gnurou@gmail.com>
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

#include "kdevkernelconfigwidget.h"
#include "kdevkernelconfig.h"

#include <QtDebug>
#include <QDir>
#include <KJob>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <language/backgroundparser/parseprojectjob.h>

KDevKernelConfigWidget::KDevKernelConfigWidget(QWidget *parent, const QString &projectRoot) : QWidget(parent), _projectRoot(projectRoot)
{
    Ui::KDevKernelConfigWidget::setupUi(this);

    connect(buildDir, SIGNAL(textChanged(const QString &)), this, SIGNAL(changed()));
    connect(arch, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
    connect(defconfig, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
    connect(arch, SIGNAL(currentIndexChanged(QString)), this, SLOT(archChanged(QString)));
    connect(crossCompiler, SIGNAL(textChanged(const QString &)), this, SIGNAL(changed()));
}

void KDevKernelConfigWidget::loadDefaults()
{
}

void KDevKernelConfigWidget::loadFrom(KConfig *config)
{
    KConfigGroup group(config->group(KERN_KGROUP));

    // Fill in the arch values
    KUrl pRoot(_projectRoot);
    pRoot.adjustPath(KUrl::AddTrailingSlash);
    QDir archDir(KUrl(pRoot, "arch").toLocalFile());
    archDir.setFilter(QDir::Dirs);
    foreach (const QString &archEntry, archDir.entryList()) {
        if (archEntry.startsWith('.')) continue;
        arch->addItem(archEntry);
    }

    if (group.hasKey(KERN_BDIR)) {
        buildDir->setUrl(group.readEntry(KERN_BDIR, KUrl()));
    } else {
        buildDir->setStartDir(pRoot);
    }

    if (group.hasKey(KERN_ARCH)) {
        arch->setCurrentItem(group.readEntry(KERN_ARCH, ""));
    }

    if (group.hasKey(KERN_DEFCONFIG)) {
        defconfig->setCurrentItem(group.readEntry(KERN_DEFCONFIG, ""));
    }

    if (group.hasKey(KERN_CROSS)) {
        crossCompiler->setUrl(KUrl(group.readEntry(KERN_CROSS, "") + "gcc"));
    } else {
        crossCompiler->setStartDir(KUrl("/usr/bin/"));
    }
}

void KDevKernelConfigWidget::saveTo(KConfig *config, KDevelop::IProject *project)
{
    KConfigGroup group(config->group(KERN_KGROUP));

    if (!buildDir->url().isEmpty())
        group.writeEntry(KERN_BDIR, buildDir->url());
    else group.deleteEntry(KERN_BDIR);

    group.writeEntry(KERN_ARCH, arch->currentText());

    if (!crossCompiler->url().isEmpty()) {
        QString cc(crossCompiler->url().toLocalFile());
        cc.remove("file://");

        if (cc.endsWith("gcc")) {
            QString crossPrefix(cc.mid(0, cc.size() - 3));
            group.writeEntry(KERN_CROSS, crossPrefix);
        } else {
            // TODO notify error
        }
    } else group.deleteEntry(KERN_CROSS);

    // Remove the .config file if configuration changed. This will trigger
    // the corresponding make rule from the plugin the next time we parse.
    if (defconfig->currentText() != group.readEntry(KERN_DEFCONFIG, "")) {
        KUrl buildDir(group.readEntry(KERN_BDIR, _projectRoot));
        buildDir.adjustPath(KUrl::AddTrailingSlash);
        QFile dotConfig(KUrl(buildDir, ".config").toLocalFile());
        if (dotConfig.exists()) dotConfig.remove();
    }

    group.writeEntry(KERN_DEFCONFIG, defconfig->currentText());

    config->sync();

    // For initial setup we get called with a null project. In this case KDev will
    // do the parsing automatically anyway.
    /*if (KDevelop::IProjectController::parseAllProjectSources() && project) {
        KJob *parseProjectJob = new KDevelop::ParseProjectJob(project);
        KDevelop::ICore::self()->runController()->registerJob(parseProjectJob);
    }*/
}

void KDevKernelConfigWidget::archChanged (const QString &arch)
{
    defconfig->clear();

    // Fill in the configs values
    KUrl pRoot(_projectRoot);
    pRoot.adjustPath(KUrl::AddTrailingSlash);
    QDir configDirs(KUrl(pRoot, QString("arch/%1/configs").arg(arch)).toLocalFile());
    // Search only simple files
    foreach (const QString &configFile, configDirs.entryList(QDir::Files)) {
        if (configFile.startsWith('.')) continue;
        defconfig->addItem(configFile.left(configFile.size() - QString("_defconfig").size()));
    }
    // Search also in subdirectory
    foreach (const QString &configFile, configDirs.entryList(QDir::Dirs)) {
        if (configFile.startsWith('.')) continue;
        QDir configSubDirs(KUrl(pRoot, QString("arch/%1/configs/%2").arg(arch).arg(configFile)).toLocalFile());
        foreach (const QString &configSubFile, configSubDirs.entryList(QDir::Files)) {
            if (configSubFile.startsWith('.')) continue;
            QString defconf = configFile + "/" + configSubFile;
            defconfig->addItem(defconf.left(defconf.size() - QString("_defconfig").size()));
        }
    }
}
