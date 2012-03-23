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

#include <KLocale>
#include <outputview/outputmodel.h>
#include <util/commandexecutor.h>

KernelBuildJob::KernelBuildJob(KDevKernelPlugin *plugin) : OutputJob(plugin)
{
}

void KernelBuildJob::start()
{
    exec = new KDevelop::CommandExecutor("make", this);
    //exec->setArguments();
    //exec->setEnvironment();
    //exec->setWorkingDirectory();

    connect(exec, SIGNAL(completed()), SLOT(onFinished()));
    connect(exec, SIGNAL(failed(QProcess::ProcessError)), SLOT(onError(QProcess::ProcessError)));
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
            setErrorText(i18n("Failed to start command"));
            break;
        case QProcess::Crashed:
            setErrorText(i18n("Command crashed"));
            break;
        default:
            setErrorText(i18n("Unknown error"));
            break;
    }

    emitResult();
}

#include "kernelbuildjob.h"
