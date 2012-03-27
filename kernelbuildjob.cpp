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

#include "kernelbuildjob.h"
#include "kdevkernelplugin.h"
#include "kdevkernelconfig.h"

#include <KLocale>
#include <outputview/outputmodel.h>
#include <util/commandexecutor.h>
#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <KConfigGroup>

KernelBuildJob::KernelBuildJob(KDevelop::ProjectBaseItem *item, KernelBuildJob::ActionType t) : OutputJob(), type(t)
{
    setCapabilities(Killable);
    project = item->project();
    setTitle(QString("%1 %2").arg("make").arg(item->text()));
    setObjectName(QString("%1 %2").arg("make").arg(item->text()));
}

void KernelBuildJob::start()
{
    setStandardToolView(KDevelop::IOutputView::BuildView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    KDevelop::OutputModel* model = new KDevelop::OutputModel(this);
    setModel(model, KDevelop::IOutputView::TakeOwnership);
    startOutput();

    exec = new KDevelop::CommandExecutor("make", this);
    QStringList args;
    KConfigGroup config(project->projectConfiguration()->group(KERN_KGROUP));

    args << QString("ARCH=%1").arg(config.readEntry(KERN_ARCH, "x86"));
    if (config.hasKey(KERN_CROSS)) {
        args << QString("CROSS_COMPILE=%1").arg(config.readEntry(KERN_CROSS, ""));
    }
    if (config.hasKey(KERN_BDIR)) {
        args << QString("O=%1").arg(KUrl(config.readEntry(KERN_BDIR, "")).toLocalFile());
    }

    switch (type) {
    case SelectConfig:
        break;
    case Build:
        break;
    case Configure:
        args << "xconfig";
        break;
    }

    exec->setArguments(args);
    exec->setWorkingDirectory(project->projectItem()->url().toLocalFile());

    connect(exec, SIGNAL(completed()), SLOT(onFinished()));
    connect(exec, SIGNAL(failed(QProcess::ProcessError)), SLOT(onError(QProcess::ProcessError)));

    connect(exec, SIGNAL(receivedStandardOutput(QStringList)), model, SLOT(appendLines(QStringList)));
    connect(exec, SIGNAL(receivedStandardError(QStringList)), model, SLOT(appendLines(QStringList)));

    model->appendLine(QString("%1 %2").arg("make").arg(args.join(" ")));
    exec->start();
}

bool KernelBuildJob::doKill()
{
    exec->kill();
    return true;
}

KDevelop::OutputModel* KernelBuildJob::model()
{
    return qobject_cast<KDevelop::OutputModel *>(OutputJob::model());
}

void KernelBuildJob::onFinished()
{
    model()->appendLine(i18n("*** Build successful ***"));
    emitResult();
}

void KernelBuildJob::onError(QProcess::ProcessError error)
{
    setError(error);

    switch (error) {
        case QProcess::FailedToStart:
            setError(QProcess::FailedToStart);
            setErrorText(i18n("Failed to start command"));
            break;
        case QProcess::Crashed:
            setError(QProcess::Crashed);
            setErrorText(i18n("Command crashed"));
            break;
        default:
            setError(QProcess::UnknownError);
            setErrorText(i18n("Unknown error"));
            break;
    }
    emitResult();
}

#include "kernelbuildjob.h"
