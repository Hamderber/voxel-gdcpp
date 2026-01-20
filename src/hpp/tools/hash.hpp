#pragma once

#include "godot_cpp/templates/hashfuncs.hpp"
#include "hpp/voxel/chunk.hpp"
#include <cstdint>
namespace Tools
{
    class Hash
    {
    public:
        static const uint64_t chunk(Voxel::Chunk *p_pChunk)
        {
            return godot::hash_make_uint64_t(p_pChunk);
        }
    };
} //namespace Tools
