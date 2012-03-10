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
#include "kdevkernelsettings.h"

#include <KPluginFactory>
#include <QVBoxLayout>

#include "kcfg_kdevkernelconfig.h"

K_PLUGIN_FACTORY(KDevKernelKCModuleFactory, registerPlugin<KDevKernelKCModule>(); )
K_EXPORT_PLUGIN(KDevKernelKCModuleFactory("kcm_kdevkernel", "kdevkernel"))

KDevKernelKCModule::KDevKernelKCModule(QWidget *parent, const QVariantList &args)
    : ProjectKCModule<KDevKernelSettings>(KDevKernelKCModuleFactory::componentData(), parent, args)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    addConfig(KDevKernelSettings::self(), configWidget);
}
