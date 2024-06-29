// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Common Library
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2022,2024 Bryan Biedenkapp, N2PLL
 *
 */
#include "Defines.h"
#include "p25/lc/tsbk/OSP_IDEN_UP_VU.h"
#include "Log.h"

using namespace p25;
using namespace p25::defines;
using namespace p25::lc;
using namespace p25::lc::tsbk;

#include <cassert>
#include <cmath>

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Initializes a new instance of the OSP_IDEN_UP_VU class. */
OSP_IDEN_UP_VU::OSP_IDEN_UP_VU() : TSBK()
{
    m_lco = TSBKO::OSP_IDEN_UP_VU;
}

/* Decode a trunking signalling block. */
bool OSP_IDEN_UP_VU::decode(const uint8_t* data, bool rawTSBK)
{
    assert(data != nullptr);

    /* stub */

    return true;
}

/* Encode a trunking signalling block. */
void OSP_IDEN_UP_VU::encode(uint8_t* data, bool rawTSBK, bool noTrellis)
{
    assert(data != nullptr);

    ulong64_t tsbkValue = 0U;

    if ((m_siteIdenEntry.chBandwidthKhz() != 0.0F) && (m_siteIdenEntry.chSpaceKhz() != 0.0F) &&
        (m_siteIdenEntry.txOffsetMhz() != 0.0F) && (m_siteIdenEntry.baseFrequency() != 0U)) {
        uint32_t calcSpace = (uint32_t)(m_siteIdenEntry.chSpaceKhz() / 0.125);

        float fCalcTxOffset = (fabs(m_siteIdenEntry.txOffsetMhz()) / m_siteIdenEntry.chSpaceKhz()) * 1000.0F;
        uint32_t uCalcTxOffset = (uint32_t)fCalcTxOffset;
        if (m_siteIdenEntry.txOffsetMhz() > 0.0F)
            uCalcTxOffset |= 0x2000U; // this sets a positive offset ...

        uint32_t calcBaseFreq = (uint32_t)(m_siteIdenEntry.baseFrequency() / 5);
        uint8_t chanBw = (m_siteIdenEntry.chBandwidthKhz() >= 12.5F) ? IDEN_UP_VU_BW_125K : IDEN_UP_VU_BW_625K;

        tsbkValue = m_siteIdenEntry.channelId();                                    // Channel ID
        tsbkValue = (tsbkValue << 4) + chanBw;                                      // Channel Bandwidth
        tsbkValue = (tsbkValue << 14) + uCalcTxOffset;                              // Transmit Offset
        tsbkValue = (tsbkValue << 10) + calcSpace;                                  // Channel Spacing
        tsbkValue = (tsbkValue << 32) + calcBaseFreq;                               // Base Frequency
    }
    else {
        LogError(LOG_P25, "OSP_IDEN_UP_VU::encode(), invalid values for TSBKO::OSP_IDEN_UP_VU, baseFrequency = %uHz, txOffsetMhz = %fMHz, chBandwidthKhz = %fKHz, chSpaceKhz = %fKHz",
            m_siteIdenEntry.baseFrequency(), m_siteIdenEntry.txOffsetMhz(), m_siteIdenEntry.chBandwidthKhz(),
            m_siteIdenEntry.chSpaceKhz());
        return; // blatantly ignore creating this TSBK
    }

    std::unique_ptr<uint8_t[]> tsbk = TSBK::fromValue(tsbkValue);
    TSBK::encode(data, tsbk.get(), rawTSBK, noTrellis);
}

/* Returns a string that represents the current TSBK. */
std::string OSP_IDEN_UP_VU::toString(bool isp)
{
    return std::string("TSBKO, OSP_IDEN_UP_VU (Channel Identifier Update for VHF/UHF Bands)");
}
