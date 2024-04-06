#pragma once

#include "Global.h"

#include <ll/api/reflection/Dispatcher.h>
#include <mc/server/commands/CommandPermissionLevel.h>

namespace we {
struct Config {
    int version = 0;
    struct CmdSetting {
        bool                   enabled    = true;
        CommandPermissionLevel permission = CommandPermissionLevel::GameDirectors;
    };
    struct {
        CmdSetting weconfig{};
    } commands{};
    struct {
        mce::Color region_line_color{"#FFEC27"};
        mce::Color region_point_color{"#10E436"};
    } colors{};
};
} // namespace we

// "#000000"
// "#144A74"
// "#8E65F3"
// "#07946E"
// "#AB5236"
// "#56575F"
// "#A2A3A7"
// "#FFFFFF"
// "#FF3040"
// "#FF7300"
// "#FFEC27"
// "#10E436"
// "#29ADFF"
// "#83769C"
// "#FF77A8"
// "#FFCCAA"
