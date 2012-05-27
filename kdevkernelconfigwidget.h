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

#ifndef KDEVKERNELCONFIGWIDGET_H
#define KDEVKERNELCONFIGWIDGET_H

#include <kconfiggroup.h>
#include "ui_kdevkernelconfigwidget.h"

#include <QWidget>
#include <QString>

class KConfig;

namespace KDevelop
{
class IProject;
}

class KDevKernelConfigWidget : public QWidget, public Ui::KDevKernelConfigWidget
{
    Q_OBJECT
public:
    KDevKernelConfigWidget(QWidget *parent, const QString &projectRoot);
    void loadFrom(KConfig *config);
    void saveTo(KConfig *config, KDevelop::IProject *project);
    void loadDefaults();

signals:
    void changed();

private slots:
    void archChanged(const QString &arch);

private:
    const QString _projectRoot;
};

#endif
