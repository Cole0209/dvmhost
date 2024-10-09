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
#include "p25/lc/tsbk/OSP_SNDCP_CH_GNT.h"

using namespace p25;
using namespace p25::defines;
using namespace p25::lc;
using namespace p25::lc::tsbk;

#include <cassert>

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Initializes a new instance of the OSP_SNDCP_CH_GNT class. */

OSP_SNDCP_CH_GNT::OSP_SNDCP_CH_GNT() : TSBK(),
    m_dataServiceOptions(0U),
    m_dataChannelNo(0U)
{
    m_lco = TSBKO::OSP_SNDCP_CH_GNT;
}

/* Decode a trunking signalling block. */

bool OSP_SNDCP_CH_GNT::decode(const uint8_t* data, bool rawTSBK)
{
    assert(data != nullptr);

    /* stub */

    return true;
}

/* Encode a trunking signalling block. */

void OSP_SNDCP_CH_GNT::encode(uint8_t* data, bool rawTSBK, bool noTrellis)
{
    assert(data != nullptr);

    ulong64_t tsbkValue = 0U;

    uint32_t calcSpace = (uint32_t)(m_siteIdenEntry.chSpaceKhz() / 0.125);
    float calcTxOffset = m_siteIdenEntry.txOffsetMhz() * 1000000.0;

    uint32_t txFrequency = (uint32_t)((m_siteIdenEntry.baseFrequency() + ((calcSpace * 125) * m_dataChannelNo)));
    uint32_t rxFrequency = (uint32_t)(txFrequency + (uint32_t)calcTxOffset);

    uint32_t rootFreq = rxFrequency - m_siteIdenEntry.baseFrequency();
    uint32_t rxChNo = rootFreq / (m_siteIdenEntry.chSpaceKhz() * 1000);

    tsbkValue = 0U;
    tsbkValue = (tsbkValue << 8) + m_dataServiceOptions;                            // Data Service Options
    if (m_grpVchId != 0U) {
        tsbkValue = (tsbkValue << 4) + m_grpVchId;                                  // Channel (T) ID
    }
    else {
        tsbkValue = (tsbkValue << 4) + m_siteData.channelId();                      // Channel (T) ID
    }
    tsbkValue = (tsbkValue << 12) + m_dataChannelNo;                                // Channel (T) Number
    if (m_grpVchId != 0U) {
        tsbkValue = (tsbkValue << 4) + m_grpVchId;                                  // Channel (R) ID
    }
    else {
        tsbkValue = (tsbkValue << 4) + m_siteData.channelId();                      // Channel (R) ID
    }
    tsbkValue = (tsbkValue << 12) + (rxChNo & 0xFFFU);                              // Channel (R) Number
    tsbkValue = (tsbkValue << 24) + m_dstId;                                        // Target Radio Address

    std::unique_ptr<uint8_t[]> tsbk = TSBK::fromValue(tsbkValue);
    TSBK::encode(data, tsbk.get(), rawTSBK, noTrellis);
}

/* Returns a string that represents the current TSBK. */

std::string OSP_SNDCP_CH_GNT::toString(bool isp)
{
    return std::string("TSBKO, OSP_SNDCP_CH_GNT (SNDCP Data Channel Grant)");
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------

/* Internal helper to copy the the class. */

void OSP_SNDCP_CH_GNT::copy(const OSP_SNDCP_CH_GNT& data)
{
    TSBK::copy(data);

    m_dataServiceOptions = data.m_dataServiceOptions;
    m_dataChannelNo = data.m_dataChannelNo;
}
