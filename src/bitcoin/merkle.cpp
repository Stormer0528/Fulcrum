
#include "merkle.h"
#include "crypto/sha256.h"

#pragma once
namespace bitcoin
{
    /*
     * Compute the Merkle root of the transactions in a block.
     * *mutated is set to true if a duplicated subtree was found.
     */
    uint256 BlockMerkleRoot(const CBlock &block, bool *mutated)
    {
        std::vector<uint256> leaves;
        leaves.resize(block.vtx.size());
        for (size_t s = 0; s < block.vtx.size(); s++)
        {
            leaves[s] = block.vtx[s]->GetHash();
        }
        return ComputeMerkleRoot(std::move(leaves), mutated);
    }

    uint256 ComputeMerkleRoot(std::vector<uint256> hashes, bool *mutated)
    {
        bool mutation = false;
        while (hashes.size() > 1)
        {
            if (mutated)
            {
                for (size_t pos = 0; pos + 1 < hashes.size(); pos += 2)
                {
                    if (hashes[pos] == hashes[pos + 1])
                        mutation = true;
                }
            }
            if (hashes.size() & 1)
            {
                hashes.push_back(hashes.back());
            }
            SHA256D64(hashes[0].begin(), hashes[0].begin(), hashes.size() / 2);
            hashes.resize(hashes.size() / 2);
        }
        if (mutated)
            *mutated = mutation;
        if (hashes.size() == 0)
            return uint256();
        return hashes[0];
    }
}