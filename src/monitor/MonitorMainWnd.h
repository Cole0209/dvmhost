// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Host Monitor Software
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2023 Bryan Biedenkapp, N2PLL
 *
 */
/**
 * @file MonitorMainWnd.h
 * @ingroup monitor
 */
#if !defined(__MONITOR_WND_H__)
#define __MONITOR_WND_H__

#include "common/lookups/AffiliationLookup.h"
#include "common/Log.h"
#include "common/Thread.h"

using namespace lookups;

#include <final/final.h>
using namespace finalcut;
#undef null

#include "MonitorMain.h"

#include "LogDisplayWnd.h"
#include "NodeStatusWnd.h"
#include "SelectedNodeWnd.h"
#include "PageSubscriberWnd.h"
#include "RadioCheckSubscriberWnd.h"
#include "InhibitSubscriberWnd.h"
#include "UninhibitSubscriberWnd.h"

#include <vector>

// ---------------------------------------------------------------------------
//  Class Prototypes
// ---------------------------------------------------------------------------

class HOST_SW_API MonitorApplication;

// ---------------------------------------------------------------------------
//  Class Declaration
// ---------------------------------------------------------------------------

/**
 * @brief This class implements the root window control.
 * @ingroup monitor
 */
class HOST_SW_API MonitorMainWnd final : public finalcut::FWidget {
public:
    /**
     * @brief Initializes a new instance of the MonitorMainWnd class.
     * @param widget 
     */
    explicit MonitorMainWnd(FWidget* widget = nullptr) : FWidget{widget}
    {
        __InternalOutputStream(m_logWnd);

        // file menu
        m_quitItem.addAccelerator(FKey::Meta_x); // Meta/Alt + X
        m_quitItem.addCallback("clicked", getFApplication(), &FApplication::cb_exitApp, this);
        m_keyF3.addCallback("activate", getFApplication(), &FApplication::cb_exitApp, this);

        // command menu
        m_pageSU.addCallback("clicked", this, [&]() {
            PageSubscriberWnd wnd{m_selectedCh, this};
            wnd.show();
        });
        m_keyF5.addCallback("activate", this, [&]() {
            PageSubscriberWnd wnd{m_selectedCh, this};
            wnd.show();
        });
        m_radioCheckSU.addCallback("clicked", this, [&]() {
            RadioCheckSubscriberWnd wnd{m_selectedCh, this};
            wnd.show();
        });
        m_cmdMenuSeparator1.setSeparator();
        m_inhibitSU.addCallback("clicked", this, [&]() {
            InhibitSubscriberWnd wnd{m_selectedCh, this};
            wnd.show();
        });
        m_keyF7.addCallback("activate", this, [&]() {
            InhibitSubscriberWnd wnd{m_selectedCh, this};
            wnd.show();
        });
        m_uninhibitSU.addCallback("clicked", this, [&]() {
            UninhibitSubscriberWnd wnd{m_selectedCh, this};
            wnd.show();
        });
        m_keyF8.addCallback("activate", this, [&]() {
            UninhibitSubscriberWnd wnd{m_selectedCh, this};
            wnd.show();
        });

        // help menu
        m_aboutItem.addCallback("clicked", this, [&]() {
            const FString line(2, UniChar::BoxDrawingsHorizontal);
            FMessageBox info("About", line + __PROG_NAME__ + line + L"\n\n"
                L"" + __BANNER__ + L"\n"
                L"Version " + __VER__ + L"\n\n"
                L"Copyright (c) 2017-2024 Bryan Biedenkapp, N2PLL and DVMProject (https://github.com/dvmproject) Authors." + L"\n"
                L"Portions Copyright (c) 2015-2021 by Jonathan Naylor, G4KLX and others", 
                FMessageBox::ButtonType::Ok, FMessageBox::ButtonType::Reject, FMessageBox::ButtonType::Reject, this);
            info.setCenterText();
            info.show();
        });
    }

    /**
     * @brief Helper to get the currently selected channel.
     * @returns lookups::VoiceChData Currently selected channel.
     */
    lookups::VoiceChData getSelectedCh() { return m_selectedCh; }

private:
    friend class MonitorApplication;

    LogDisplayWnd m_logWnd{this};
    SelectedNodeWnd m_selectWnd{this};
    std::vector<NodeStatusWnd*> m_nodes;
    uint32_t m_activeNodeId = 0U;

    lookups::VoiceChData m_selectedCh;

    FString m_line{13, UniChar::BoxDrawingsHorizontal};

    FMenuBar m_menuBar{this};

    FMenu m_fileMenu{"&File", &m_menuBar};
    FMenuItem m_quitItem{"&Quit", &m_fileMenu};

    FMenu m_cmdMenu{"&Commands", &m_menuBar};
    FMenuItem m_pageSU{"&Page Subscriber", &m_cmdMenu};
    FMenuItem m_radioCheckSU{"Radio &Check Subscriber", &m_cmdMenu};
    FMenuItem m_cmdMenuSeparator1{&m_cmdMenu};
    FMenuItem m_inhibitSU{"&Inhibit Subscriber", &m_cmdMenu};
    FMenuItem m_uninhibitSU{"&Uninhibit Subscriber", &m_cmdMenu};

    FMenu m_helpMenu{"&Help", &m_menuBar};
    FMenuItem m_aboutItem{"&About", &m_helpMenu};

    FStatusBar m_statusBar{this};
    FStatusKey m_keyF3{FKey::F3, "Quit", &m_statusBar};
    FStatusKey m_keyF5{FKey::F5, "Page Subscriber", &m_statusBar};
    FStatusKey m_keyF7{FKey::F7, "Inhibit Subscriber", &m_statusBar};
    FStatusKey m_keyF8{FKey::F8, "Uninhibit Subscriber", &m_statusBar};

    /**
     * @brief Helper to initialize the individual channel display elements.
     */
    void intializeNodeDisplay()
    {
        const auto& rootWidget = getRootWidget();
        const int defaultOffsX = 2;
        int offsX = defaultOffsX, offsY = 8;

        int maxWidth = 77;
        if (rootWidget) {
            maxWidth = rootWidget->getClientWidth() - 3;
        }

        /*
        ** Channels
        */
        yaml::Node& voiceChList = g_conf["channels"];

        if (voiceChList.size() != 0U) {
            for (size_t i = 0; i < voiceChList.size(); i++) {
                yaml::Node& channel = voiceChList[i];

                std::string restApiAddress = channel["restAddress"].as<std::string>("127.0.0.1");
                uint16_t restApiPort = (uint16_t)channel["restPort"].as<uint32_t>(REST_API_DEFAULT_PORT);
                std::string restApiPassword = channel["restPassword"].as<std::string>();
                bool restSsl = channel["restSsl"].as<bool>(false);

                ::LogInfoEx(LOG_HOST, "Channel REST API Adddress %s:%u", restApiAddress.c_str(), restApiPort);

                VoiceChData data = VoiceChData(0U, 0U, restApiAddress, restApiPort, restApiPassword, restSsl);

                NodeStatusWnd* wnd = new NodeStatusWnd(this);
                wnd->setChData(data);

                // set control position
                if (offsX + NODE_STATUS_WIDTH > maxWidth) {
                    offsY += NODE_STATUS_HEIGHT + 2;
                    offsX = defaultOffsX;
                }

                wnd->setGeometry(FPoint{offsX, offsY}, FSize{NODE_STATUS_WIDTH, NODE_STATUS_HEIGHT});

                wnd->addCallback("update-selected", this, [&](NodeStatusWnd* wnd) {
                    std::stringstream ss;
                    ss << (uint32_t)(wnd->getChannelId()) << "-" << wnd->getChannelNo() << " / "
                       << wnd->getChData().address() << ":" << wnd->getChData().port() << " / "
                       << "Peer ID " << (uint32_t)(wnd->getPeerId());

                    m_selectWnd.setSelectedText(ss.str());
                    m_selectedCh = wnd->getChData();

                    auto it = std::find(m_nodes.begin(), m_nodes.end(), wnd);
                    if (it != m_nodes.end()) {
                        uint32_t i = it - m_nodes.begin();
                        m_activeNodeId = i;
                    }
                }, wnd);

                offsX += NODE_STATUS_WIDTH + 2;
                m_nodes.push_back(wnd);
            }
        }

        // display all the node windows
        for (auto* wnd : m_nodes) {
            wnd->setModal(false);
            wnd->show();

            wnd->lowerWindow();
            wnd->deactivateWindow();
        }

        // raise and activate first window
        m_nodes.at(0)->raiseWindow();
        m_nodes.at(0)->activateWindow();

        redraw();
    }

    /*
    ** Event Handlers
    */

    /**
     * @brief Event that occurs on keyboard key press.
     * @param e Keyboard Event.
     */
    void onKeyPress(finalcut::FKeyEvent* e) override
    {
        const FKey key = e->key();
        if (key == FKey::Tab) {
            // lower and deactivate current window
            m_nodes.at(m_activeNodeId)->lowerWindow();
            m_nodes.at(m_activeNodeId)->deactivateWindow();

            m_activeNodeId++;
            if (m_activeNodeId >= m_nodes.size()) {
                m_activeNodeId = 0U;
            }

            // raise and activate window
            m_nodes.at(m_activeNodeId)->raiseWindow();
            m_nodes.at(m_activeNodeId)->activateWindow();
        }
    }

    /**
     * @brief Event that occurs when the window is shown.
     * @param e Show Event
     */
    void onShow(FShowEvent* e) override
    {
        intializeNodeDisplay();
        if (g_hideLoggingWnd) {
            const auto& rootWidget = getRootWidget();
            m_logWnd.setGeometry(FPoint{(int)(rootWidget->getClientWidth() - 81), (int)(rootWidget->getClientHeight() - 1)}, FSize{80, 20});

            m_logWnd.minimizeWindow();
        }
    }

    /**
     * @brief Event that occurs when the window is closed.
     * @param e Close Event
     */
    void onClose(FCloseEvent* e) override
    {
        FApplication::closeConfirmationDialog(this, e);
    }
};

#endif // __MONITOR_WND_H__