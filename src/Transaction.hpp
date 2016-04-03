/*
 *  This file is part of the mhwd - Manjaro Hardware Detection project
 *
 *  mhwd - Manjaro Hardware Detection
 *  Roland Singer <roland@manjaro.org>
 *  Łukasz Matysiak <december0123@gmail.com>
 *  Filipe Marques <eagle.software3@gmail.com>
 *
 *  Copyright (C) 2012 - 2016 Manjaro (http://manjaro.org)
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

#ifndef TRANSACTION_HPP_
#define TRANSACTION_HPP_

#include <vector>

#include "Config.hpp"
#include "Data.hpp"
#include "Enums.hpp"

class Transaction
{
public:
    Transaction() = delete;
    Transaction(Data data, std::shared_ptr<Config> config, MHWD::TRANSACTIONTYPE type,
            bool allowReinstallation);

    bool isAllowedToReinstall() const;
    std::shared_ptr<Config> config_;
    MHWD::TRANSACTIONTYPE type_;
    std::vector<std::shared_ptr<Config>> dependencyConfigs_;
    std::vector<std::shared_ptr<Config>> conflictedConfigs_;
    std::vector<std::shared_ptr<Config>> configsRequirements_;

private:
    bool allowedToReinstall_ = false;
};

#endif /* TRANSACTION_HPP_ */
