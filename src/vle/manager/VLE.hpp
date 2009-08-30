/**
 * @file vle/manager/VLE.hpp
 * @author The VLE Development Team
 */

/*
 * VLE Environment - the multimodeling and simulation environment
 * This file is a part of the VLE environment (http://vle.univ-littoral.fr)
 * Copyright (C) 2003 - 2008 The VLE Development Team
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


#ifndef VLE_MANAGER_VLE_HPP
#define VLE_MANAGER_VLE_HPP

#include <vle/manager/DllDefines.hpp>
#include <vle/manager/Types.hpp>
#include <vle/vpz/Vpz.hpp>
#include <vector>

namespace vle { namespace manager {

    /**
     * @brief A class to start the simulation. This class initialises
     * the user directory and install signal. The desctructor clear all
     * singleton classes.
     */
    class VLE_MANAGER_EXPORT VLE
    {
    public:
        /**
         * @brief Initialise signal and user directory.
         */
        explicit VLE(int port = 8000);

        /**
         * @brief Delete all singleton classes.
         */
        ~VLE();

        /*  - - - - - - - - - - - - - --ooOoo-- - - - - - - - - - - -  */

        /**
         * @brief read an experimental frame VPZ, build the complete condition
         * combinations and launch or connect to the vle::Simulator and send
         * experimental frame instance VPZi. vle::Manager can listening on
         * localhost port and wait for VPZ stream like a daemon.
         * @param nbProcessor number of processor.
         * @return true if manager is a success.
         */
        bool runManager(bool allInLocal, bool savevpz, int nbProcessor,
                        const CmdArgs& args);

        /**
         * @brief vle::Simulator: manage a number of processor and launch
         * simulation on it. vle::Simulator run in localhost daemon.
         * @param nbProcessor number of processor.
         * @return true if simulator is a success.
         */
        bool runSimulator(int nbProcessor);

        /**
         * @brief vle::JustRun: start a simulation without analysis of the
         * complete condition combinations, just take the fist and run
         * simulation.
         *
         * @return true if justRun simulation is a success.
         */
        bool justRun(int nbProcessor, const CmdArgs& args);

    private:
        int             mPort;
    };


    /**
     * @brief Initialize the VLE system by:
     * - installling signal (segmentation fault etc.) to standard error
     *   function.
     * - initialize the user directory ($HOME/.vle/ etc.).
     * - initialize the WinSock
     * - initialize the thread system.
     * - initialize the singleton utils::Trace system.
     * - initialize the singleton \c boost::pool devs::Pools and value::Pools.
     */
    VLE_MANAGER_EXPORT void init();

    /**
     * @brief Delete all singleton from VLE system.
     * - kill the utils::Trace singleton.
     * - kill the utils::Path singleton.
     * - kill the value::Pools singleton.
     * - kill the devs::Pools singleton.
     */
    VLE_MANAGER_EXPORT void finalize();

    /**
     * @brief A simple functor to produce a vpz::Vpz object from the std::string
     * filename. To use with std::transform for example.
     */
    struct VLE_MANAGER_EXPORT BuildVPZ
    {
        vpz::Vpz* operator()(const std::string& filename) const
        { return new vpz::Vpz(filename); }
    };

}} // namespace vle manager

#endif
