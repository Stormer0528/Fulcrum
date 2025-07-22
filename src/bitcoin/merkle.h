
#include "uint256.h"
#include "block.h"

#pragma once
namespace bitcoin
{
    /*
     * Compute the Merkle root of the transactions in a block.
     * *mutated is set to true if a duplicated subtree was found.
     */
    uint256 BlockMerkleRoot(const CBlock &block, bool *mutated = nullptr);
    uint256 ComputeMerkleRoot(std::vector<uint256> hashes, bool* mutated = nullptr);
}