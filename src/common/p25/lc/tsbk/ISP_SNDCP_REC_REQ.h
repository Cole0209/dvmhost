// SPDX-License-Identifier: GPL-2.0-only
/**
* Digital Voice Modem - Common Library
* GPLv2 Open Source. Use is subject to license terms.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* @package DVM / Common Library
* @license GPLv2 License (https://opensource.org/licenses/GPL-2.0)
*
*   Copyright (C) 2024 Bryan Biedenkapp, N2PLL
*
*/
#if !defined(__P25_LC_TSBK__ISP_SNDCP_REC_REQ_H__)
#define  __P25_LC_TSBK__ISP_SNDCP_REC_REQ_H__

#include "common/Defines.h"
#include "common/p25/lc/TSBK.h"

namespace p25
{
    namespace lc
    {
        namespace tsbk
        {
            // ---------------------------------------------------------------------------
            //  Class Declaration
            //      Implements SNDCP REC REQ - SNDCP Reconnect Request (ISP).
            // ---------------------------------------------------------------------------

            class HOST_SW_API ISP_SNDCP_REC_REQ : public TSBK {
            public:
                /// <summary>Initializes a new instance of the ISP_SNDCP_REC_REQ class.</summary>
                ISP_SNDCP_REC_REQ();

                /// <summary>Decode a trunking signalling block.</summary>
                bool decode(const uint8_t* data, bool rawTSBK = false) override;
                /// <summary>Encode a trunking signalling block.</summary>
                void encode(uint8_t* data, bool rawTSBK = false, bool noTrellis = false) override;

                /// <summary>Returns a string that represents the current TSBK.</summary>
                std::string toString(bool isp = true) override;

            public:
                /// <summary>Flag indicationg SU has buffered data to send</summary>
                __PROPERTY(bool, dataToSend, DataToSend);
                /// <summary>SNDCP Data Service Options</summary>
                __PROPERTY(uint8_t, dataServiceOptions, DataServiceOptions);
                /// <summary>SNDCP Data Access Control</summary>
                __PROPERTY(uint32_t, dataAccessControl, DataAccessControl);

                __COPY(ISP_SNDCP_REC_REQ);
            };
        } // namespace tsbk
    } // namespace lc
} // namespace p25

#endif // __P25_LC_TSBK__ISP_SNDCP_REC_REQ_H__
