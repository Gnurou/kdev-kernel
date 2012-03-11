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

#include <KJob>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <language/backgroundparser/parseprojectjob.h>

KDevKernelConfigWidget::KDevKernelConfigWidget(QWidget* parent) : QWidget(parent)
{
	Ui::KDevKernelConfigWidget::setupUi(this);
	connect(buildDir, SIGNAL(textChanged(const QString &)), this, SIGNAL(changed()));
	connect(arch, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
	connect(crossCompiler, SIGNAL(textChanged(const QString &)), this, SIGNAL(changed()));
}

void KDevKernelConfigWidget::loadDefaults()
{
}

static const QString KGROUP("Kernel");
static const QString KBDIR("buildDir");
static const QString KARCH("arch");
static const QString KCROSS("crossPrefix");

void KDevKernelConfigWidget::loadFrom(KConfig* config)
{
    KConfigGroup group(config->group(KGROUP));
    if (group.hasKey(KBDIR)) {
	    buildDir->setUrl(group.readEntry(KBDIR, KUrl()));
    }
    if (group.hasKey(KARCH)) {
	    QString archStr(group.readEntry(KARCH, ""));
	    arch->setCurrentItem(archStr);
    }
    if (group.hasKey(KCROSS)) {
	    crossCompiler->setUrl(KUrl(group.readEntry(KCROSS, "") + "gcc"));
    }
}

void KDevKernelConfigWidget::saveTo(KConfig* config, KDevelop::IProject* project)
{
    KConfigGroup group(config->group(KGROUP));
    if (!buildDir->url().isEmpty())
	    group.writeEntry(KBDIR, buildDir->url());
    else group.deleteEntry(KBDIR);
    if (arch->currentIndex() != 0)
	    group.writeEntry(KARCH, arch->currentText());
    else group.deleteEntry(KARCH);
    if (!crossCompiler->url().isEmpty()) {
	    QString cc(crossCompiler->url().toLocalFile());
	    cc.remove("file://");
	    if (cc.endsWith("gcc")) {
		    QString crossPrefix(cc.mid(0, cc.size() - 3));
		    group.writeEntry(KCROSS, crossPrefix);
	    } else {
		    // TODO notify error
	    }
    }
    else group.deleteEntry(KCROSS);

    config->sync();
    if ( KDevelop::IProjectController::parseAllProjectSources()) {
        KJob* parseProjectJob = new KDevelop::ParseProjectJob(project);
        KDevelop::ICore::self()->runController()->registerJob(parseProjectJob);
    }
}
