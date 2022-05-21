//
// Created by OEOTYAN on 2022/05/20.
//
#pragma once
#ifndef WORLDEDIT_CLIPBOARDCOMMAND_H
#define WORLDEDIT_CLIPBOARDCOMMAND_H

#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BlockActor.hpp>
#include <MC/BlockSource.hpp>
#include <MC/CompoundTag.hpp>
#include <MC/Actor.hpp>
#include <MC/Player.hpp>
#include <MC/ServerPlayer.hpp>
#include <MC/Dimension.hpp>
#include <MC/ItemStack.hpp>
#include "Version.h"
#include "string/StringTool.h"
#include <LLAPI.h>
#include <ServerAPI.h>
#include <EventAPI.h>
#include <ScheduleAPI.h>
#include <DynamicCommandAPI.h>
#include "WorldEdit.h"
#include "store/Clipboard.hpp"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // copy cut paste rotate flip clearclipboard

    void clipboardCommandSetup() {
        DynamicCommand::setup(
            "clearclipboard",        // command name
            "clear your clipboard",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                mod.playerClipboardMap.erase(xuid);
                output.success(fmt::format("§aclipboard cleared"));
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "copy",                           // command name
            "copy region to your clipboard",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                    mod.playerRegionMap.end()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto boundingBox = region->getBoundBox();
                    mod.playerClipboardMap[xuid] =
                        Clipboard(boundingBox.bpos2 - boundingBox.bpos1);
                    auto pPos = origin.getPlayer()->getPosition();
                    mod.playerClipboardMap[xuid].playerRelPos =
                        BlockPos(floor(pPos.x), floor(pPos.y), floor(pPos.z)) -
                        boundingBox.bpos1;
                    auto dimID = mod.playerRegionMap[xuid]->getDimensionID();
                    auto blockSource = Level::getBlockSource(dimID);
                    mod.playerRegionMap[xuid]->forEachBlockInRegion(
                        [&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.bpos1;
                            auto blockInstance =
                                blockSource->getBlockInstance(pos);
                            mod.playerClipboardMap[xuid].storeBlock(
                                blockInstance, localPos);
                        });
                    output.success(fmt::format("§aregion copied"));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "paste",                 // command name
            "paste your clipboard",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerClipboardMap.find(xuid) !=
                    mod.playerClipboardMap.end()) {
                    auto pPos = origin.getPlayer()->getPosition();
                    BlockPos pbPos =
                        BlockPos(floor(pPos.x), floor(pPos.y), floor(pPos.z));
                    auto rPos = mod.playerClipboardMap[xuid].playerRelPos;

                    auto dimID = origin.getPlayer()->getDimensionId();
                    mod.playerClipboardMap[xuid].forEachBlockInClipboard(
                        [&](const BlockPos& pos) {
                            auto worldPos = pos + pbPos - rPos;
                            mod.playerClipboardMap[xuid].getSet(pos).setBlock(
                                worldPos, dimID);
                        });
                    output.success(fmt::format("§aclipboard pasted"));
                } else {
                    output.error("You don't have a clipboard yet");
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_CLIPBOARDCOMMAND_H