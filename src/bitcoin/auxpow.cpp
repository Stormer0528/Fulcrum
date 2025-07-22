// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2011 Vince Durham
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2016 Daniel Kraft
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#include <auxpow.h>

#include <hash.h>
#include <algorithm>
#include "block.h"
#include "merkle.h"

/* Moved from wallet.cpp.  CMerkleTx is necessary for auxpow, independent
   of an enabled (or disabled) wallet.  Always include the code.  */
namespace bitcoin
{
    const uint256 CMerkleTx::ABANDON_HASH(uint256S("0000000000000000000000000000000000000000000000000000000000000001"));

    int
    CAuxPow::getExpectedIndex(uint32_t nNonce, int nChainId, unsigned h)
    {
        // Choose a pseudo-random slot in the chain merkle tree
        // but have it be fixed for a size/nonce/chain combination.
        //
        // This prevents the same work from being used twice for the
        // same chain while reducing the chance that two chains clash
        // for the same slot.

        /* This computation can overflow the uint32 used.  This is not an issue,
           though, since we take the mod against a power-of-two in the end anyway.
           This also ensures that the computation is, actually, consistent
           even if done in 64 bits as it was in the past on some systems.

           Note that h is always <= 30 (enforced by the maximum allowed chain
           merkle branch length), so that 32 bits are enough for the computation.  */

        uint32_t rand = nNonce;
        rand = rand * 1103515245 + 12345;
        rand += nChainId;
        rand = rand * 1103515245 + 12345;

        return rand % (1 << h);
    }


    void
    CAuxPow::initAuxPow(CBlockHeader &header)
    {
        /* Set auxpow flag right now, since we take the block hash below.  */
        header.SetAuxpowFlag(true);

        /* Build a minimal coinbase script input for merge-mining.  */
        const uint256 blockHash = header.GetHash();
        std::vector<unsigned char> inputData(blockHash.begin(), blockHash.end());
        std::reverse(inputData.begin(), inputData.end());
        inputData.push_back(1);
        inputData.insert(inputData.end(), 7, 0);

        /* Fake a parent-block coinbase with just the required input
           script and no outputs.  */
        CMutableTransaction coinbase;
        coinbase.vin.resize(1);
        coinbase.vin[0].prevout.SetNull();
        coinbase.vin[0].scriptSig = (CScript() << inputData);
        assert(coinbase.vout.empty());
        CTransactionRef coinbaseRef = MakeTransactionRef(coinbase);

        /* Build a fake parent block with the coinbase.  */
        CBlock parent;
        parent.nVersion = 1;
        parent.vtx.resize(1);
        parent.vtx[0] = coinbaseRef;
        parent.hashMerkleRoot = BlockMerkleRoot(parent);

        /* Construct the auxpow object.  */
        header.SetAuxpow(new CAuxPow(coinbaseRef));
        assert(header.auxpow->vChainMerkleBranch.empty());
        header.auxpow->nChainIndex = 0;
        assert(header.auxpow->vMerkleBranch.empty());
        header.auxpow->nIndex = 0;
        header.auxpow->parentBlock = parent;
    }

}