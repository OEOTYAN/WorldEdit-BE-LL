//
// Created by OEOTYAN on 2022/06/10.
//
#include "player.hpp"

#define MINIMUM__RESPONSE_TICK 3

namespace worldedit {

    bool playerLeftClick(Player* player,
                         const bool isLong,
                         class ItemStack* item,
                         BlockInstance& blockInstance,
                         FaceID mFace) {
        static std::unordered_map<std::string, long long> tickMap;

        if (!(player->isOP() && player->isCreative())) {
            return true;
        }

        bool needDiscard = false;

        auto xuid = player->getXuid();
        long long tick = player->getLevel().getCurrentServerTick().t;
        if (tickMap.find(xuid) != tickMap.end()) {
            if (abs(tick - tickMap[xuid]) < MINIMUM__RESPONSE_TICK) {
                needDiscard = true;
            }
        }

        auto itemName = item->getTypeName();

        if (itemName == "minecraft:wooden_axe") {
            if (!isLong) {
                tickMap[xuid] = tick;
                if (!needDiscard) {
                    changeMainPos(player, blockInstance);
                }
                return false;
            }
        } else {
            tickMap[xuid] = tick;
            auto& mod = worldedit::getMod();
            if (mod.playerHandToolMap.find(xuid) != mod.playerHandToolMap.end()) {
                auto& toolMap = mod.playerHandToolMap[xuid];
                itemName += std::to_string(item->getAuxValue());
                if (toolMap.find(itemName) != toolMap.end()) {
                    if (!needDiscard) {
                        auto& tool = toolMap[itemName];
                        if (tool->lneedFace && blockInstance != BlockInstance::Null) {
                            BlockPos bPos = blockInstance.getPosition();
                            switch (mFace) {
                                case FaceID::Down:
                                    bPos.y -= 1;
                                    break;
                                case FaceID::Up:
                                    bPos.y += 1;
                                    break;
                                case FaceID::North:
                                    bPos.z -= 1;
                                    break;
                                case FaceID::South:
                                    bPos.z += 1;
                                    break;
                                case FaceID::West:
                                    bPos.x -= 1;
                                    break;
                                case FaceID::East:
                                    bPos.x += 1;
                                    break;
                                default:
                                    break;
                            }
                            auto bi = blockInstance.getBlockSource()->getBlockInstance(bPos);
                            tool->leftClick(player, bi);
                        } else {
                            tool->leftClick(player, blockInstance);
                        }
                    }
                    return false;
                }
            }
        }
        return true;
    }

    bool playerRightClick(Player* player,
                          const bool isLong,
                          class ItemStack* item,
                          BlockInstance& blockInstance,
                          FaceID mFace) {
        static std::unordered_map<std::string, long long> tickMap;

        if (!(player->isOP() && player->isCreative())) {
            return true;
        }

        bool needDiscard = false;

        auto xuid = player->getXuid();
        long long tick = player->getLevel().getCurrentServerTick().t;
        if (tickMap.find(xuid) != tickMap.end()) {
            if (abs(tick - tickMap[xuid]) < MINIMUM__RESPONSE_TICK) {
                needDiscard = true;
            }
        }

        auto itemName = item->getTypeName();

        if (item->getTypeName() == "minecraft:wooden_axe") {
            if (!isLong) {
                tickMap[xuid] = tick;
                if (!needDiscard) {
                    changeVicePos(player, blockInstance);
                }
                return false;
            }
        } else {
            tickMap[xuid] = tick;
            auto& mod = worldedit::getMod();
            itemName += std::to_string(item->getAuxValue());
            if (mod.playerHandToolMap.find(xuid) != mod.playerHandToolMap.end()) {
                auto& toolMap = mod.playerHandToolMap[xuid];
                if (toolMap.find(itemName) != toolMap.end()) {
                    if (!needDiscard) {
                        auto& tool = toolMap[itemName];
                        if (tool->needFace && blockInstance != BlockInstance::Null) {
                            BlockPos bPos = blockInstance.getPosition();
                            switch (mFace) {
                                case FaceID::Down:
                                    bPos.y -= 1;
                                    break;
                                case FaceID::Up:
                                    bPos.y += 1;
                                    break;
                                case FaceID::North:
                                    bPos.z -= 1;
                                    break;
                                case FaceID::South:
                                    bPos.z += 1;
                                    break;
                                case FaceID::West:
                                    bPos.x -= 1;
                                    break;
                                case FaceID::East:
                                    bPos.x += 1;
                                    break;
                                default:
                                    break;
                            }
                            auto bi = blockInstance.getBlockSource()->getBlockInstance(bPos);
                            tool->rightClick(player, bi);
                        } else {
                            tool->rightClick(player, blockInstance);
                        }
                    }
                    return false;
                }
            }
            if (mod.playerBrushMap.find(xuid) != mod.playerBrushMap.end()) {
                auto& brushMap = mod.playerBrushMap[xuid];
                if (brushMap.find(itemName) != brushMap.end()) {
                    if (!needDiscard) {
                        auto& brush = brushMap[itemName];
                        if (brush->needFace && blockInstance != BlockInstance::Null) {
                            BlockPos bPos = blockInstance.getPosition();
                            switch (mFace) {
                                case FaceID::Down:
                                    bPos.y -= 1;
                                    break;
                                case FaceID::Up:
                                    bPos.y += 1;
                                    break;
                                case FaceID::North:
                                    bPos.z -= 1;
                                    break;
                                case FaceID::South:
                                    bPos.z += 1;
                                    break;
                                case FaceID::West:
                                    bPos.x -= 1;
                                    break;
                                case FaceID::East:
                                    bPos.x += 1;
                                    break;
                                default:
                                    break;
                            }
                            auto bi = blockInstance.getBlockSource()->getBlockInstance(bPos);
                            brush->set(player, bi);
                        } else {
                            brush->set(player, blockInstance);
                        }
                    }
                    return false;
                }
            }
        }
        return true;
    }

    void playerSubscribe() {
        Event::PlayerUseItemOnEvent::subscribe([](const Event::PlayerUseItemOnEvent& ev) {
            return playerRightClick(ev.mPlayer, false, ev.mItemStack, *const_cast<BlockInstance*>(&ev.mBlockInstance),
                                    static_cast<FaceID>(ev.mFace));
        });
        Event::PlayerUseItemEvent::subscribe([](const Event::PlayerUseItemEvent& ev) {
            bool requiereWater = true;
            if (Level::getBlock(ev.mPlayer->getPosition().toBlockPos(), ev.mPlayer->getDimensionId()) !=
                BedrockBlocks::mAir) {
                requiereWater = false;
            }
            FaceID face;
            BlockInstance blockInstance = ev.mPlayer->getBlockFromViewVector(face, requiereWater, false, 2048.0f);
            return playerRightClick(ev.mPlayer, true, ev.mItemStack, blockInstance, face);
        });

        Event::PlayerDestroyBlockEvent::subscribe([](const Event::PlayerDestroyBlockEvent& ev) {
            bool requiereWater = true;
            if (Level::getBlock(ev.mPlayer->getPosition().toBlockPos(), ev.mPlayer->getDimensionId()) !=
                BedrockBlocks::mAir) {
                requiereWater = false;
            }
            FaceID face;
            ev.mPlayer->getBlockFromViewVector(face, requiereWater);
            return playerLeftClick(ev.mPlayer, false, ev.mPlayer->getHandSlot(),
                                   *const_cast<BlockInstance*>(&ev.mBlockInstance), face);
        });

        Event::PlayerOpenContainerEvent::subscribe([](const Event::PlayerOpenContainerEvent& ev) {
            bool requiereWater = true;
            if (Level::getBlock(ev.mPlayer->getPosition().toBlockPos(), ev.mPlayer->getDimensionId()) !=
                BedrockBlocks::mAir) {
                requiereWater = false;
            }
            FaceID face;
            BlockInstance blockInstance = ev.mPlayer->getBlockFromViewVector(face, requiereWater);
            return playerRightClick(ev.mPlayer, false, ev.mPlayer->getHandSlot(), blockInstance, face);
        });

        Event::PlayerPlaceBlockEvent::subscribe([](const Event::PlayerPlaceBlockEvent& ev) {
            bool requiereWater = true;
            if (Level::getBlock(ev.mPlayer->getPosition().toBlockPos(), ev.mPlayer->getDimensionId()) !=
                BedrockBlocks::mAir) {
                requiereWater = false;
            }
            FaceID face;
            BlockInstance blockInstance = ev.mPlayer->getBlockFromViewVector(face, requiereWater);
            return playerRightClick(ev.mPlayer, false, ev.mPlayer->getHandSlot(), blockInstance, face);
        });
    }
}  // namespace worldedit

THook(void,
      "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@"
      "AEBVAnimatePacket@@@Z",
      ServerNetworkHandler* serverNetworkHandler,
      NetworkIdentifier const& networkIdentifier,
      AnimatePacket const& animatePacket) {
    if (animatePacket.mAction == AnimatePacket::Action::Swing) {
        Player* player = serverNetworkHandler->getServerPlayer(networkIdentifier);
        bool requiereWater = true;
        if (Level::getBlock(player->getPosition().toBlockPos(), player->getDimensionId()) != BedrockBlocks::mAir) {
            requiereWater = false;
        }
        FaceID face;
        BlockInstance blockInstance = player->getBlockFromViewVector(face, requiereWater, false, 2048.0f);
        worldedit::playerLeftClick(player, true, player->getHandSlot(), blockInstance, face);
    }
    return original(serverNetworkHandler, networkIdentifier, animatePacket);
}