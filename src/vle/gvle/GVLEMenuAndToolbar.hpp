/**
 * @file vle/gvle/GVLEMenuAndToolbar.hpp
 * @author The VLE Development Team
 * See the AUTHORS or Authors.txt file
 */

/*
 * VLE Environment - the multimodeling and simulation environment
 * This file is a part of the VLE environment
 * http://www.sourceforge.net/projects/vle
 *
 * Copyright (C) 2003 - 2007 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (C) 2003 - 2009 ULCO http://www.univ-littoral.fr
 * Copyright (C) 2007 - 2009 INRA http://www.inra.fr
 * Copyright (C) 2007 - 2009 Cirad http://www.cirad.fr
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef VLE_GVLE_GVLEMENUANDTOOLBAR_HPP
#define VLE_GVLE_GVLEMENUANDTOOLBAR_HPP

#include <gtkmm.h>
#include <libglademm.h>

#include <iostream>

namespace vle { namespace gvle {

class GVLE;

/**
 * @brief A class to manage the GVLE main menu window.
 */
class GVLEMenuAndToolbar
{
public:
    static const Glib::ustring UI_DEFINITION;

    GVLEMenuAndToolbar(GVLE* gvle);

    virtual ~GVLEMenuAndToolbar();

    void setParent(GVLE* gvle)
    { mParent = gvle; }

    Gtk::Toolbar* getToolbar()
    { return mToolbar; }

    Gtk::MenuBar* getMenuBar()
    { return mMenuBar; }

    // Menu item
    void hidePaste();
    void showPaste();
    void hideOpenVpz();
    void showOpenVpz();
    void hideSave();
    void showSave();
    void hideSaveAs();
    void showSaveAs();
    void hideCloseTab();
    void showCloseTab();
    void hideCloseProject();
    void showCloseProject();
    void hideImportExport();
    void showImportExport();

    // Menu
    void showMinimalMenu();

    void hideFileModel();

    void hideEditMenu();
    void showEditMenu();

    void hideToolsMenu();
    void showToolsMenu();

    void hideProjectMenu();
    void showProjectMenu();

    void hideViewMenu();
    void showViewMenu();

    void hideSimulationMenu();
    void showSimulationMenu();

    void hideZoomMenu();
    void showZoomMenu();

    // show/hide on actions
    void onOpenFile();
    void onOpenProject();
    void onOpenVpz();

    void onCloseTab(bool vpz, bool empty);

private:
    void init();
    void makeButtonsTools();
    void createUI();

    /* Populate actions */
    void createFileActions();
    void createEditActions();
    void createToolsActions();
    void createProjectActions();
    void createViewActions();
    void createSimulationActions();
    void createZoomActions();
    void createHelpActions();

    void on_my_post_activate(const Glib::RefPtr<Gtk::Action>& action);

    /* class members */
    GVLE*            mParent;
    Gtk::Toolbar*    mToolbar;
    Gtk::MenuBar*    mMenuBar;

    Glib::RefPtr<Gtk::UIManager>   m_refUIManager;
    Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
};

}} // namespace vle gvle

#endif
