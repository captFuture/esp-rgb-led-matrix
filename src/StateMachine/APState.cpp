/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
@brief  System state: AP
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
@see APState.h

*******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "APState.h"
#include <Arduino.h>
#include "ErrorState.h"
#include "DisplayMgr.h"
#include "MyWebServer.h"
#include "UpdateMgr.h"

#include <WiFi.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Set short wait time for showing a system message in ms */
const uint32_t  APState::SYS_MSG_WAIT_TIME_SHORT = 250u;

/* Set access point SSID */
const char*     APState::WIFI_AP_SSID          = "esp32-rgb-led-matrix";

/* Set access point passphrase (min. 8 characters) */
const char*     APState::WIFI_AP_PASSPHRASE    = "Luke, I am your father.";

/* Access point state instance */
APState         APState::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void APState::entry(StateMachine& sm)
{
    /* Setup wifi access point failed? */
    if (false == WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSPHRASE))
    {
        String errorStr = "Setup wifi access point failed.";

        /* Fatal error */
        Serial.println(errorStr);
        DisplayMgr::getInstance().showSysMsg(errorStr);

        sm.setState(ErrorState::getInstance());
    }
    /* Wifi access point successful up. */
    else
    {
        /* Show SSID and ip address  */
        String infoStr = "SSID: ";
        infoStr += WIFI_AP_SSID;
        infoStr += " IP: ";
        infoStr += WiFi.softAPIP();

        Serial.println(infoStr);
        DisplayMgr::getInstance().showSysMsg(infoStr);
    }

    return;
}

void APState::process(StateMachine& sm)
{
    /* Handle update, there may be one in the background. */
    UpdateMgr::getInstance().process();

    /* As long as no update is running, do handle all other connections. */
    if (false == UpdateMgr::getInstance().isUpdateRunning())
    {
        /* Handle all clients */
        MyWebServer::getInstance().handleClient();
    }

    return;
}

void APState::exit(StateMachine& sm)
{
    /* Nothing to do. */
    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
