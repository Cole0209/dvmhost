// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Common Library
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2018 Jonathan Naylor, G4KLX
 *  Copyright (C) 2022,2024 Bryan Biedenkapp, N2PLL
 *
 */
#include "nxdn/NXDNDefines.h"
#include "nxdn/channel/LICH.h"
#include "Log.h"
#include "Utils.h"

using namespace nxdn;
using namespace nxdn::defines;
using namespace nxdn::channel;

#include <cassert>
#include <cstring>

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Initializes a new instance of the LICH class. */

LICH::LICH() :
    m_rfct(RFChannelType::RCCH),
    m_fct(FuncChannelType::USC_SACCH_NS),
    m_option(ChOption::DATA_NORMAL),
    m_outbound(true),
    m_lich(0U)
{
    /* stub */
}

/* Initializes a copy instance of the LICH class. */

LICH::LICH(const LICH& data) :
    m_rfct(RFChannelType::RCCH),
    m_fct(FuncChannelType::USC_SACCH_NS),
    m_option(ChOption::DATA_NORMAL),
    m_outbound(true),
    m_lich(0U)
{
    copy(data);
}

/* Finalizes a instance of LICH class. */

LICH::~LICH() = default;

/* Equals operator. */

LICH& LICH::operator=(const LICH& data)
{
    if (&data != this) {
        m_lich = data.m_lich;

        m_rfct = data.m_rfct;
        m_fct = data.m_fct;
        m_option = data.m_option;
        m_outbound = data.m_outbound;
    }

    return *this;
}

/* Decode a link information channel. */

bool LICH::decode(const uint8_t* data)
{
    assert(data != nullptr);

    uint8_t lich[1U];
    ::memset(lich, 0x00U, 1U);

    uint32_t offset = NXDN_FSW_LENGTH_BITS;
    for (uint32_t i = 0U; i < (NXDN_LICH_LENGTH_BITS / 2U); i++, offset += 2U) {
        bool b = READ_BIT(data, offset);
        WRITE_BIT(lich, i, b);
    }

    m_lich = lich[0U];

#if DEBUG_NXDN_LICH
    LogDebugEx(LOG_NXDN, "LICH::decode()", "m_lich = %02X", m_lich);
#endif

    bool newParity = getParity();
    bool origParity = (m_lich & 0x01U) == 0x01U;

    m_rfct = (RFChannelType::E)((m_lich >> 6) & 0x03U);
    m_fct = (FuncChannelType::E)((m_lich >> 4) & 0x03U);
    m_option = (ChOption::E)((m_lich >> 2) & 0x03U);
    m_outbound = ((m_lich >> 1) & 0x01U) == 0x01U;

    return origParity == newParity;
}

/* Encode a link information channel. */

void LICH::encode(uint8_t* data)
{
    assert(data != nullptr);

    m_lich = 0U;

    m_lich &= 0x3FU;
    m_lich |= (m_rfct & 0x03U) << 6;

    m_lich &= 0xCFU;
    m_lich |= (m_fct & 0x03U) << 4;

    m_lich &= 0xF3U;
    m_lich |= (m_option & 0x03U) << 2;

    m_lich &= 0xFDU;
    m_lich |= (m_outbound ? 0x01U : 0x00U) << 1;

#if DEBUG_NXDN_LICH
    LogDebugEx(LOG_NXDN, "LICH::encode()", "m_lich = %02X", m_lich);
#endif

    bool parity = getParity();
    if (parity)
        m_lich |= 0x01U;
    else
        m_lich &= 0xFEU;

    uint8_t lich[1U];
    ::memset(lich, 0x00U, 1U);

    lich[0U] = m_lich;

    uint32_t offset = NXDN_FSW_LENGTH_BITS;
    for (uint32_t i = 0U; i < (NXDN_LICH_LENGTH_BITS / 2U); i++) {
        bool b = READ_BIT(lich, i);
        WRITE_BIT(data, offset, b);
        offset++;
        WRITE_BIT(data, offset, true);
        offset++;
    }
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------

/* Internal helper to copy the the class. */

void LICH::copy(const LICH& data)
{
    m_lich = data.m_lich;

    m_rfct = (RFChannelType::E)((m_lich >> 6) & 0x03U);
    m_fct = (FuncChannelType::E)((m_lich >> 4) & 0x03U);
    m_option = (ChOption::E)((m_lich >> 2) & 0x03U);
    m_outbound = ((m_lich >> 1) & 0x01U) == 0x01U;
}

/* Internal helper to generate the parity bit for the LICH. */

bool LICH::getParity() const
{
    switch (m_lich & 0xF0U) {
    case 0x80U:
    case 0xB0U:
        return true;
    default:
        return false;
    }
}
