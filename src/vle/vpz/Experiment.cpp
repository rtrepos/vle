/*
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems.
 * https://www.vle-project.org
 *
 * Copyright (c) 2003-2018 Gauthier Quesnel <gauthier.quesnel@inra.fr>
 * Copyright (c) 2003-2018 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2018 INRA http://www.inra.fr
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

#include <vle/utils/Exception.hpp>
#include <vle/value/Double.hpp>
#include <vle/value/Set.hpp>
#include <vle/vpz/Experiment.hpp>

#include "utils/i18n.hpp"

namespace vle {
namespace vpz {

Experiment::Experiment()
{
    Condition cond(defaultSimulationEngineCondName());
    cond.setValueToPort("begin", value::Double::create(0));
    cond.setValueToPort("duration", value::Double::create(100));
    conditions().add(cond);
}

void
Experiment::write(std::ostream& out) const
{
    out << "<experiment "
        << "name=\"" << m_name.c_str() << "\" ";

    if (not m_combination.empty()) {
        out << "combination=\"" << m_combination.c_str() << "\" ";
    }

    out << " >\n";

    m_conditions.write(out);
    m_views.write(out);

    out << "</experiment>\n";
}

void
Experiment::clear()
{
    m_name.clear();

    m_conditions.clear();
    m_views.clear();
}

void
Experiment::addConditions(const Conditions& c)
{
    m_conditions.add(c);
}

void
Experiment::addViews(const Views& m)
{
    m_views.add(m);
}

void
Experiment::setName(const std::string& name)
{
    if (name.empty()) {
        throw utils::ArgError(_("Empty experiment name"));
    }

    m_name.assign(name);
}

void
Experiment::setDuration(double duration)
{
    if (not conditions().exist(defaultSimulationEngineCondName()))
        throw utils::ArgError(_("The simulation engine condition"
                                "does not exist"));

    auto& condSim = conditions().get(defaultSimulationEngineCondName());
    condSim.setValueToPort("duration", vle::value::Double::create(duration));
}

double
Experiment::duration() const
{
    if (not conditions().exist(defaultSimulationEngineCondName()))
        throw utils::ArgError(_("The simulation engine condition"
                                "does not exist"));

    const auto& condSim = conditions().get(defaultSimulationEngineCondName());
    return condSim.valueOfPort("duration")->toDouble().value();
}

void
Experiment::setBegin(double begin)
{
    if (not conditions().exist(defaultSimulationEngineCondName()))
        throw utils::ArgError(_("The simulation engine condition"
                                "does not exist"));

    auto& condSim = conditions().get(defaultSimulationEngineCondName());
    condSim.setValueToPort("begin", vle::value::Double::create(begin));
}

double
Experiment::begin() const
{
    if (not conditions().exist(defaultSimulationEngineCondName()))
        throw utils::ArgError(_("The simulation engine condition"
                                "does not exist"));

    const auto& condSim = conditions().get(defaultSimulationEngineCondName());
    return condSim.valueOfPort("begin")->toDouble().value();
}

void
Experiment::cleanNoPermanent()
{
    m_conditions.cleanNoPermanent();
    m_views.observables().cleanNoPermanent();
}

void
Experiment::setCombination(const std::string& name)
{
    if (name != "linear" and name != "total") {
        throw utils::ArgError(_("Unknow combination '%s'"), name.c_str());
    }

    m_combination.assign(name);
}
}
} // namespace vle vpz
