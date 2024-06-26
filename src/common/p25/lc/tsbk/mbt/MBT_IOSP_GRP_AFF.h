// SPDX-License-Identifier: GPL-2.0-only
/**
* Digital Voice Modem - Common Library
* GPLv2 Open Source. Use is subject to license terms.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* @package DVM / Common Library
* @license GPLv2 License (https://opensource.org/licenses/GPL-2.0)
*
*   Copyright (C) 2022 Bryan Biedenkapp, N2PLL
*
*/
#if !defined(__P25_LC_TSBK__MBT_IOSP_GRP_AFF_H__)
#define  __P25_LC_TSBK__MBT_IOSP_GRP_AFF_H__

#include "common/Defines.h"
#include "common/p25/lc/AMBT.h"

namespace p25
{
    namespace lc
    {
        namespace tsbk
        {
            // ---------------------------------------------------------------------------
            //  Class Declaration
            //      Implements GRP AFF REQ - Group Affiliation Request (ISP) and
            //          GRP AFF RSP - Group Affiliation Response (OSP)
            // ---------------------------------------------------------------------------

            class HOST_SW_API MBT_IOSP_GRP_AFF : public AMBT {
            public:
                /// <summary>Initializes a new instance of the MBT_IOSP_GRP_AFF class.</summary>
                MBT_IOSP_GRP_AFF();

                /// <summary>Decode a alternate trunking signalling block.</summary>
                bool decodeMBT(const data::DataHeader& dataHeader, const data::DataBlock* blocks) override;
                /// <summary>Encode a alternate trunking signalling block.</summary>
                void encodeMBT(data::DataHeader& dataHeader, uint8_t* pduUserData) override;

                /// <summary>Returns a string that represents the current TSBK.</summary>
                std::string toString(bool isp = false) override;

            public:
                /// <summary>Announcement group.</summary>
                __PROPERTY(uint32_t, announceGroup, AnnounceGroup);
            };
        } // namespace tsbk
    } // namespace lc
} // namespace p25

#endif // __P25_LC_TSBK__MBT_IOSP_GRP_AFF_H__
