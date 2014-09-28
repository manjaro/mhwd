/*
 * Transaction.hpp
 *
 *  Created on: 28 sie 2014
 *      Author: dec
 */

#ifndef TRANSACTION_HPP_
#define TRANSACTION_HPP_

#include <vector>

#include "Config.hpp"
#include "Data.hpp"
#include "Enums.hpp"

class Transaction
{
public:
    Transaction() {}
    Transaction(Data data, std::shared_ptr<Config> config, MHWD::TRANSACTIONTYPE type, bool allowReinstallation);

    bool isAllowedToReinstall() const;
    std::shared_ptr<Config> config_;
    MHWD::TRANSACTIONTYPE type_;
    std::vector<std::shared_ptr<Config>> dependencyConfigs_;
    std::vector<std::shared_ptr<Config>> conflictedConfigs_;
    std::vector<std::shared_ptr<Config>> configsRequirements_;

    bool allowedToReinstall_ = false;
};

#endif /* TRANSACTION_HPP_ */
