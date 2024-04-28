#include "PlayerStateManager.h"
#include "worldedit/WorldEdit.h"

#include <ll/api/event/EventBus.h>
#include <ll/api/event/player/PlayerConnectEvent.h>
#include <ll/api/event/player/PlayerLeaveEvent.h>

#include <ll/api/event/player/PlayerDestroyBlockEvent.h>
#include <ll/api/event/player/PlayerSwingEvent.h>

#include <ll/api/event/player/PlayerInteractBlockEvent.h>
#include <ll/api/event/player/PlayerPlaceBlockEvent.h>
#include <ll/api/event/player/PlayerUseItemEvent.h>
#include <ll/api/event/player/PlayerUseItemOnEvent.h>

#include "mc/server/commands/CommandOrigin.h"
#include <ll/api/service/Bedrock.h>
#include <ll/api/utils/ErrorUtils.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/actor/BlockActor.h>
#include <mc/world/level/dimension/Dimension.h>

namespace we {
PlayerStateManager::PlayerStateManager()
: storagedState(WorldEdit::getInstance().getSelf().getDataDir() / u8"player_states") {
    using namespace ll::event;
    auto& bus = EventBus::getInstance();
    listeners.emplace_back(
        bus.emplaceListener<PlayerConnectEvent>([this](PlayerConnectEvent& ev) {
            if (!ev.self().isSimulated()) {
                WorldEdit::getInstance().getPool().addTask([id  = ev.self().getUuid(),
                                                            ptr = weak_from_this()] {
                    if (!ptr.expired()) ptr.lock()->getOrCreate(id);
                });
            }
        })
    );
    listeners.emplace_back(
        bus.emplaceListener<PlayerLeaveEvent>([this](PlayerLeaveEvent& ev) {
            WorldEdit::getInstance().getPool().addTask([id  = ev.self().getUuid(),
                                                        ptr = weak_from_this()] {
                if (!ptr.expired()) ptr.lock()->release(id);
            });
        })
    );
    listeners.emplace_back(
        bus.emplaceListener<PlayerDestroyBlockEvent>([this](PlayerDestroyBlockEvent& ev) {
            if (!ev.self().isOperator() || !ev.self().isCreative()) {
                return;
            }
            ev.setCancelled(
                ClickState::Hold
                == playerLeftClick(
                    ev.self(),
                    false,
                    ev.self().getSelectedItem(),
                    {ev.pos(), ev.self().getDimensionId()},
                    FacingID::Unknown
                )
            );
            if (ev.isCancelled()) {
                WorldEdit::getInstance().geServerScheduler().add<ll::schedule::DelayTask>(
                    1_tick,
                    [dst = WithDim<BlockPos>{ev.pos(), ev.self().getDimensionId()}] {
                        if (auto dim = ll::service::getLevel()->getDimension(dst.dim);
                            dim) {
                            auto& blockSource = dim->getBlockSourceFromMainChunkSource();
                            if (auto blockActor = blockSource.getBlockEntity(dst.pos);
                                blockActor) {
                                blockActor->refresh(blockSource);
                            }
                        }
                    }
                );
            }
        })
    );
    listeners.emplace_back(bus.emplaceListener<PlayerSwingEvent>([this](
                                                                     PlayerSwingEvent& ev
                                                                 ) {
        if (!ev.self().isOperator() || !ev.self().isCreative()) {
            return;
        }
        auto hit = ev.self().traceRay(
            (float)WorldEdit::getInstance().getConfig().player_state.maximum_brush_length,
            false
        );
        if (!hit) {
            return;
        }
        if (hit.mIsHitLiquid && !ev.self().isImmersedInWater()) {
            hit.mBlockPos = hit.mLiquidPos;
            hit.mFacing   = hit.mLiquidFacing;
        }
        playerLeftClick(
            ev.self(),
            true,
            ev.self().getSelectedItem(),
            {hit.mBlockPos, ev.self().getDimensionId()},
            hit.mFacing
        );
    }));
    listeners.emplace_back(bus.emplaceListener<PlayerInteractBlockEvent>(
        [this](PlayerInteractBlockEvent& ev) {
            if (!ev.self().isOperator() || !ev.self().isCreative()) {
                return;
            }
            ev.setCancelled(
                ClickState::Hold
                == playerRightClick(
                    ev.self(),
                    false,
                    ev.self().getSelectedItem(),
                    {ev.pos(), ev.self().getDimensionId()},
                    FacingID::Unknown
                )
            );
        }
    ));
    listeners.emplace_back(
        bus.emplaceListener<PlayerPlacingBlockEvent>([this](PlayerPlacingBlockEvent& ev) {
            if (!ev.self().isOperator() || !ev.self().isCreative()) {
                return;
            }
            ev.setCancelled(
                ClickState::Hold
                == playerRightClick(
                    ev.self(),
                    false,
                    ev.self().getSelectedItem(),
                    {ev.pos(), ev.self().getDimensionId()},
                    FacingID::Unknown
                )
            );
        })
    );
    listeners.emplace_back(
        bus.emplaceListener<PlayerUseItemOnEvent>([this](PlayerUseItemOnEvent& ev) {
            if (!ev.self().isOperator() || !ev.self().isCreative()) {
                return;
            }
            ev.setCancelled(
                ClickState::Hold
                == playerRightClick(
                    ev.self(),
                    false,
                    ev.item(),
                    {ev.blockPos(), ev.self().getDimensionId()},
                    ev.face()
                )
            );
        })
    );
    listeners.emplace_back(bus.emplaceListener<
                           PlayerUseItemEvent>([this](PlayerUseItemEvent& ev) {
        if (!ev.self().isOperator() || !ev.self().isCreative()) {
            return;
        }
        auto hit = ev.self().traceRay(
            (float)WorldEdit::getInstance().getConfig().player_state.maximum_brush_length,
            false
        );
        if (!hit) {
            return;
        }
        if (hit.mIsHitLiquid && !ev.self().isImmersedInWater()) {
            hit.mBlockPos = hit.mLiquidPos;
            hit.mFacing   = hit.mLiquidFacing;
        }
        playerRightClick(
            ev.self(),
            true,
            ev.item(),
            {hit.mBlockPos, ev.self().getDimensionId()},
            hit.mFacing
        );
    }));
}
PlayerStateManager::~PlayerStateManager() {
    using namespace ll::event;
    for (auto& l : listeners) {
        EventBus::getInstance().removeListener(l);
    }
    playerStates.for_each([&](auto&& p) {
        if (p.second->temp || !p.second->dirty()) {
            return;
        }
        CompoundTag nbt;
        p.second->serialize(nbt);
        storagedState.set(p.first.asString(), nbt.toBinaryNbt());
    });
}
std::shared_ptr<PlayerState> PlayerStateManager::get(mce::UUID const& uuid, bool temp) {
    std::shared_ptr<PlayerState> res;
    if (playerStates.if_contains(uuid, [&](auto&& p) { res = p.second; })) {
        return res;
    } else if (!temp) {
        std::optional<std::string> nbt = storagedState.get(uuid.asString());
        if (nbt) {
            res = std::make_shared<PlayerState>(uuid, temp);
            res->deserialize(CompoundTag::fromBinaryNbt(*nbt).value());
            return res;
        }
    }
    return {};
}
std::shared_ptr<PlayerState>
PlayerStateManager::getOrCreate(mce::UUID const& uuid, bool temp) {
    std::shared_ptr<PlayerState> res;
    playerStates.lazy_emplace_l(
        uuid,
        [&](auto&& p) { res = p.second; },
        [&, this](auto&& ctor) {
            res = std::make_shared<PlayerState>(uuid, temp);
            if (!temp) {
                try {
                    std::optional<std::string> nbt = storagedState.get(uuid.asString());
                    if (nbt) {
                        res->deserialize(CompoundTag::fromBinaryNbt(*nbt).value());
                    }
                } catch (...) {}
            }
            ctor(uuid, res);
        }
    );
    return res;
}

std::shared_ptr<PlayerState> PlayerStateManager::get(CommandOrigin const& o) {
    if (auto actor = o.getEntity(); actor && actor->isPlayer()) {
        return get(
            static_cast<Player*>(actor)->getUuid(),
            !static_cast<Player*>(actor)->isSimulated()
        );
    } else {
        return get(mce::UUID::EMPTY, true);
    }
}

std::shared_ptr<PlayerState> PlayerStateManager::getOrCreate(CommandOrigin const& o) {
    if (auto actor = o.getEntity(); actor && actor->isPlayer()) {
        return getOrCreate(
            static_cast<Player*>(actor)->getUuid(),
            !static_cast<Player*>(actor)->isSimulated()
        );
    } else {
        return getOrCreate(mce::UUID::EMPTY, true);
    }
}

bool PlayerStateManager::release(mce::UUID const& uuid) {
    return playerStates.erase_if(uuid, [&](auto&& p) {
        if (p.second->temp || !p.second->dirty()) {
            return true;
        }
        CompoundTag nbt;
        p.second->serialize(nbt);
        return storagedState.set(uuid.asString(), nbt.toBinaryNbt());
    });
}

void PlayerStateManager::remove(mce::UUID const& uuid) {
    playerStates.erase(uuid);
    storagedState.del(uuid.asString());
}

// bool PlayerStateManager::has(mce::UUID const& uuid, bool temp) {
//     if (temp) {
//         return playerStates.contains(uuid);
//     } else {
//         return playerStates.contains(uuid) || storagedState.get(uuid.asString());
//     }
// }

void PlayerStateManager::removeTemps() {
    erase_if(playerStates, [](auto&& p) { return p.second->temp; });
}

PlayerStateManager::ClickState PlayerStateManager::playerLeftClick(
    Player&                  player,
    bool                     isLong,
    ItemStack const&         item,
    WithDim<BlockPos> const& dst,
    FacingID //  mFace TODO: brush
) {
    auto  data    = getOrCreate(player.getUuid(), player.isSimulated());
    auto& current = player.getLevel().getCurrentTick();
    bool  needDiscard{};
    if (current.t - data->lastLeftClick.load().t
        < WorldEdit::getInstance().getConfig().player_state.minimum_response_tick) {
        needDiscard = true;
    }
    auto& itemName = item.getFullNameHash();
    if (data->config.wand == itemName) {
        if (isLong) {
            return ClickState::Pass;
        }
        data->lastLeftClick = current;
        if (!needDiscard) {
            data->setMainPos(dst);
        }
        return ClickState::Hold;
    }
    return ClickState::Pass;
}
PlayerStateManager::ClickState PlayerStateManager::playerRightClick(
    Player&                  player,
    bool                     isLong,
    ItemStack const&         item,
    WithDim<BlockPos> const& dst,
    FacingID // mFace TODO: brush
) {
    auto  data    = getOrCreate(player.getUuid(), player.isSimulated());
    auto& current = player.getLevel().getCurrentTick();
    bool  needDiscard{};
    if (current.t - data->lastRightClick.load().t
        < WorldEdit::getInstance().getConfig().player_state.minimum_response_tick) {
        needDiscard = true;
    }
    auto& itemName = item.getFullNameHash();
    if (data->config.wand == itemName) {
        if (isLong) {
            return ClickState::Pass;
        }
        data->lastRightClick = current;
        if (!needDiscard) {
            data->setVicePos(dst);
        }
        return ClickState::Hold;
    }
    return ClickState::Pass;
}

} // namespace we
