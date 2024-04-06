#pragma once

#include "Region.h"

namespace we {
class CuboidRegion : public Region {
    GeoContainer box;

protected:
    BlockPos mainPos;
    BlockPos vicePos;

public:
    CuboidRegion(DimensionType, BoundingBox const&);

    virtual void serialize(CompoundTag&) const;

    virtual void deserialize(CompoundTag const&);

    void updateBoundingBox() override;

    Type getType() const override { return Cuboid; }

    bool needResetVice() const override { return false; }

    bool expand(std::span<BlockPos>) override;

    bool contract(std::span<BlockPos>) override;

    bool shift(BlockPos const&) override;

    bool setMainPos(BlockPos const&) override;

    bool setVicePos(BlockPos const&) override;

    void forEachLine(std::function<void(BlockPos const&, BlockPos const&)>&& todo
    ) const override;
};
} // namespace we
