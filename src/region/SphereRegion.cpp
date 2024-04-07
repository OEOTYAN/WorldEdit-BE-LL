#include "SphereRegion.h"
#include "utils/Math.h"
#include "utils/Serialize.h"
#include "worldedit/WorldEdit.h"

namespace we {
void SphereRegion::serialize(CompoundTag& tag) const {
    Region::serialize(tag);
    doSerialize(center, tag["center"]);
    doSerialize(radius, tag["radius"]);
}
void SphereRegion::deserialize(CompoundTag const& tag) {
    Region::deserialize(tag);
    ll::reflection::deserialize(center, tag["center"]);
    ll::reflection::deserialize(radius, tag["radius"]);
}

void SphereRegion::updateBoundingBox() {
    auto newRadius    = (int)std::ceil(radius);
    boundingBox.min.x = center.x - newRadius;
    boundingBox.min.y = center.y - newRadius;
    boundingBox.min.z = center.z - newRadius;
    boundingBox.max.x = center.x + newRadius;
    boundingBox.max.y = center.y + newRadius;
    boundingBox.max.z = center.z + newRadius;

    auto& geo = WorldEdit::getInstance().getGeo();
    sphere    = geo.sphere(
        getDim(),
        center.center(),
        (float)radius,
        WorldEdit::getInstance().getConfig().colors.region_line_color
    );
    centerbox = geo.box(
        getDim(),
        center,
        WorldEdit::getInstance().getConfig().colors.region_point_color
    );
}

void SphereRegion::forEachBlockUVInRegion(
    std::function<void(BlockPos const&, double, double)>&& todo
) const {
    forEachBlockInRegion([todo = std::move(todo), this](BlockPos const& pos) {
        int counts = 0;
        for (auto& calPos : pos.getNeighbors()) {
            counts += contains(calPos);
        }
        if (counts < 6) {
            double y = (pos.y - center.y) / radius;
            if (std::abs(y) > 0.8) {
                todo(
                    pos,
                    (std::atan2(pos.z - center.z, pos.x - center.x) + std::numbers::pi)
                        / (std::numbers::pi * 2),
                    std::acos(std::clamp(
                        -sign(y)
                            * sqrt(std::max(
                                0.0,
                                pow2(radius) - pow2(static_cast<double>(pos.z) - center.z)
                                    - pow2(static_cast<double>(pos.x) - center.x)
                            ))
                            / radius,
                        -1.0,
                        1.0
                    )) * std::numbers::inv_pi
                );
            } else {
                todo(
                    pos,
                    (atan2(pos.z - center.z, pos.x - center.x) + std::numbers::pi)
                        / (std::numbers::pi * 2),
                    acos(-y) * std::numbers::inv_pi
                );
            }
        }
    });
}

bool SphereRegion::setMainPos(BlockPos const& pos) {
    center = pos;
    radius = 0.5;
    updateBoundingBox();
    return true;
}

bool SphereRegion::setVicePos(BlockPos const& pos) {
    if (auto dis = pos.distanceTo(center); dis > radius) {
        radius = dis + 0.5f;
        updateBoundingBox();
        return true;
    }
    return false;
}

std::optional<int> SphereRegion::checkChanges(std::span<BlockPos> changes) {
    int x = 0, y = 0, z = 0;
    for (auto& change : changes) {
        x += std::abs(change.x);
        y += std::abs(change.y);
        z += std::abs(change.z);
    }
    if (x == y && y == z) {
        return x / 2;
    }
    return {};
}

bool SphereRegion::expand(std::span<BlockPos> changes) {
    auto check = checkChanges(changes);
    if (!check) {
        return false;
    }
    radius += *check;
    updateBoundingBox();
    return true;
}

bool SphereRegion::contract(std::span<BlockPos> changes) {
    auto check = checkChanges(changes);
    if (!check) {
        return false;
    }
    radius -= *check;
    radius  = std::max(radius, 0.5);
    updateBoundingBox();
    return true;
}

bool SphereRegion::shift(BlockPos const& change) {
    center = center + change;
    updateBoundingBox();
    return true;
}

SphereRegion::SphereRegion(DimensionType d, BoundingBox const& b)
: Region(d, b),
  center((b.min + b.max) / 2),
  radius(b.getSideLength().dot(1) / 6.0) {}
} // namespace we
