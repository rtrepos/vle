/**
 * @file vle/gvle/NewDynamicsBox.cpp
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

#include <vle/gvle/NewDynamicsBox.hpp>
#include <vle/utils/Package.hpp>

namespace vle { namespace gvle {

NewDynamicsBox::NewDynamicsBox(Glib::RefPtr<Gnome::Glade::Xml> xml):
    mXml(xml)
{
    xml->get_widget("DialogNewDynamics", mDialog);
    xml->get_widget("EntryClassName", mEntryClassName);
    xml->get_widget("EntryNamespace", mEntryNamespace);
    xml->get_widget("ButtonOkNewDynamics", mButtonApply);
    xml->get_widget("ButtonCancelNewDynamics", mButtonCancel);

    mButtonApply->signal_clicked().connect(
	sigc::mem_fun(*this, &NewDynamicsBox::onApply));

    mButtonCancel->signal_clicked().connect(
	sigc::mem_fun(*this, &NewDynamicsBox::onCancel));
}

void NewDynamicsBox::onApply()
{
    mDialog->response(Gtk::RESPONSE_OK);
    mDialog->hide_all();
}

void NewDynamicsBox::onCancel()
{
    mDialog->response(Gtk::RESPONSE_CANCEL);
    mDialog->hide_all();
}

int NewDynamicsBox::run()
{
    mEntryClassName->set_text("");
    mEntryNamespace->set_text(utils::Package::package().name());
    mDialog->show_all();
    return mDialog->run();
}

}} // namespace vle gvle