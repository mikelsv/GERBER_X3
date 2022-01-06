// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#include "hpgl_types.h"

namespace Hpgl {

const Color dxfColors[] {
    { 0.0000, 0.0000, 0.0000 }, // unused
    { 1.0000, 0.0000, 0.0000 }, // 1
    { 1.0000, 1.0000, 0.0000 },
    { 0.0000, 1.0000, 0.0000 },
    { 0.0000, 1.0000, 1.0000 },
    { 0.0000, 0.0000, 1.0000 },
    { 1.0000, 0.0000, 1.0000 },
    { 0.5000, 0.5000, 0.5000 }, //{ 1.0000, 1.0000, 1.0000 }, // black or white
    { 0.5000, 0.5000, 0.5000 },
    { 0.7500, 0.7500, 0.7500 },
    { 1.0000, 0.0000, 0.0000 }, // 10
    { 1.0000, 0.5000, 0.5000 },
    { 0.6500, 0.0000, 0.0000 },
    { 0.6500, 0.3250, 0.3250 },
    { 0.5000, 0.0000, 0.0000 },
    { 0.5000, 0.2500, 0.2500 },
    { 0.3000, 0.0000, 0.0000 },
    { 0.3000, 0.1500, 0.1500 },
    { 0.1500, 0.0000, 0.0000 },
    { 0.1500, 0.0750, 0.0750 },
    { 1.0000, 0.2500, 0.0000 }, // 20
    { 1.0000, 0.6250, 0.5000 },
    { 0.6500, 0.1625, 0.0000 },
    { 0.6500, 0.4063, 0.3250 },
    { 0.5000, 0.1250, 0.0000 },
    { 0.5000, 0.3125, 0.2500 },
    { 0.3000, 0.0750, 0.0000 },
    { 0.3000, 0.1875, 0.1500 },
    { 0.1500, 0.0375, 0.0000 },
    { 0.1500, 0.0938, 0.0750 },
    { 1.0000, 0.5000, 0.0000 }, // 30
    { 1.0000, 0.7500, 0.5000 },
    { 0.6500, 0.3250, 0.0000 },
    { 0.6500, 0.4875, 0.3250 },
    { 0.5000, 0.2500, 0.0000 },
    { 0.5000, 0.3750, 0.2500 },
    { 0.3000, 0.1500, 0.0000 },
    { 0.3000, 0.2250, 0.1500 },
    { 0.1500, 0.0750, 0.0000 },
    { 0.1500, 0.1125, 0.0750 },
    { 1.0000, 0.7500, 0.0000 }, // 40
    { 1.0000, 0.8750, 0.5000 },
    { 0.6500, 0.4875, 0.0000 },
    { 0.6500, 0.5688, 0.3250 },
    { 0.5000, 0.3750, 0.0000 },
    { 0.5000, 0.4375, 0.2500 },
    { 0.3000, 0.2250, 0.0000 },
    { 0.3000, 0.2625, 0.1500 },
    { 0.1500, 0.1125, 0.0000 },
    { 0.1500, 0.1313, 0.0750 },
    { 1.0000, 1.0000, 0.0000 }, // 50
    { 1.0000, 1.0000, 0.5000 },
    { 0.6500, 0.6500, 0.0000 },
    { 0.6500, 0.6500, 0.3250 },
    { 0.5000, 0.5000, 0.0000 },
    { 0.5000, 0.5000, 0.2500 },
    { 0.3000, 0.3000, 0.0000 },
    { 0.3000, 0.3000, 0.1500 },
    { 0.1500, 0.1500, 0.0000 },
    { 0.1500, 0.1500, 0.0750 },
    { 0.7500, 1.0000, 0.0000 }, // 60
    { 0.8750, 1.0000, 0.5000 },
    { 0.4875, 0.6500, 0.0000 },
    { 0.5688, 0.6500, 0.3250 },
    { 0.3750, 0.5000, 0.0000 },
    { 0.4375, 0.5000, 0.2500 },
    { 0.2250, 0.3000, 0.0000 },
    { 0.2625, 0.3000, 0.1500 },
    { 0.1125, 0.1500, 0.0000 },
    { 0.1313, 0.1500, 0.0750 },
    { 0.5000, 1.0000, 0.0000 }, // 70
    { 0.7500, 1.0000, 0.5000 },
    { 0.3250, 0.6500, 0.0000 },
    { 0.4875, 0.6500, 0.3250 },
    { 0.2500, 0.5000, 0.0000 },
    { 0.3750, 0.5000, 0.2500 },
    { 0.1500, 0.3000, 0.0000 },
    { 0.2250, 0.3000, 0.1500 },
    { 0.0750, 0.1500, 0.0000 },
    { 0.1125, 0.1500, 0.0750 },
    { 0.2500, 1.0000, 0.0000 }, // 80
    { 0.6250, 1.0000, 0.5000 },
    { 0.1625, 0.6500, 0.0000 },
    { 0.4063, 0.6500, 0.3250 },
    { 0.1250, 0.5000, 0.0000 },
    { 0.3125, 0.5000, 0.2500 },
    { 0.0750, 0.3000, 0.0000 },
    { 0.1875, 0.3000, 0.1500 },
    { 0.0375, 0.1500, 0.0000 },
    { 0.0938, 0.1500, 0.0750 },
    { 0.0000, 1.0000, 0.0000 }, // 90
    { 0.5000, 1.0000, 0.5000 },
    { 0.0000, 0.6500, 0.0000 },
    { 0.3250, 0.6500, 0.3250 },
    { 0.0000, 0.5000, 0.0000 },
    { 0.2500, 0.5000, 0.2500 },
    { 0.0000, 0.3000, 0.0000 },
    { 0.1500, 0.3000, 0.1500 },
    { 0.0000, 0.1500, 0.0000 },
    { 0.0750, 0.1500, 0.0750 },
    { 0.0000, 1.0000, 0.2500 }, // 100
    { 0.5000, 1.0000, 0.6250 },
    { 0.0000, 0.6500, 0.1625 },
    { 0.3250, 0.6500, 0.4063 },
    { 0.0000, 0.5000, 0.1250 },
    { 0.2500, 0.5000, 0.3125 },
    { 0.0000, 0.3000, 0.0750 },
    { 0.1500, 0.3000, 0.1875 },
    { 0.0000, 0.1500, 0.0375 },
    { 0.0750, 0.1500, 0.0938 },
    { 0.0000, 1.0000, 0.5000 }, // 110
    { 0.5000, 1.0000, 0.7500 },
    { 0.0000, 0.6500, 0.3250 },
    { 0.3250, 0.6500, 0.4875 },
    { 0.0000, 0.5000, 0.2500 },
    { 0.2500, 0.5000, 0.3750 },
    { 0.0000, 0.3000, 0.1500 },
    { 0.1500, 0.3000, 0.2250 },
    { 0.0000, 0.1500, 0.0750 },
    { 0.0750, 0.1500, 0.1125 },
    { 0.0000, 1.0000, 0.7500 }, // 120
    { 0.5000, 1.0000, 0.8750 },
    { 0.0000, 0.6500, 0.4875 },
    { 0.3250, 0.6500, 0.5688 },
    { 0.0000, 0.5000, 0.3750 },
    { 0.2500, 0.5000, 0.4375 },
    { 0.0000, 0.3000, 0.2250 },
    { 0.1500, 0.3000, 0.2625 },
    { 0.0000, 0.1500, 0.1125 },
    { 0.0750, 0.1500, 0.1313 },
    { 0.0000, 1.0000, 1.0000 }, // 130
    { 0.5000, 1.0000, 1.0000 },
    { 0.0000, 0.6500, 0.6500 },
    { 0.3250, 0.6500, 0.6500 },
    { 0.0000, 0.5000, 0.5000 },
    { 0.2500, 0.5000, 0.5000 },
    { 0.0000, 0.3000, 0.3000 },
    { 0.1500, 0.3000, 0.3000 },
    { 0.0000, 0.1500, 0.1500 },
    { 0.0750, 0.1500, 0.1500 },
    { 0.0000, 0.7500, 1.0000 }, // 140
    { 0.5000, 0.8750, 1.0000 },
    { 0.0000, 0.4875, 0.6500 },
    { 0.3250, 0.5688, 0.6500 },
    { 0.0000, 0.3750, 0.5000 },
    { 0.2500, 0.4375, 0.5000 },
    { 0.0000, 0.2250, 0.3000 },
    { 0.1500, 0.2625, 0.3000 },
    { 0.0000, 0.1125, 0.1500 },
    { 0.0750, 0.1313, 0.1500 },
    { 0.0000, 0.5000, 1.0000 }, // 150
    { 0.5000, 0.7500, 1.0000 },
    { 0.0000, 0.3250, 0.6500 },
    { 0.3250, 0.4875, 0.6500 },
    { 0.0000, 0.2500, 0.5000 },
    { 0.2500, 0.3750, 0.5000 },
    { 0.0000, 0.1500, 0.3000 },
    { 0.1500, 0.2250, 0.3000 },
    { 0.0000, 0.0750, 0.1500 },
    { 0.0750, 0.1125, 0.1500 },
    { 0.0000, 0.2500, 1.0000 }, // 160
    { 0.5000, 0.6250, 1.0000 },
    { 0.0000, 0.1625, 0.6500 },
    { 0.3250, 0.4063, 0.6500 },
    { 0.0000, 0.1250, 0.5000 },
    { 0.2500, 0.3125, 0.5000 },
    { 0.0000, 0.0750, 0.3000 },
    { 0.1500, 0.1875, 0.3000 },
    { 0.0000, 0.0375, 0.1500 },
    { 0.0750, 0.0938, 0.1500 },
    { 0.0000, 0.0000, 1.0000 }, // 170
    { 0.5000, 0.5000, 1.0000 },
    { 0.0000, 0.0000, 0.6500 },
    { 0.3250, 0.3250, 0.6500 },
    { 0.0000, 0.0000, 0.5000 },
    { 0.2500, 0.2500, 0.5000 },
    { 0.0000, 0.0000, 0.3000 },
    { 0.1500, 0.1500, 0.3000 },
    { 0.0000, 0.0000, 0.1500 },
    { 0.0750, 0.0750, 0.1500 },
    { 0.2500, 0.0000, 1.0000 }, // 180
    { 0.6250, 0.5000, 1.0000 },
    { 0.1625, 0.0000, 0.6500 },
    { 0.4063, 0.3250, 0.6500 },
    { 0.1250, 0.0000, 0.5000 },
    { 0.3125, 0.2500, 0.5000 },
    { 0.0750, 0.0000, 0.3000 },
    { 0.1875, 0.1500, 0.3000 },
    { 0.0375, 0.0000, 0.1500 },
    { 0.0938, 0.0750, 0.1500 },
    { 0.5000, 0.0000, 1.0000 }, // 190
    { 0.7500, 0.5000, 1.0000 },
    { 0.3250, 0.0000, 0.6500 },
    { 0.4875, 0.3250, 0.6500 },
    { 0.2500, 0.0000, 0.5000 },
    { 0.3750, 0.2500, 0.5000 },
    { 0.1500, 0.0000, 0.3000 },
    { 0.2250, 0.1500, 0.3000 },
    { 0.0750, 0.0000, 0.1500 },
    { 0.1125, 0.0750, 0.1500 },
    { 0.7500, 0.0000, 1.0000 }, // 200
    { 0.8750, 0.5000, 1.0000 },
    { 0.4875, 0.0000, 0.6500 },
    { 0.5688, 0.3250, 0.6500 },
    { 0.3750, 0.0000, 0.5000 },
    { 0.4375, 0.2500, 0.5000 },
    { 0.2250, 0.0000, 0.3000 },
    { 0.2625, 0.1500, 0.3000 },
    { 0.1125, 0.0000, 0.1500 },
    { 0.1313, 0.0750, 0.1500 },
    { 1.0000, 0.0000, 1.0000 }, // 210
    { 1.0000, 0.5000, 1.0000 },
    { 0.6500, 0.0000, 0.6500 },
    { 0.6500, 0.3250, 0.6500 },
    { 0.5000, 0.0000, 0.5000 },
    { 0.5000, 0.2500, 0.5000 },
    { 0.3000, 0.0000, 0.3000 },
    { 0.3000, 0.1500, 0.3000 },
    { 0.1500, 0.0000, 0.1500 },
    { 0.1500, 0.0750, 0.1500 },
    { 1.0000, 0.0000, 0.7500 }, // 220
    { 1.0000, 0.5000, 0.8750 },
    { 0.6500, 0.0000, 0.4875 },
    { 0.6500, 0.3250, 0.5688 },
    { 0.5000, 0.0000, 0.3750 },
    { 0.5000, 0.2500, 0.4375 },
    { 0.3000, 0.0000, 0.2250 },
    { 0.3000, 0.1500, 0.2625 },
    { 0.1500, 0.0000, 0.1125 },
    { 0.1500, 0.0750, 0.1313 },
    { 1.0000, 0.0000, 0.5000 }, // 230
    { 1.0000, 0.5000, 0.7500 },
    { 0.6500, 0.0000, 0.3250 },
    { 0.6500, 0.3250, 0.4875 },
    { 0.5000, 0.0000, 0.2500 },
    { 0.5000, 0.2500, 0.3750 },
    { 0.3000, 0.0000, 0.1500 },
    { 0.3000, 0.1500, 0.2250 },
    { 0.1500, 0.0000, 0.0750 },
    { 0.1500, 0.0750, 0.1125 },
    { 1.0000, 0.0000, 0.2500 }, // 240
    { 1.0000, 0.5000, 0.6250 },
    { 0.6500, 0.0000, 0.1625 },
    { 0.6500, 0.3250, 0.4063 },
    { 0.5000, 0.0000, 0.1250 },
    { 0.5000, 0.2500, 0.3125 },
    { 0.3000, 0.0000, 0.0750 },
    { 0.3000, 0.1500, 0.1875 },
    { 0.1500, 0.0000, 0.0375 },
    { 0.1500, 0.0750, 0.0938 },
    { 0.3300, 0.3300, 0.3300 }, // 250
    { 0.4640, 0.4640, 0.4640 },
    { 0.5980, 0.5980, 0.5980 },
    { 0.7320, 0.7320, 0.7320 },
    { 0.8660, 0.8660, 0.8660 },
    { 0.5000, 0.5000, 0.5000 } // 255//    { 1.0000, 1.0000, 1.0000 } // 255
};



}
