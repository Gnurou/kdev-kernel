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

#include "projectconfigpage.h"
#include "ui_projectconfigpage.h"

#include "../kdevkernelconfig.h"

#include "projectconfig.h"

#include <interfaces/iproject.h>
#include <util/path.h>

namespace KDevKernel
{

ProjectConfigPage::ProjectConfigPage(KDevelop::IPlugin* plugin, KDevelop::IProject* project, QWidget* parent)
    : ConfigPage(plugin, new ProjectConfig, parent)
    , ui(new Ui::ProjectConfigPage)
{
    this->project = project;

    configSkeleton()->setSharedConfig(project->projectConfiguration());
    configSkeleton()->load();

    ui->setupUi(this);

    connect(ui->kcfg_arch, QOverload<const QString &>::of(&KComboBox::currentIndexChanged), this, &ProjectConfigPage::archChanged);
}

ProjectConfigPage::~ProjectConfigPage() = default;

QString ProjectConfigPage::name() const
{
    return i18n("kdev-kernel");
}

QString ProjectConfigPage::fullName() const
{
    return i18n("Linux Kernel Project Manager");
}

QIcon ProjectConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("kdevkernel"));
}

void ProjectConfigPage::apply()
{
    KConfigGroup cg = configSkeleton()->sharedConfig()->group(KERN_KGROUP);
    KDevelop::Path projectRoot = project->path();

    if (!ui->kcfg_buildDir->url().isEmpty())
        cg.writeEntry(KERN_BDIR, ui->kcfg_buildDir->url());
    else cg.deleteEntry(KERN_BDIR);

    if (!ui->kcfg_crossCompiler->url().isEmpty()) {
        QString cc(ui->kcfg_crossCompiler->url().toLocalFile());
        cc.remove("file://");

        if (cc.endsWith("gcc")) {
            QString crossPrefix(cc.mid(0, cc.size() - 3));
            cg.writeEntry(KERN_CROSS, crossPrefix);
        } else {
            // TODO notify error
        }
    } else cg.deleteEntry(KERN_CROSS);

    // Remove the .config file if configuration changed. This will trigger
    // the corresponding make rule from the plugin the next time we parse.
    if (ui->kcfg_defconfig->currentText() != cg.readEntry(KERN_DEFCONFIG, "")) {
        KDevelop::Path buildDir(cg.readEntry(KERN_BDIR, projectRoot.toUrl()));
//         buildDir.adjustPath(KUrl::AddTrailingSlash);
        QFile dotConfig(KDevelop::Path(buildDir, ".config").toLocalFile());
        if (dotConfig.exists()) dotConfig.remove();
    }

    cg.writeEntry(KERN_DEFCONFIG, ui->kcfg_defconfig->currentText());

    ConfigPage::apply();
}

void ProjectConfigPage::reset()
{
    KConfigGroup cg = configSkeleton()->sharedConfig()->group(KERN_KGROUP);

    // Fill in the arch values
    KDevelop::Path projectRoot = project->path();
    QDir archDir(KDevelop::Path(projectRoot, "arch").toLocalFile());
    archDir.setFilter(QDir::Dirs);
    foreach (const QString &archEntry, archDir.entryList()) {
        if (archEntry.startsWith('.')) continue;
        ui->kcfg_arch->addItem(archEntry);
    }

    ui->kcfg_buildDir->setStartDir(projectRoot.toUrl());
    ui->kcfg_crossCompiler->setStartDir(QUrl("/usr/bin/"));

    ConfigPage::reset();
}

void ProjectConfigPage::archChanged(const QString &arch)
{
    ui->kcfg_defconfig->clear();

    QUrl projectRoot = project->path().toUrl();
    projectRoot.setPath(projectRoot.path() + "/");
    QDir configDirs(projectRoot.resolved(QUrl(QString("arch/%1/configs").arg(arch))).toLocalFile());
    foreach (const QString &configFile, configDirs.entryList()) {
        if (configFile.startsWith('.')) continue;
        ui->kcfg_defconfig->addItem(configFile.left(configFile.size() - QString("_defconfig").size()));
    }
}

}
