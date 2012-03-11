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

#include "kcm_kdevkernel.h"

#include <KPluginFactory>
#include <QVBoxLayout>

#include "kdevkernelconfigwidget.h"
#include "kcfg_kdevkernelconfig.h"

K_PLUGIN_FACTORY(KDevKernelKCModuleFactory, registerPlugin<KDevKernelKCModule>(); )
K_EXPORT_PLUGIN(KDevKernelKCModuleFactory("kcm_kdevkernel", "kdevkernel"))

KDevKernelKCModule::KDevKernelKCModule(QWidget *parent, const QVariantList &args)
    : ProjectKCModule<KDevKernelConfig>(KDevKernelKCModuleFactory::componentData(), parent, args)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    configWidget = new KDevKernelConfigWidget(this);
    connect(configWidget, SIGNAL(changed()), SLOT(dataChanged()));
    layout->addWidget(configWidget);

    addConfig(KDevKernelConfig::self(), configWidget);
}

void KDevKernelKCModule::dataChanged()
{
    emit changed(true);
}

KDevKernelKCModule::~KDevKernelKCModule()
{
}

void KDevKernelKCModule::defaults()
{
    KCModule::defaults();
}

void KDevKernelKCModule::save()
{
    KCModule::save();
}

void KDevKernelKCModule::load()
{
    KCModule::load();
}

#include "kcm_kdevkernel.moc"
