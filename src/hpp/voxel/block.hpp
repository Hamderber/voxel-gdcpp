#pragma once

namespace Voxel
{
    class Block
    {
    public:
        Block() = default;
        ~Block() = default;

        void set_solid(bool p_isSolid) { m_isSolid = p_isSolid; }
        bool is_solid() const { return m_isSolid; }

    private:
        bool m_isSolid;
    };
} //namespace Voxel
