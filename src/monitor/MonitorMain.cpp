// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Host Monitor Software
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2023 Bryan Biedenkapp, N2PLL
 *
 */
#include "Defines.h"
#include "common/yaml/Yaml.h"
#include "common/Log.h"
#include "MonitorMain.h"
#include "MonitorApplication.h"
#include "MonitorMainWnd.h"

using namespace lookups;

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <vector>

// ---------------------------------------------------------------------------
//	Macros
// ---------------------------------------------------------------------------

#define IS(s) (::strcmp(argv[i], s) == 0)

// ---------------------------------------------------------------------------
//  Global Variables
// ---------------------------------------------------------------------------

std::string g_progExe = std::string(__EXE_NAME__);
std::string g_iniFile = std::string(DEFAULT_CONF_FILE);
yaml::Node g_conf;
bool g_debug = false;

bool g_hideLoggingWnd = false;

lookups::IdenTableLookup* g_idenTable = nullptr;

// ---------------------------------------------------------------------------
//	Global Functions
// ---------------------------------------------------------------------------

/* Helper to print a fatal error message and exit. */

void fatal(const char* msg, ...)
{
    char buffer[400U];
    ::memset(buffer, 0x20U, 400U);

    va_list vl;
    va_start(vl, msg);

    ::vsprintf(buffer, msg, vl);

    va_end(vl);

    ::fprintf(stderr, "%s: FATAL PANIC; %s\n", g_progExe.c_str(), buffer);
    exit(EXIT_FAILURE);
}

/* Helper to pring usage the command line arguments. (And optionally an error.) */

void usage(const char* message, const char* arg)
{
    ::fprintf(stdout, __PROG_NAME__ " %s (built %s)\r\n", __VER__, __BUILD__);
    ::fprintf(stdout, "Copyright (c) 2017-2025 Bryan Biedenkapp, N2PLL and DVMProject (https://github.com/dvmproject) Authors.\n");
    ::fprintf(stdout, "Portions Copyright (c) 2015-2021 by Jonathan Naylor, G4KLX and others\n\n");
    if (message != nullptr) {
        ::fprintf(stderr, "%s: ", g_progExe.c_str());
        ::fprintf(stderr, message, arg);
        ::fprintf(stderr, "\n\n");
    }

    ::fprintf(stdout, 
        "usage: %s [-dvh]"
        "[--hide-log]"
        "[-c <configuration file>]"
        "\n\n"
        "  -d                          enable debug\n"
        "  -v                          show version information\n"
        "  -h                          show this screen\n"
        "\n"
        "  --hide-log                  hide interactive logging window on startup\n"
        "\n"
        "  -c <file>                   specifies the monitor configuration file to use\n"
        "\n"
        "  --                          stop handling options\n",
        g_progExe.c_str());

    exit(EXIT_FAILURE);
}

/* Helper to validate the command line arguments. */

int checkArgs(int argc, char* argv[])
{
    int i, p = 0;

    // iterate through arguments
    for (i = 1; i <= argc; i++)
    {
        if (argv[i] == nullptr) {
            break;
        }

        if (*argv[i] != '-') {
            continue;
        }
        else if (IS("--")) {
            ++p;
            break;
        }
        else if (IS("-c")) {
            if (argc-- <= 0)
                usage("error: %s", "must specify the monitor configuration file to use");
            g_iniFile = std::string(argv[++i]);

            if (g_iniFile.empty())
                usage("error: %s", "monitor configuration file cannot be blank!");

            p += 2;
        }
        else if (IS("--hide-log")) {
            ++p;
            g_hideLoggingWnd = true;
        }
        else if (IS("-d")) {
            ++p;
            g_debug = true;
        }
        else if (IS("-v")) {
            ::fprintf(stdout, __PROG_NAME__ " %s (built %s)\r\n", __VER__, __BUILD__);
            ::fprintf(stdout, "Copyright (c) 2017-2025 Bryan Biedenkapp, N2PLL and DVMProject (https://github.com/dvmproject) Authors.\n");
            ::fprintf(stdout, "Portions Copyright (c) 2015-2021 by Jonathan Naylor, G4KLX and others\n\n");
            if (argc == 2)
                exit(EXIT_SUCCESS);
        }
        else if (IS("-h")) {
            usage(nullptr, nullptr);
            if (argc == 2)
                exit(EXIT_SUCCESS);
        }
        else {
            usage("unrecognized option `%s'", argv[i]);
        }
    }

    if (p < 0 || p > argc) {
        p = 0;
    }

    return ++p;
}

// ---------------------------------------------------------------------------
//  Program Entry Point
// ---------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argv[0] != nullptr && *argv[0] != 0)
        g_progExe = std::string(argv[0]);

    if (argc > 1) {
        // check arguments
        int i = checkArgs(argc, argv);
        if (i < argc) {
            argc -= i;
            argv += i;
        }
        else {
            argc--;
            argv++;
        }
    }

    // initialize system logging
    bool ret = ::LogInitialise("", "", 0U, 1U);
    if (!ret) {
        ::fprintf(stderr, "unable to open the log file\n");
        return 1;
    }

    ::LogInfo(__PROG_NAME__ " " __VER__ " (built " __BUILD__ ")\r\n" \
        "Copyright (c) 2017-2025 Bryan Biedenkapp, N2PLL and DVMProject (https://github.com/dvmproject) Authors.\r\n" \
        "Portions Copyright (c) 2015-2021 by Jonathan Naylor, G4KLX and others\r\n" \
        ">> Host Monitor\r\n");

    try {
        ret = yaml::Parse(g_conf, g_iniFile.c_str());
        if (!ret) {
            ::fatal("cannot read the configuration file, %s\n", g_iniFile.c_str());
        }
    }
    catch (yaml::OperationException const& e) {
        ::fatal("cannot read the configuration file - %s (%s)", g_iniFile.c_str(), e.message());
    }

    // setup the finalcut tui
    MonitorApplication app{argc, argv};

    MonitorMainWnd wnd{&app};
    finalcut::FWidget::setMainWidget(&wnd);

    // try to load bandplan identity table
    std::string idenLookupFile = g_conf["iden_table"]["file"].as<std::string>();
    uint32_t idenReloadTime = g_conf["iden_table"]["time"].as<uint32_t>(0U);

    if (idenLookupFile.length() <= 0U) {
        ::LogError(LOG_HOST, "No bandplan identity table? This must be defined!");
        return 1;
    }

    yaml::Node& voiceChList = g_conf["channels"];
    if (voiceChList.size() == 0U) {
        ::LogError(LOG_HOST, "No channels defined to monitor? This must be defined!");
        return 1;
    }

    g_logDisplayLevel = 0U;

    LogInfo("Iden Table Lookups");
    LogInfo("    File: %s", idenLookupFile.length() > 0U ? idenLookupFile.c_str() : "None");
    if (idenReloadTime > 0U)
        LogInfo("    Reload: %u mins", idenReloadTime);

    g_idenTable = new IdenTableLookup(idenLookupFile, idenReloadTime);
    g_idenTable->read();

    // show and start the application
    wnd.show();

    finalcut::FApplication::setColorTheme<dvmColorTheme>();
    app.resetColors();
    app.redraw();
    
    int _errno = app.exec();
    ::LogFinalise();
    return _errno;
}
