/*
 * Transaction.cpp
 *
 *  Created on: 28 sie 2014
 *      Author: dec
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
