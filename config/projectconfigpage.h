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

#ifndef KDEVKERNEL_PROJECTCONFIGPAGE_H
#define KDEVKERNEL_PROJECTCONFIGPAGE_H

#include "interfaces/configpage.h"

namespace KDevelop
{
class IProject;
class Path;
}

namespace KDevKernel
{

namespace Ui
{
class ProjectConfigPage;
}

class ProjectConfigPage : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    ProjectConfigPage(KDevelop::IPlugin* plugin, KDevelop::IProject* project, QWidget* parent);
    ~ProjectConfigPage() override;

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    void apply() override;
    void reset() override;

private Q_SLOTS:
    void archChanged(const QString &arch);

private:
    QScopedPointer<Ui::ProjectConfigPage> ui;
    KDevelop::IProject* project;
};

}

#endif // KDEVKERNEL_PROJECTCONFIGPAGE_H
