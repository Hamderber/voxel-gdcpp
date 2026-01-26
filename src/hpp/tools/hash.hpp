#pragma once

#include "godot_cpp/variant/vector2i.hpp"
#include "hpp/voxel/chunk.hpp"
#include <cstdint>

namespace Tools
{
    class Hash
    {
    public:
        static const uint64_t chunk(Voxel::Chunk *pChunk)
        {
            const auto pos = pChunk->get_pos();
            return chunk_pos(pos);
        }

        static const uint64_t chunk_pos(godot::Vector2i pos)
        {
            // Must prevent UB from shifting signed integers. Otherwise keys aren't deterministic depending on what value is signed.
            return (static_cast<uint64_t>(pos.x & 0xFFFFFFFFu) << 32) |
                   (static_cast<uint64_t>(pos.y) & 0xFFFFFFFFu);
        }
    };
} //namespace Tools
