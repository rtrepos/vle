/*
 * @file apps/gvle/main.cpp
 *
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * http://www.vle-project.org
 *
 * Copyright (c) 2003-2007 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (c) 2003-2010 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2010 INRA http://www.inra.fr
 *
 * See the AUTHORS or Authors.txt file for copyright owners and contributors
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


#include <vle/manager/VLE.hpp>
#include <vle/gvle/GVLE.hpp>
#include <vle/utils/Tools.hpp>
#include <vle/utils/Trace.hpp>
#include <gtkmm/main.h>
#include <glibmm/optioncontext.h>
#include <glibmm/thread.h>
#include <iostream>

namespace vle { namespace gvle {

class GVLEOptionGroup : public Glib::OptionGroup
{
public:
    GVLEOptionGroup() :
	Glib::OptionGroup("gvle_group", "description of GVLE options",
			  "help description of GVLE group"),
	mInfo(false),
	mVersion(false),
	mLevel(0)
    {
        Glib::OptionEntry entry1;
        entry1.set_long_name("infos");
        entry1.set_short_name('i');
        entry1.set_description(_("Information on GVLE."));
        add_entry(entry1, mInfo);

        Glib::OptionEntry entry2;
        entry2.set_long_name("version");
        entry2.set_short_name('v');
        entry2.set_description(_("Version information."));
        add_entry(entry2, mVersion);

	Glib::OptionEntry entry3;
	entry3.set_long_name("verbose level");
	entry3.set_short_name('V');
	entry3.set_description(_("[int] [0-3] min to max verbose."));
	add_entry(entry3, mLevel);
    }

    bool mInfo;    /// information on VLE program.
    bool mVersion; /// information on VLE version.
    int mLevel;    /// verbose level
};

}} // namespace vle gvle

int main(int argc, char** argv)
{
    Gtk::Main application(&argc, &argv);
    vle::manager::init();

    Glib::OptionContext context;
    vle::gvle::GVLEOptionGroup group;
    context.set_main_group(group);
    context.set_help_enabled(true);

    try {
        context.parse(argc, argv);
    } catch (const Glib::Error& e) {
        std::cerr << _("Error parsing command line: ") << e.what() << std::endl;
        vle::utils::finalize();
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << _("Error parsing command line: ") << e.what() << std::endl;
        vle::utils::finalize();
        return EXIT_FAILURE;
    }

    vle::utils::Trace::setLogFile(
        vle::utils::Trace::getLogFilename("gvle.log"));
    vle::utils::Trace::setLevel(vle::utils::Trace::cast(group.mLevel));

    bool result = true;
    if (group.mInfo) {
        std::cerr << _("GVLE - the Gui of VLE\n");
        vle::utils::printInformations(std::cerr);
    } else if (group.mVersion) {
        std::cerr << _("GVLE - the Gui of VLE\n");
	vle::utils::printVersion(std::cerr);
    } else {
	try {
	    Glib::RefPtr < Gnome::Glade::Xml >  mRefXML =
		Gnome::Glade::Xml::create(
		    vle::utils::Path::path().getGladeFile("gvle.glade"));
	    vle::gvle::GVLE* g = 0;
	    mRefXML->get_widget_derived("WindowPackageBrowser", g);
	    if (argc > 1)
		g->setFileName(argv[1]);
	    application.run(*g);
	    delete g;
        } catch(const Glib::Exception& e) {
            result = false;
            std::cerr << _("\n/!\\ eov Glib error reported: ") <<
                vle::utils::demangle(typeid(e)) << "\n" << e.what();
        } catch(const std::exception& e) {
            result = false;
            std::cerr << _("\n/!\\ eov exception reported: ") <<
                vle::utils::demangle(typeid(e)) << "\n" << e.what();
        }
    }

    vle::manager::finalize();
    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
