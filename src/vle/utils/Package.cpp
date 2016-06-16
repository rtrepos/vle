/*
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems.
 * http://www.vle-project.org
 *
 * Copyright (c) 2003-2014 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (c) 2003-2014 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2014 INRA http://www.inra.fr
 *
 * See the AUTHORS or Authors.txt file for copyright owners and
 * contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <fstream>
#include <ostream>
#include <cstring>
#include <thread>
#include <stack>
#include <vle/utils/Package.hpp>
#include <vle/utils/Path.hpp>
#include <vle/utils/Preferences.hpp>
#include <vle/utils/Trace.hpp>
#include <vle/utils/Exception.hpp>
#include <vle/utils/Spawn.hpp>
#include <vle/utils/Filesystem.hpp>
#include <boost/version.hpp>
#include <boost/cast.hpp>
#include <boost/version.hpp>
#include <boost/config.hpp>
#include "details/ShellUtils.hpp"

namespace {

void remove_all(const vle::utils::FSpath& path)
{
    if (not path.exists())
        return;

    if (path.is_file())
        path.remove();

    std::string command;
    try {
        vle::utils::Preferences prefs(true);
        prefs.get("vle.command.dir.remove", &command);

        command = (vle::fmt(command) % path.string()).str();

        auto argv = vle::utils::Spawn::splitCommandLine(command);
        auto exe = std::move(argv.front());
        argv.erase(argv.begin());

        vle::utils::Spawn spawn;
        if (not spawn.start(exe, vle::utils::FSpath::current_path().string(),
                            argv, 50000u))
            throw vle::utils::InternalError(_("fail to start cmake command"));

        spawn.wait();

        std::string message;
        bool success;

        spawn.status(&message, &success);

        if (not message.empty())
            std::cerr << message << '\n';
    } catch (const std::exception& e) {
        TraceAlways(_("Package remove all: unable to remove `%s' with"
                      " the `%s' command"),
                    path.string().c_str(),
                    command.c_str());
    }
}

} // namespace anonymous

namespace vle { namespace utils {

std::vector<std::string>
Spawn::splitCommandLine(const std::string& command)
{
    std::vector<std::string> argv;
    details::ShellUtils su;
    int argcp;
    su.g_shell_parse_argv(command, argcp, argv);

    if (argv.empty())
        throw utils::ArgError(
            (fmt(_("Package command line: error, empty command `%1%'"))
             % command).str());

    argv.front() = Path::findProgram(argv.front());

    return argv;
}

struct Package::Pimpl
{
    Pimpl()
    {
    }

    Pimpl(std::string pkgname)
        : m_pkgname(std::move(pkgname))
        , m_pkgbinarypath()
        , m_pkgsourcepath()
    {
        refreshPath();
    }

    ~Pimpl() = default;

    std::string  m_pkgname;
    std::string  m_pkgbinarypath;
    std::string  m_pkgsourcepath;

    Spawn        m_spawn;
    std::string  m_message;
    std::string  m_strout;
    std::string  m_strerr;
    std::string  mCommandConfigure;
    std::string  mCommandTest;
    std::string  mCommandBuild;
    std::string  mCommandInstall;
    std::string  mCommandClean;
    std::string  mCommandPack;
    std::string  mCommandUnzip;
    bool         m_issuccess;

    void process(const std::string& exe,
                 const std::string& workingDir,
                 const std::vector < std::string >& argv)
    {
        if (not m_spawn.isfinish()) {
            if (utils::Trace::isInLevel(utils::TRACE_LEVEL_DEVS)) {
                utils::Trace::send(
                    (fmt("-[%1%] Need to wait old process before") % exe).str());
            }
            m_spawn.wait();
            m_spawn.status(&m_message, &m_issuccess);
        }
        bool started = m_spawn.start(exe, workingDir, argv);

        if (not started) {
            throw utils::ArgError(
                (fmt(_("Failed to start `%1%'")) % exe).str());
        }
    }

    void refreshPath()
    {
        m_pkgbinarypath.clear();
        m_pkgsourcepath.clear();

        if (not m_pkgname.empty()) {
            FSpath path_binary = Path::path().getBinaryPackagesDir();
            path_binary /= m_pkgname;
            m_pkgbinarypath = path_binary.string();
            FSpath path_source_dir = Path::path().getCurrentDir();
            FSpath path_source_pkg = path_source_dir;
            path_source_pkg /= m_pkgname;
            if (path_source_pkg.exists() or path_source_dir.exists()) {
                m_pkgsourcepath = path_source_pkg.string();
            }
        }
    }

    void select(const std::string& name)
    {
        m_pkgname = name;
        refreshPath();
    }
};


Package::Package(const std::string& pkgname)
    : m_pimpl(new Package::Pimpl(pkgname))
{
}

Package::Package()
    : m_pimpl(new Package::Pimpl())
{
}

Package::~Package()
{
    delete m_pimpl;
}

void Package::create()
{
    const std::string& dirname(utils::Path::path().getTemplate("package"));
    FSpath source(dirname);

    FSpath destination(getDir(PKG_SOURCE));

    if (destination.exists())
        throw utils::FileError(
            (fmt(_("Pkg create error: "
                   "the directory %1% already exists")) %
             destination.string()).str());

    refreshPath();

    FSpath::create_directory(destination);
    std::string command;

    try {
        {
            utils::Preferences prefs(true);
            prefs.get("vle.command.dir.copy", &command);
        }

        command = (vle::fmt(command) % source.string() %
                   destination.string()).str();

        std::vector<std::string> argv = Spawn::splitCommandLine(command);
        std::string exe = std::move(argv.front());
        argv.erase(argv.begin());

        utils::Spawn spawn;
        if (not spawn.start(exe, FSpath::current_path().string(), argv, 50000u))
            throw utils::InternalError(_("fail to start cmake command"));

        std::string output, error;
        while (not spawn.isfinish()) {
            if (spawn.get(&output, &error)) {
                std::cout << output;
                std::cerr << error;

                output.clear();
                error.clear();

                std::this_thread::sleep_for(std::chrono::microseconds(200));
            } else
                break;
        }

        spawn.wait();

        std::string message;
        bool success;
        spawn.status(&message, &success);

        if (not message.empty())
            std::cerr << message << '\n';
    } catch (const std::exception& e) {
        TraceAlways(_("Package creatinig: unable to copy `%s' in `%s' with"
                      "the `%s' command"),
                      source.string().c_str(),
                      destination.string().c_str(),
                      command.c_str());
    }

    m_pimpl->m_strout.append(_("Package creating - done\n"));
}

void Package::configure()
{
    std::string pkg_buildir = getBuildDir(PKG_SOURCE);

    if (m_pimpl->mCommandConfigure.empty()) {
        refreshCommands();
    }
    if (pkg_buildir.empty()) {
        refreshPath();
    }
    if (m_pimpl->mCommandConfigure.empty() ||
            pkg_buildir.empty()) {
        throw utils::FileError(_("Pkg configure error: empty command"));
    }
    if (pkg_buildir.empty()) {
        throw utils::FileError(
                _("Pkg configure error: building directory path is empty"));
    }

    FSpath path { pkg_buildir };
    if (not path.exists()) {
        if (not FSpath::create_directories(path))
            throw utils::FileError(
                _("Pkg configure error: fails to build directories"));
    }

    FScurrent_path_restore restore(path);

    std::string pkg_binarydir = getDir(PKG_BINARY);
    std::string cmd = (fmt(m_pimpl->mCommandConfigure) % pkg_binarydir).str();
    std::vector<std::string> argv = Spawn::splitCommandLine(cmd);
    std::string exe = std::move(argv.front());
    argv.erase(argv.begin());
    try {
        m_pimpl->process(exe, pkg_buildir, argv);
    } catch(const std::exception& e) {
        throw utils::InternalError(
            (fmt(_("Pkg configure error: %1%")) % e.what()).str());
    }
}

void Package::test()
{
    std::string pkg_buildir = getBuildDir(PKG_SOURCE);
    if (m_pimpl->mCommandBuild.empty()) {
        refreshCommands();
    }
    if (pkg_buildir.empty()) {
        refreshPath();
    }
    if (m_pimpl->mCommandBuild.empty() ||
            pkg_buildir.empty()) {
        throw utils::FileError(_("Pkg test error: empty command"));
    }
    if (pkg_buildir.empty()) {
        throw utils::FileError(
                _("Pkg test error: building directory path is empty"));
    }

    FSpath path { pkg_buildir };
    if (not path.exists())
        throw utils::FileError(
            (fmt(_("Pkg test error: building directory '%1%' "
                   "does not exist ")) % pkg_buildir).str());

    FScurrent_path_restore restore(path);

    std::string cmd = (fmt(m_pimpl->mCommandTest) % pkg_buildir).str();
    std::vector<std::string> argv = Spawn::splitCommandLine(cmd);
    std::string exe = std::move(argv.front());
    argv.erase(argv.begin());

    try {
        m_pimpl->process(exe, pkg_buildir, argv);
    } catch (const std::exception& e) {
        throw utils::InternalError(
            (fmt(_("Pkg error: test launch failed %1%")) % e.what()).str());
    }
}

void Package::build()
{
    std::string pkg_buildir = getBuildDir(PKG_SOURCE);
    if (m_pimpl->mCommandBuild.empty()) {
        refreshCommands();
    }

    if (pkg_buildir.empty()) {
        refreshPath();
    }
    if (m_pimpl->mCommandBuild.empty() ||
            pkg_buildir.empty()) {
        throw utils::FileError(_("Pkg build error: empty command"));
    }
    if (pkg_buildir.empty()) {
        throw utils::FileError(
                _("Pkg build error: building directory path is empty"));
    }

    FSpath path { pkg_buildir };
    if (not path.exists())
        configure();

    FScurrent_path_restore restore(path);

    std::string cmd = (vle::fmt(m_pimpl->mCommandBuild) % pkg_buildir).str();
    std::vector<std::string> argv = Spawn::splitCommandLine(cmd);
    std::string exe = std::move(argv.front());
    argv.erase(argv.begin());

    try {
        m_pimpl->process(exe, pkg_buildir, argv);
    } catch(const std::exception& e) {
        throw utils::InternalError(
            (fmt(_("Pkg build error: build failed %1%")) % e.what()).str());
    }
}

void Package::install()
{

    std::string pkg_buildir = getBuildDir(PKG_SOURCE);
    if (m_pimpl->mCommandBuild.empty()) {
        refreshCommands();
    }
    if (pkg_buildir.empty()) {
        refreshPath();
    }
    if (m_pimpl->mCommandBuild.empty() ||
            pkg_buildir.empty()) {
        throw utils::FileError(_("Pkg install error: empty command"));
    }
    if (pkg_buildir.empty()) {
        throw utils::FileError(
                _("Pkg install error: building directory path is empty"));
    }

    FSpath path { pkg_buildir };
    if (not path.exists())
        throw utils::FileError(
            (fmt(_("Pkg install error: building directory '%1%' "
                   "does not exist ")) % pkg_buildir.c_str()).str());

    FScurrent_path_restore restore(path);

    std::string cmd = (vle::fmt(m_pimpl->mCommandInstall) % pkg_buildir).str();
    std::vector<std::string> argv = Spawn::splitCommandLine(cmd);
    std::string exe = std::move(argv.front());
    argv.erase(argv.begin());

    FSpath builddir = pkg_buildir;

    if (not builddir.exists()) {
        throw utils::ArgError(
            (fmt(_("Pkg build error: directory '%1%' does not exist")) %
             builddir.string()).str());
    }

    if (not builddir.is_directory()) {
        throw utils::ArgError(
            (fmt(_("Pkg build error: '%1%' is not a directory")) %
             builddir.string()).str());
    }

    try {
        m_pimpl->process(exe, pkg_buildir, argv);
    } catch(const std::exception& e) {
        throw utils::InternalError(
            (fmt(_("Pkg build error: install lib failed %1%"))
             % e.what()).str());
    }
}

void Package::clean()
{
    FSpath pkg_buildir = getBuildDir(PKG_SOURCE);

    if (pkg_buildir.exists())
        ::remove_all(pkg_buildir);
}

void Package::rclean()
{
    FSpath pkg_buildir = getDir(PKG_BINARY);

    if (pkg_buildir.exists())
        ::remove_all(pkg_buildir);
}

void Package::pack()
{
    std::string pkg_buildir = getBuildDir(PKG_SOURCE);
    if (m_pimpl->mCommandBuild.empty()) {
        refreshCommands();
    }
    if (pkg_buildir.empty()) {
        refreshPath();
        pkg_buildir = getBuildDir(PKG_SOURCE);
    }
    if (m_pimpl->mCommandBuild.empty()) {
        throw utils::FileError(_("Pkg packaging error: empty command"));
    }
    if (pkg_buildir.empty()) {
        throw utils::FileError(_("Pkg packaging error: "
                "no building directory found"));
    }

    FSpath::create_directory(pkg_buildir);

    std::string cmd = (fmt(m_pimpl->mCommandPack) % pkg_buildir).str();
    std::vector<std::string> argv = Spawn::splitCommandLine(cmd);
    std::string exe = std::move(argv.front());
    argv.erase(argv.begin());

    try {
        m_pimpl->process(exe, pkg_buildir, argv);
    } catch(const std::exception& e) {
        throw utils::InternalError(
            (fmt(_("Pkg packaging error: package failed %1%")) % e.what()).str());
    }
}

bool Package::isFinish() const
{
    return m_pimpl->m_spawn.isfinish();
}

bool Package::isSuccess() const
{
    if (not m_pimpl->m_spawn.isfinish()) {
        return false;
    }

    m_pimpl->m_spawn.status(&m_pimpl->m_message,
                            &m_pimpl->m_issuccess);

    return m_pimpl->m_issuccess;
}

bool Package::wait(std::ostream& out, std::ostream& err)
{
    std::string output;
    std::string error;

    output.reserve(Spawn::default_buffer_size);
    error.reserve(Spawn::default_buffer_size);

    while (not m_pimpl->m_spawn.isfinish()) {
        if (m_pimpl->m_spawn.get(&output, &error)) {
            out << output;
            err << error;

            output.clear();
            error.clear();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else {
            break;
        }
    }

    m_pimpl->m_spawn.wait();

    return m_pimpl->m_spawn.status(&m_pimpl->m_message, &m_pimpl->m_issuccess);
}

bool Package::get(std::string *out, std::string *err)
{
    out->reserve(Spawn::default_buffer_size);
    err->reserve(Spawn::default_buffer_size);

    m_pimpl->m_spawn.get(out, err);
    return true;
}

void Package::select(const std::string& name)
{
    m_pimpl->select(name);
}

void Package::remove(const std::string& toremove, VLE_PACKAGE_TYPE type)
{
    std::string pkg_dir = getDir(type);
    FSpath torm(pkg_dir);
    torm /= toremove;

    if (torm.exists())
        ::remove_all(torm);
}

std::string Package::getParentDir(VLE_PACKAGE_TYPE type) const
{
    std::string base_dir = getDir(type);
    if (base_dir.empty()){
        return "";
    } else {
        FSpath base_path = FSpath(base_dir);
        return base_path.parent_path().string();
    }
}

std::string Package::getDir(VLE_PACKAGE_TYPE type) const
{
    switch (type) {
    case PKG_SOURCE:
        return m_pimpl->m_pkgsourcepath;
        break;
    case PKG_BINARY:
        return m_pimpl->m_pkgbinarypath;
        break;
    }
    return "";
}

std::string Package::getLibDir(VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "lib");
}

std::string Package::getSrcDir(VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "src");
}

std::string Package::getDataDir(VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "data");
}

std::string Package::getDocDir(VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "doc");
}

std::string Package::getExpDir(VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "exp");
}

std::string Package::getBuildDir(VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "buildvle");
}

std::string Package::getOutputDir(VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "output");
}

std::string Package::getPluginDir(VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "plugins");
}

std::string Package::getPluginSimulatorDir(VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "plugins", "simulator");
}

std::string Package::getPluginOutputDir(VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "plugins", "output");
}

std::string Package::getPluginGvleGlobalDir(VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "plugins", "gvle", "global");
}

std::string Package::getPluginGvleModelingDir(VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "plugins", "gvle",
            "modeling");
}

std::string Package::getPluginGvleOutputDir(VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "plugins", "gvle", "output");
}

std::string Package::getFile(const std::string& file,
        VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), file);
}

std::string Package::getLibFile(const std::string& file,
        VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "lib", file);
}

std::string Package::getSrcFile(const std::string& file,
        VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "src", file);
}

std::string Package::getDataFile(const std::string& file,
        VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "data", file);
}

std::string Package::getDocFile(const std::string& file,
        VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "doc", file);
}

std::string Package::getExpFile(const std::string& file,
        VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "exp", file);
}

std::string Package::getOutputFile(const std::string& file,
        VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "output", file);
}

std::string Package::getPluginFile(const std::string& file,
        VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "plugins", file);
}

std::string Package::getPluginSimulatorFile(const std::string& file,
        VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "plugins", "simulator",
            file);
}

std::string Package::getPluginOutputFile(const std::string& file,
        VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(getDir(type), "plugins", "output", file);
}

std::string Package::getPluginGvleModelingFile(const std::string& file,
        VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(
        getDir(type), "plugins", "gvle", "modeling", file);
}

std::string Package::getPluginGvleOutputFile(const std::string& file,
        VLE_PACKAGE_TYPE type) const
{
    return utils::Path::buildDirname(
        getDir(type), "plugins", "gvle", "output", file);
}

bool Package::existsBinary() const
{
    FSpath binary_dir = m_pimpl->m_pkgbinarypath;

    return binary_dir.is_directory();
}

bool Package::existsSource() const
{
    return (not m_pimpl->m_pkgsourcepath.empty());
}

bool Package::existsFile(const std::string& path, VLE_PACKAGE_TYPE type)
{
    std::string base_dir = getDir(type);
    if (not base_dir.empty()) {
        FSpath tmp(base_dir);
        tmp /= path;

        return tmp.is_file();
    }
    return false;
}

void Package::addDirectory(const std::string& path, const std::string& name,
        VLE_PACKAGE_TYPE type)
{
    std::string base_dir = getDir(type);
    if (not base_dir.empty()) {
        FSpath tmp(base_dir);
        tmp /= path;
        tmp /= name;
        if (not tmp.exists()) {
            tmp.create_directory();
        }
    }
}

PathList Package::getExperiments(VLE_PACKAGE_TYPE type) const
{
    FSpath pkg(getExpDir(type));

    if (not pkg.is_directory())
        throw utils::InternalError(
            (fmt(_("Pkg list error: '%1%' is not an experiments directory")) %
             pkg.string()).str());

    PathList result;
    std::stack < FSpath > stack;
    stack.push(pkg);

    while (not stack.empty()) {
        FSpath dir = stack.top();
        stack.pop();

        for (FSdirectory_iterator it(dir), end; it != end; ++it) {
            if (it->is_file()) {
                std::string ext = it->path().extension();
                if (ext == ".vpz") {
                    result.push_back(it->path().string());
                }
            } else if (it->is_directory()) {
                stack.push(it->path());
            }
        }
    }
    return result;
}

PathList Package::listLibraries(const std::string& path) const
{
    PathList result;
    FSpath simdir(path);

    if (simdir.is_directory()) {
        std::stack < FSpath > stack;
        stack.push(simdir);
        while (not stack.empty()) {
            FSpath dir = stack.top();
            stack.pop();
            for (FSdirectory_iterator it(dir), end; it != end; ++it) {
                if (it->is_file()) {
                    std::string ext = it->path().extension();

#if defined _WIN32
                    if (ext == ".dll") {
#elif defined __APPLE__
                    if (ext == ".dylib") {
#else
                    if (ext == ".so") {
#endif

                        result.push_back(it->path().string());
                    } else if (it->is_directory()) {
                        stack.push(it->path());
                    }
                }
            }
       }
   }
   return result;
}

PathList Package::getPluginsSimulator() const
{
    return listLibraries(getPluginSimulatorDir(PKG_BINARY));
}

PathList Package::getPluginsOutput() const
{
    return listLibraries(getPluginOutputDir(PKG_BINARY));
}

PathList Package::getPluginsGvleGlobal() const
{
    return listLibraries(getPluginGvleGlobalDir(PKG_BINARY));
}

PathList Package::getPluginsGvleModeling() const
{
    return listLibraries(getPluginGvleModelingDir(PKG_BINARY));
}

PathList Package::getPluginsGvleOutput() const
{
    return listLibraries(getPluginGvleOutputDir(PKG_BINARY));
}

std::string
Package::getMetadataExpDir(VLE_PACKAGE_TYPE type) const
{
    FSpath f = getDir(type);
    f /= "metadata";
    f /= "exp";
    return f.string();
}

std::string
Package::getMetadataExpFile(const std::string& expName,
        VLE_PACKAGE_TYPE type) const
{
    FSpath f = getDir(type);
    f /= "metadata";
    f /= "exp";
    f /= (expName+".vpm");
    return f.string();
}

std::string Package::rename(const std::string& oldname,
        const std::string& newname,
        VLE_PACKAGE_TYPE type)
{
    FSpath oldfilepath = getDir(type);
    oldfilepath /= oldname;

    FSpath newfilepath = oldfilepath.parent_path();
    newfilepath /= newname;

    if (not oldfilepath.exists() or newfilepath.exists()) {
        throw utils::ArgError(
            (fmt(_("In Package `%1%', can not rename `%2%' in `%3%'")) %
             name() % oldfilepath.string() % newfilepath.string()).str());
    }

    FSpath::rename(oldfilepath, newfilepath);

    return newfilepath.string();
}

void Package::copy(const std::string& source, std::string& target)
{
    FSpath::copy_file(source, target);
}

const std::string& Package::name() const
{
    return m_pimpl->m_pkgname;
}

void Package::refreshCommands()
{
    utils::Preferences prefs(true);

    prefs.get("vle.packages.configure", &m_pimpl->mCommandConfigure);
    prefs.get("vle.packages.test", &m_pimpl->mCommandTest);
    prefs.get("vle.packages.build", &m_pimpl->mCommandBuild);
    prefs.get("vle.packages.install", &m_pimpl->mCommandInstall);
    prefs.get("vle.packages.clean", &m_pimpl->mCommandClean);
    prefs.get("vle.packages.package", &m_pimpl->mCommandPack);
    prefs.get("vle.packages.unzip", &m_pimpl->mCommandUnzip);
}

void Package::refreshPath()
{
    m_pimpl->refreshPath();
}

void Package::fillBinaryContent(std::vector<std::string>& pkgcontent)
{
    std::string header = "Package content from: ";
    header += getDir(vle::utils::PKG_BINARY);

    pkgcontent.clear();
    pkgcontent.push_back(header);

    vle::utils::PathList tmp;

    tmp = getExperiments();
    pkgcontent.push_back("-- experiments : ");
    std::sort(tmp.begin(), tmp.end());
    std::vector<std::string>::const_iterator itb = tmp.begin();
    std::vector<std::string>::const_iterator ite = tmp.end();
    for (; itb!=ite; itb++){
        pkgcontent.push_back(*itb);
    }

    tmp = getPluginsSimulator();
    pkgcontent.push_back("-- simulator plugins : ");
    std::sort(tmp.begin(), tmp.end());
    itb = tmp.begin();
    ite = tmp.end();
    for (; itb!=ite; itb++){
        pkgcontent.push_back(*itb);
    }

    tmp = getPluginsOutput();
    pkgcontent.push_back("-- output plugins : ");
    std::sort(tmp.begin(), tmp.end());
    itb = tmp.begin();
    ite = tmp.end();
    for (; itb!=ite; itb++){
        pkgcontent.push_back(*itb);
    }

    tmp = getPluginsGvleGlobal();
    pkgcontent.push_back("-- gvle global plugins : ");
    std::sort(tmp.begin(), tmp.end());
    itb = tmp.begin();
    ite = tmp.end();
    for (; itb!=ite; itb++){
        pkgcontent.push_back(*itb);
    }

    tmp = getPluginsGvleModeling();
    pkgcontent.push_back("-- gvle modeling plugins : ");
    std::sort(tmp.begin(), tmp.end());
    itb = tmp.begin();
    ite = tmp.end();
    for (; itb!=ite; itb++){
        pkgcontent.push_back(*itb);
    }

    tmp = getPluginsGvleOutput();
    pkgcontent.push_back("-- gvle output plugins : ");
    std::sort(tmp.begin(), tmp.end());
    itb = tmp.begin();
    ite = tmp.end();
    for (; itb!=ite; itb++){
        pkgcontent.push_back(*itb);
    }
    return;
}

VLE_API std::ostream& operator<<(std::ostream& out,
        const VLE_PACKAGE_TYPE& type)
{
    switch (type) {
    case PKG_SOURCE:
        out << "SOURCE";
        break;
    case PKG_BINARY:
        out << "BINARY";
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const Package& pkg)
{
    out << "\npackage name.....: " << pkg.m_pimpl->m_pkgname << "\n"
        << "source path........: " << pkg.m_pimpl->m_pkgsourcepath << "\n"
        << "binary path........: " << pkg.m_pimpl->m_pkgbinarypath << "\n"
        << "exists binary......: " << pkg.existsBinary() << "\n"
        << "\n";
    return out;
}

}} // namespace vle utils
