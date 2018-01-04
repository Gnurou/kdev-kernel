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

#include "projectconfig.h"

#include <interfaces/iproject.h>

namespace KDevKernel
{

ProjectConfigPage::ProjectConfigPage(KDevelop::IPlugin* plugin, KDevelop::IProject* project, QWidget* parent)
    : ConfigPage(plugin, new ProjectConfig, parent)
    , ui(new Ui::ProjectConfigPage)
{
    configSkeleton()->setSharedConfig(project->projectConfiguration());
    configSkeleton()->load();

    ui->setupUi(this);
}

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

ProjectConfigPage::~ProjectConfigPage() = default;

}
