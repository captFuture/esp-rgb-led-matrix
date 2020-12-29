/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  VOLUMIO plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "VolumioPlugin.h"
#include "RestApi.h"

#include <Logging.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

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

/* Initialize image path for standard icon. */
const char* VolumioPlugin::IMAGE_PATH_STD_ICON  = "/images/volumio.bmp";

/* Initialize image path for "stop" icon. */
const char* VolumioPlugin::IMAGE_PATH_STOP_ICON = "/images/volumioStop.bmp";

/* Initialize image path for "play" icon. */
const char* VolumioPlugin::IMAGE_PATH_PLAY_ICON = "/images/volumioPlay.bmp";

/* Initialize configuration path. */
const char* VolumioPlugin::CONFIG_PATH          = "/configuration";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void VolumioPlugin::start()
{
    lock();

    m_configurationFilename = String(CONFIG_PATH) + "/" + getUID() + ".json";

    /* Try to load configuration. If there is no configuration available, a default configuration
     * will be created.
     */
    createConfigDirectory();
    if (false == loadConfiguration())
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to create initial configuration file %s.", m_configurationFilename.c_str());
        }
    }

    initHttpClient();
    if (false == startHttpRequest())
    {
        /* If a request fails, show standard icon and a '?' */
        m_bitmapWidget.load(IMAGE_PATH_STD_ICON);
        m_textWidget.setFormatStr("\\calign?");

        m_requestTimer.start(UPDATE_PERIOD_SHORT);
    }
    else
    {
        m_requestTimer.start(UPDATE_PERIOD);
    }

    unlock();

    return;
}

void VolumioPlugin::stop()
{
    lock();

    m_requestTimer.stop();

    if (false != SPIFFS.remove(m_configurationFilename))
    {
        LOG_INFO("File %s removed", m_configurationFilename.c_str());
    }

    unlock();

    return;
}

void VolumioPlugin::process()
{
    if ((true == m_requestTimer.isTimerRunning()) &&
        (true == m_requestTimer.isTimeout()))
    {
        if (false == startHttpRequest())
        {
            /* If a request fails, show standard icon and a '?' */
            m_bitmapWidget.load(IMAGE_PATH_STD_ICON);
            m_textWidget.setFormatStr("\\calign?");

            m_requestTimer.start(UPDATE_PERIOD_SHORT);
        }
        else
        {
            m_requestTimer.start(UPDATE_PERIOD);
        }
    }

    return;
}

void VolumioPlugin::active(IGfx& gfx)
{
    lock();

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load icon from filesystem. */
            (void)m_bitmapWidget.load(IMAGE_PATH_STD_ICON);

            m_iconCanvas->update(gfx);
        }
    }
    else
    {
        m_iconCanvas->update(gfx);
    }

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(gfx.getWidth() - ICON_WIDTH, gfx.getHeight(), ICON_WIDTH, 0);

        if (nullptr != m_textCanvas)
        {
            (void)m_textCanvas->addWidget(m_textWidget);

            m_textCanvas->update(gfx);
        }
    }
    else
    {
        m_textCanvas->update(gfx);
    }

    unlock();

    return;
}

void VolumioPlugin::inactive()
{
    /* Nothing to do. */
    return;
}

void VolumioPlugin::registerWebInterface(AsyncWebServer& srv, const String& baseUri)
{
    m_url = baseUri + "/host";

    m_callbackWebHandler = &srv.on( m_url.c_str(),
                                    [this](AsyncWebServerRequest *request)
                                    {
                                        this->webReqHandler(request);
                                    });

    LOG_INFO("[%s] Register: %s", getName(), m_url.c_str());

    return;
}

void VolumioPlugin::unregisterWebInterface(AsyncWebServer& srv)
{
    LOG_INFO("[%s] Unregister: %s", getName(), m_url.c_str());

    if (false == srv.removeHandler(m_callbackWebHandler))
    {
        LOG_WARNING("Couldn't remove %s handler.", getName());
    }

    m_callbackWebHandler = nullptr;

    return;
}

void VolumioPlugin::update(IGfx& gfx)
{
    lock();

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr != m_iconCanvas)
    {
        m_iconCanvas->update(gfx);
    }

    if (nullptr != m_textCanvas)
    {
        m_textCanvas->update(gfx);
    }

    unlock();

    return;
}

String VolumioPlugin::getHost() const
{
    String host;

    lock();
    host = m_volumioHost;
    unlock();

    return host;
}

void VolumioPlugin::setHost(const String& host)
{
    lock();
    m_volumioHost = host;
    (void)saveConfiguration();
    unlock();

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void VolumioPlugin::webReqHandler(AsyncWebServerRequest *request)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    const size_t        MAX_USAGE       = 80U;
    size_t              usageInPercent  = 0U;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET == request->method())
    {
        String      host    = getHost();
        JsonObject  dataObj = jsonDoc.createNestedObject("data");

        dataObj["host"] = host;

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }
    else if (HTTP_POST == request->method())
    {
        /* Argument missing? */
        if (false == request->hasArg("set"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Argument is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            String host = request->arg("set");

            setHost(host);

            /* Prepare response */
            (void)jsonDoc.createNestedObject("data");
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
            httpStatusCode      = HttpStatus::STATUS_CODE_OK;
        }
    }
    else
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }

    usageInPercent = (100U * jsonDoc.memoryUsage()) / jsonDoc.capacity();
    if (MAX_USAGE < usageInPercent)
    {
        LOG_WARNING("JSON document uses %u%% of capacity.", usageInPercent);
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

bool VolumioPlugin::startHttpRequest()
{
    bool status = false;

    if (0 < m_volumioHost.length())
    {
        String url = String("http://") + m_volumioHost + "/api/v1/getState";

        if (true == m_client.begin(url))
        {
            if (false == m_client.GET())
            {
                LOG_WARNING("GET %s failed.", url.c_str());
            }
            else
            {
                status = true;
            }
        }
    }

    return status;
}

void VolumioPlugin::initHttpClient()
{
    m_client.regOnResponse([this](const HttpResponse& rsp){
        size_t              payloadSize             = 0U;
        const char*         payload                 = reinterpret_cast<const char*>(rsp.getPayload(payloadSize));
        size_t              payloadIndex            = 0U;
        String              payloadString;
        const size_t        JSON_DOC_SIZE           = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        DeserializationError    error;

        while(payloadSize > payloadIndex)
        {
            payloadString += payload[payloadIndex];
            ++payloadIndex;
        }

        error = deserializeJson(jsonDoc, payloadString);

        if (DeserializationError::Ok != error.code())
        {
            LOG_WARNING("JSON parse error: %s", error.c_str());
        }
        else
        {
            if (false == jsonDoc["status"].is<String>())
            {
                LOG_WARNING("JSON status is no string.");
            }
            else if (false == jsonDoc["title"].is<String>())
            {
                LOG_WARNING("JSON title is no string.");
            }
            else if (false == jsonDoc["seek"].is<uint32_t>())
            {
                LOG_WARNING("JSON seek is no uint32_t.");
            }
            else
            {
                String      status      = jsonDoc["status"].as<String>();
                String      title       = jsonDoc["title"].as<String>();
                uint32_t    seekValue   = jsonDoc["seek"].as<uint32_t>();

                if (true == title.isEmpty())
                {
                    title = "\\calign-";
                }

                lock();

                /* Workaround for a VOLUMIO bug, which provides a wrong status. */
                if (status == "stop")
                {
                    if (m_lastSeekValue != seekValue)
                    {
                        status = "play";
                    }
                }
                m_lastSeekValue = seekValue;

                if (status == "stop")
                {
                    (void)m_bitmapWidget.load(IMAGE_PATH_STOP_ICON);
                }
                else if (status == "play")
                {
                    (void)m_bitmapWidget.load(IMAGE_PATH_PLAY_ICON);
                }
                else
                {
                    (void)m_bitmapWidget.load(IMAGE_PATH_STD_ICON);
                }

                m_textWidget.setFormatStr(title);

                unlock();
            }
        }
    });

    m_client.regOnClosed([this]() {
        LOG_INFO("Connection closed.");

        lock();
        if (true == m_isConnectionError)
        {
            /* If a request fails, show standard icon and a '?' */
            m_bitmapWidget.load(IMAGE_PATH_STD_ICON);
            m_textWidget.setFormatStr("\\calign?");

            m_requestTimer.start(UPDATE_PERIOD_SHORT);
        }
        m_isConnectionError = false;
        unlock();
    });

    m_client.regOnError([this]() {
        LOG_WARNING("Connection error happened.");

        lock();
        m_isConnectionError = true;
        unlock();
    });
}

bool VolumioPlugin::saveConfiguration()
{
    bool    status  = true;
    File    fd      = SPIFFS.open(m_configurationFilename, "w");

    if (false == fd)
    {
        LOG_WARNING("Failed to create file %s.", m_configurationFilename.c_str());
        status = false;
    }
    else
    {
        const size_t        JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

        jsonDoc["host"] = m_volumioHost;

        (void)serializeJson(jsonDoc, fd);
        fd.close();

        LOG_INFO("File %s saved.", m_configurationFilename.c_str());
    }

    return status;
}

bool VolumioPlugin::loadConfiguration()
{
    bool    status  = true;
    File    fd      = SPIFFS.open(m_configurationFilename, "r");

    if (false == fd)
    {
        LOG_WARNING("Failed to load file %s.", m_configurationFilename.c_str());
        status = false;
    }
    else
    {
        const size_t            JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument     jsonDoc(JSON_DOC_SIZE);
        DeserializationError    error                   = deserializeJson(jsonDoc, fd.readString());

        if (DeserializationError::Ok != error.code())
        {
            LOG_WARNING("Failed to load file %s: %s", m_configurationFilename.c_str(), error.c_str());
            status = false;
        }
        else if (false == jsonDoc["host"].is<String>())
        {
            LOG_WARNING("Host not found or invalid type.");
            status = false;
        }
        else
        {
            m_volumioHost = jsonDoc["host"].as<String>();
        }

        fd.close();
    }

    return status;
}

void VolumioPlugin::createConfigDirectory()
{
    if (false == SPIFFS.exists(CONFIG_PATH))
    {
        if (false == SPIFFS.mkdir(CONFIG_PATH))
        {
            LOG_WARNING("Couldn't create directory: %s", CONFIG_PATH);
        }
    }
}

void VolumioPlugin::lock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTakeRecursive(m_xMutex, portMAX_DELAY);
    }

    return;
}

void VolumioPlugin::unlock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreGiveRecursive(m_xMutex);
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/