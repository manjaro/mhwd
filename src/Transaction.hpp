/*
 * Transaction.hpp
 *
 *  Created on: 28 sie 2014
 *      Author: dec
 */

#ifndef TRANSACTION_HPP_
#define TRANSACTION_HPP_

#include "Config.hpp"
#include "Data.hpp"
#include "Enums.hpp"

class Transaction
{
public:
    Transaction(){}
    Transaction(Data data, Config* config, MHWD::TRANSACTIONTYPE type, bool allowReinstallation);

    bool isAllowedToReinstall() const;
    Config* config_;
    MHWD::TRANSACTIONTYPE type_;
    std::vector<Config*> dependencyConfigs_;
    std::vector<Config*> conflictedConfigs_;
    std::vector<Config*> configsRequirements_;

//private:
    Data data_;
    bool allowedToReinstall_;
};

#endif /* TRANSACTION_HPP_ */
