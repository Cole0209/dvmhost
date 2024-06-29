// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Remote Command Client
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2024 Bryan Biedenkapp, N2PLL
 *
 */
/**
 * @defgroup remote Remote Command Software (dvmcmd)
 * @brief Digital Voice Modem - Remote Command Client
 * @details Helper to to preform CLI REST API operations against the dvmhost and dvmfne.
 * @ingroup remote
 * 
 * @file Defines.h
 * @ingroup remote
 */
#if !defined(__DEFINES_H__)
#define __DEFINES_H__

#include "common/Defines.h"

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

#undef __PROG_NAME__
#define __PROG_NAME__ "Digital Voice Modem (DVM) Remote Command Client"
#undef __EXE_NAME__ 
#define __EXE_NAME__ "dvmcmd"

#endif // __DEFINES_H__
