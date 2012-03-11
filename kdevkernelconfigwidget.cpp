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

KDevKernelConfigWidget::KDevKernelConfigWidget(QWidget* parent) : QWidget(parent)
{
	Ui::KDevKernelConfigWidget::setupUi(this);
}

void KDevKernelConfigWidget::loadDefaults()
{

}

void KDevKernelConfigWidget::loadFrom(KConfig* config)
{

}

void KDevKernelConfigWidget::saveConfig(KConfigGroup& group, KDevKernelConfig& config)
{

}

void KDevKernelConfigWidget::saveTo(KConfig* config, KDevelop::IProject* project)
{

}
