/*
 *  mhwd - Manjaro Hardware Detection
 *  Roland Singer <roland@manjaro.org>
 *  Łukasz Matysiak <december0123@gmail.com>
 *
 *  Copyright (C) 2007 Free Software Foundation, Inc.
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

#include "Transaction.hpp"

Transaction::Transaction(Data data, std::shared_ptr<Config> config, MHWD::TRANSACTIONTYPE type,
        bool allowReinstallation)
        :  config_(config), type_(type),
           dependencyConfigs_(data.getAllDependenciesToInstall(config)),
           conflictedConfigs_(data.getAllLocalConflicts(config)),
           configsRequirements_(data.getAllLocalRequirements(config)),
           allowedToReinstall_(allowReinstallation)
{
}

bool Transaction::isAllowedToReinstall() const
{
    return allowedToReinstall_;
}
