/***************************************************************************
 *   Copyright (c) 2002 Jürgen Riegel <juergen.riegel@web.de>              *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"
#ifndef _PreComp_
#endif

#include <App/DocumentObject.h>
#include <App/GroupExtension.h>
#include "Application.h"
#include "CommandT.h"
#include "DockWindowManager.h"
#include "Document.h"
#include "PythonConsole.h"
#include "Selection.h"
#include "SelectionObject.h"
#include "ViewProvider.h"
#include "ViewProviderDocumentObject.h"
#include "ViewProviderLink.h"

using namespace Gui;



//===========================================================================
// Std_Recompute
//===========================================================================

DEF_STD_CMD(StdCmdFeatRecompute)

StdCmdFeatRecompute::StdCmdFeatRecompute()
  :Command("Std_Recompute")
{
    // setting the
    sGroup        = "File";
    sMenuText     = QT_TR_NOOP("&Recompute");
    sToolTipText  = QT_TR_NOOP("Recompute feature or document");
    sWhatsThis    = "Std_Recompute";
    sStatusTip    = QT_TR_NOOP("Recompute feature or document");
    sPixmap       = "view-refresh";
    sAccel        = "Ctrl+R";
}

void StdCmdFeatRecompute::activated(int iMsg)
{
    Q_UNUSED(iMsg);
}

//===========================================================================
// Std_RandomColor
//===========================================================================

DEF_STD_CMD_A(StdCmdRandomColor)

StdCmdRandomColor::StdCmdRandomColor()
  :Command("Std_RandomColor")
{
    sGroup        = "File";
    sMenuText     = QT_TR_NOOP("Random color");
    sToolTipText  = QT_TR_NOOP("Set each selected object to a randomly-selected color");
    sWhatsThis    = "Std_RandomColor";
    sStatusTip    = QT_TR_NOOP("Set each selected object to a randomly-selected color");
    sPixmap       = "Std_RandomColor";
}

void StdCmdRandomColor::activated(int iMsg)
{
    Q_UNUSED(iMsg);

    auto setRandomColor = [](ViewProvider* view) {
        // NOLINTBEGIN
        auto fMax = (float)RAND_MAX;
        auto fRed = (float)rand()/fMax;
        auto fGrn = (float)rand()/fMax;
        auto fBlu = (float)rand()/fMax;
        // NOLINTEND
        auto objColor = App::Color(fRed, fGrn, fBlu);

        auto vpLink = dynamic_cast<ViewProviderLink*>(view);
        if (vpLink) {
            if (!vpLink->OverrideMaterial.getValue()) {
                vpLink->OverrideMaterial.setValue(true);
            }
            vpLink->ShapeMaterial.setDiffuseColor(objColor);
        }
        else if (view) {
            if (auto color = dynamic_cast<App::PropertyColor*>(view->getPropertyByName("ShapeColor"))) {
                // get the view provider of the selected object and set the shape color
                color->setValue(objColor);
            }
        }
    };

    // get the complete selection
    std::vector<SelectionSingleton::SelObj> sel = Selection().getCompleteSelection();

    Command::openCommand(QT_TRANSLATE_NOOP("Command", "Set Random Color"));
    for (const auto & it : sel) {
        ViewProvider* view = Application::Instance->getViewProvider(it.pObject);
        setRandomColor(view);

        if (auto grp = it.pObject->getExtension<App::GroupExtension>()) {
            std::vector<App::DocumentObject*> objs = grp->getObjects();
            for (auto obj : objs) {
                ViewProvider* view = Application::Instance->getViewProvider(obj);
                setRandomColor(view);
            }
        }
    }

    Command::commitCommand();
}

bool StdCmdRandomColor::isActive()
{
    return (Gui::Selection().size() != 0);
}

//===========================================================================
// Std_ToggleFreeze
//===========================================================================
DEF_STD_CMD_A(StdCmdToggleFreeze)

StdCmdToggleFreeze::StdCmdToggleFreeze()
    : Command("Std_ToggleFreeze")
{
    sGroup = "File";
    sMenuText = QT_TR_NOOP("Toggle freeze");
    static std::string toolTip = std::string("<p>")
        + QT_TR_NOOP("Toggles freeze sate of the selected objects. A freezed object is not recomputed when its parents change.")
        + "</p>";
    sToolTipText = toolTip.c_str();
    sStatusTip = sToolTipText;
    sWhatsThis = "Std_ToggleFreeze";
    sPixmap = "Std_ToggleFreeze";
    sAccel = "";
    eType = AlterDoc;
}

void StdCmdToggleFreeze::activated(int iMsg)
{
    Q_UNUSED(iMsg);
    getActiveGuiDocument()->openCommand(QT_TRANSLATE_NOOP("Command", "Toggle freeze"));

    std::vector<Gui::SelectionSingleton::SelObj> sels = Gui::Selection().getCompleteSelection();

    for (Gui::SelectionSingleton::SelObj& sel : sels) {
        App::DocumentObject* obj = sel.pObject;
        if (!obj)
            continue;

        if (obj->isFreezed())
            obj->unfreeze();
        else
            obj->freeze();
    }

    getActiveGuiDocument()->commitCommand();
}

bool StdCmdToggleFreeze::isActive()
{
    return (Gui::Selection().size() != 0);
}




//===========================================================================
// Std_SendToPythonConsole
//===========================================================================

DEF_STD_CMD_A(StdCmdSendToPythonConsole)

StdCmdSendToPythonConsole::StdCmdSendToPythonConsole()
  :Command("Std_SendToPythonConsole")
{
    // setting the
    sGroup        = "Edit";
    sMenuText     = QT_TR_NOOP("&Send to Python Console");
    sToolTipText  = QT_TR_NOOP("Sends the selected object to the Python console");
    sWhatsThis    = "Std_SendToPythonConsole";
    sStatusTip    = QT_TR_NOOP("Sends the selected object to the Python console");
    sPixmap       = "applications-python";
    sAccel        = "Ctrl+Shift+P";
}

bool StdCmdSendToPythonConsole::isActive()
{
    //active only if either 1 object is selected or multiple subobjects from the same object
    return Gui::Selection().getSelectionEx().size() == 1;
}

void StdCmdSendToPythonConsole::activated(int iMsg)
{
    Q_UNUSED(iMsg);
    const std::vector<Gui::SelectionObject> &sels = Gui::Selection().getSelectionEx("*", App::DocumentObject::getClassTypeId(),
                                                                                    ResolveMode::OldStyleElement, false);
    if (sels.empty())
        return;
    const App::DocumentObject *obj = sels[0].getObject();
    if (!obj)
        return;
    QString docname = QString::fromLatin1(obj->getDocument()->getName());
    QString objname = QString::fromLatin1(obj->getNameInDocument());
    try {
        // clear variables from previous run, if any
        QString cmd = QLatin1String("try:\n    del(doc,lnk,obj,shp,sub,subs)\nexcept Exception:\n    pass\n");
        Gui::Command::runCommand(Gui::Command::Gui,cmd.toLatin1());
        cmd = QString::fromLatin1("doc = App.getDocument(\"%1\")").arg(docname);
        Gui::Command::runCommand(Gui::Command::Gui,cmd.toLatin1());
        //support links
        if (obj->isDerivedFrom<App::Link>()) {
            cmd = QString::fromLatin1("lnk = doc.getObject(\"%1\")").arg(objname);
            Gui::Command::runCommand(Gui::Command::Gui,cmd.toLatin1());
            cmd = QString::fromLatin1("obj = lnk.getLinkedObject()");
            Gui::Command::runCommand(Gui::Command::Gui,cmd.toLatin1());
            const auto link = static_cast<const App::Link*>(obj);
            obj = link->getLinkedObject();
        } else {
            cmd = QString::fromLatin1("obj = doc.getObject(\"%1\")").arg(objname);
            Gui::Command::runCommand(Gui::Command::Gui,cmd.toLatin1());
        }
        if (obj->isDerivedFrom<App::GeoFeature>()) {
            const auto geoObj = static_cast<const App::GeoFeature*>(obj);
            const App::PropertyGeometry* geo = geoObj->getPropertyOfGeometry();
            if (geo){
                cmd = QString::fromLatin1("shp = obj.") + QLatin1String(geo->getName()); //"Shape", "Mesh", "Points", etc.
                Gui::Command::runCommand(Gui::Command::Gui, cmd.toLatin1());
                if (sels[0].hasSubNames()) {
                    std::vector<std::string> subnames = sels[0].getSubNames();
                    QString subname = QString::fromLatin1(subnames[0].c_str());
                    cmd = QString::fromLatin1("sub = obj.getSubObject(\"%1\")").arg(subname);
                    Gui::Command::runCommand(Gui::Command::Gui,cmd.toLatin1());
                    if (subnames.size() > 1) {
                        std::ostringstream strm;
                        strm << "subs = [";
                        for (const auto & subname : subnames) {
                            strm << "obj.getSubObject(\"" << subname << "\"),";
                        }
                        strm << "]";
                        Gui::Command::runCommand(Gui::Command::Gui, strm.str().c_str());
                    }
                }
            }
        }
        //show the python console if it's not already visible, and set the keyboard focus to it
        QWidget* pc = DockWindowManager::instance()->getDockWindow("Python console");
        auto pcPython = qobject_cast<PythonConsole*>(pc);
        if (pcPython) {
            DockWindowManager::instance()->activate(pcPython);
            pcPython->setFocus();
        }
    }
    catch (const Base::Exception& e) {
        e.ReportException();
    }

}


namespace Gui {

void CreateFeatCommands()
{
    CommandManager &rcCmdMgr = Application::Instance->commandManager();

    rcCmdMgr.addCommand(new StdCmdFeatRecompute());
    rcCmdMgr.addCommand(new StdCmdToggleFreeze());
    rcCmdMgr.addCommand(new StdCmdRandomColor());
    rcCmdMgr.addCommand(new StdCmdSendToPythonConsole());
}

} // namespace Gui
