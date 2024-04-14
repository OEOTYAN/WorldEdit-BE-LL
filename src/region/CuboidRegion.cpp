#include "CuboidRegion.h"
#include "utils/Serialize.h"
#include "worldedit/WorldEdit.h"

namespace we {
void CuboidRegion::serialize(CompoundTag& tag) const {
    Region::serialize(tag);
    doSerialize(mainPos, tag["mainPos"]);
    doSerialize(vicePos, tag["vicePos"]);
}
void CuboidRegion::deserialize(CompoundTag const& tag) {
    Region::deserialize(tag);
    ll::reflection::deserialize(mainPos, tag.at("mainPos"));
    ll::reflection::deserialize(vicePos, tag.at("vicePos"));
}

void CuboidRegion::updateBoundingBox() {
    boundingBox.min.x = std::min(mainPos.x, vicePos.x);
    boundingBox.min.y = std::min(mainPos.y, vicePos.y);
    boundingBox.min.z = std::min(mainPos.z, vicePos.z);
    boundingBox.max.x = std::max(mainPos.x, vicePos.x);
    boundingBox.max.y = std::max(mainPos.y, vicePos.y);
    boundingBox.max.z = std::max(mainPos.z, vicePos.z);
    boundingBox.max.x = std::max(mainPos.x, vicePos.x);
    box               = WorldEdit::getInstance().getGeo().box(
        getDim(),
        boundingBox,
        WorldEdit::getInstance().getConfig().colors.region_line_color
    );
}

bool CuboidRegion::setMainPos(BlockPos const& pos) {
    mainPos = pos;
    updateBoundingBox();
    return true;
}

bool CuboidRegion::setVicePos(BlockPos const& pos) {
    vicePos = pos;
    updateBoundingBox();
    return true;
}

bool CuboidRegion::expand(std::span<BlockPos> changes) {
    for (auto& change : changes) {
        if (change.x > 0) {
            if (std::max(mainPos.x, vicePos.x) == mainPos.x) {
                mainPos += {change.x, 0, 0};
            } else {
                vicePos += {change.x, 0, 0};
            }
        } else {
            if (std::min(mainPos.x, vicePos.x) == mainPos.x) {
                mainPos += {change.x, 0, 0};
            } else {
                vicePos += {change.x, 0, 0};
            }
        }

        if (change.y > 0) {
            if (std::max(mainPos.y, vicePos.y) == mainPos.y) {
                mainPos += {0, change.y, 0};
            } else {
                vicePos += {0, change.y, 0};
            }
        } else {
            if (std::min(mainPos.y, vicePos.y) == mainPos.y) {
                mainPos += {0, change.y, 0};
            } else {
                vicePos += {0, change.y, 0};
            }
        }

        if (change.z > 0) {
            if (std::max(mainPos.z, vicePos.z) == mainPos.z) {
                mainPos += {0, 0, change.z};
            } else {
                vicePos += {0, 0, change.z};
            }
        } else {
            if (std::min(mainPos.z, vicePos.z) == mainPos.z) {
                mainPos += {0, 0, change.z};
            } else {
                vicePos += {0, 0, change.z};
            }
        }
    }
    updateBoundingBox();
    return true;
}

bool CuboidRegion::contract(std::span<BlockPos> changes) {
    for (auto& change : changes) {
        if (change.x < 0) {
            if (std::max(mainPos.x, vicePos.x) == mainPos.x) {
                mainPos += {change.x, 0, 0};
            } else {
                vicePos += {change.x, 0, 0};
            }
        } else {
            if (std::min(mainPos.x, vicePos.x) == mainPos.x) {
                mainPos += {change.x, 0, 0};
            } else {
                vicePos += {change.x, 0, 0};
            }
        }

        if (change.y < 0) {
            if (std::max(mainPos.y, vicePos.y) == mainPos.y) {
                mainPos += {0, change.y, 0};
            } else {
                vicePos += {0, change.y, 0};
            }
        } else {
            if (std::min(mainPos.y, vicePos.y) == mainPos.y) {
                mainPos += {0, change.y, 0};
            } else {
                vicePos += {0, change.y, 0};
            }
        }

        if (change.z < 0) {
            if (std::max(mainPos.z, vicePos.z) == mainPos.z) {
                mainPos += {0, 0, change.z};
            } else {
                vicePos += {0, 0, change.z};
            }
        } else {
            if (std::min(mainPos.z, vicePos.z) == mainPos.z) {
                mainPos += {0, 0, change.z};
            } else {
                vicePos += {0, 0, change.z};
            }
        }
    }
    updateBoundingBox();
    return true;
}

bool CuboidRegion::shift(BlockPos const& change) {
    mainPos += change;
    vicePos += change;
    updateBoundingBox();
    return true;
}

void CuboidRegion::forEachLine(
    std::function<void(BlockPos const&, BlockPos const&)>&& todo
) const {
    todo(mainPos, vicePos);
}

CuboidRegion::CuboidRegion(DimensionType d, BoundingBox const& b) : Region(d, b) {
    mainPos = b.min;
    vicePos = b.max;
}
} // namespace we
