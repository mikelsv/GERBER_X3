/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     * * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include "dxf_entity.h"
namespace Dxf {
struct Leader final : Entity {
    Leader(SectionParser* sp);
    Type type() const override { return Type::LEADER; }
    GraphicObject toGo() const override {
        qWarning("%s NOT IMPLEMENTED!", __FUNCTION__);
        return {};
    }
    // void write(QDataStream&) const override { }
    // void read(QDataStream&) override { }
    void parse(CodeData& code) override {
        do {
            data.push_back(code);
            code = sp->nextCode();
        } while (code.code() != 0);
    }
};
} // namespace Dxf
