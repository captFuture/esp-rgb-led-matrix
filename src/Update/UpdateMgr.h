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
@brief  Update manager
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module handles the over-the-air update.

*******************************************************************************/
/** @defgroup updatemgr Update Manager
 * This module handles the over-the-air update.
 *
 * @{
 */

#ifndef __UPDATEMGR_H__
#define __UPDATEMGR_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <ArduinoOTA.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The update manager handles everything around an on-the-air update.
 */
class UpdateMgr
{
public:

    /**
     * Get update manager instance.
     * 
     * @return Update manager
     */
    static UpdateMgr& getInstance(void)
    {
        return m_instance;
    }

    /**
     * Initialize update manager, to be able to receive updates over-the-air.
     */
    void init(void);

    /**
     * Is an update in progress?
     * 
     * @return If an update is running it returns true otherwise false.
     */
    bool isUpdateRunning(void) const
    {
        return m_updateIsRunning;
    }

    /**
     * Handle over-the-air update.
     */
    void process(void);

    /** Over-the-air update password */
    static const char*      OTA_PASSWORD;

    /** Standard wait time for showing a system message in ms */
    static const uint32_t   SYS_MSG_WAIT_TIME_STD;

private:

    /** Instance of the update manager. */
    static UpdateMgr    m_instance;

    /** Is the over-the-air update initialized? */
    bool                m_isInitialized;

    /** Is an update in progress? */
    bool                m_updateIsRunning;

    /** The number of display pixels, showing the current OTA progress. */
    uint16_t            m_progress;

    /**
     * Constructs the update manager.
     */
    UpdateMgr();

    /**
     * Destroys the update manager.
     */
    ~UpdateMgr();

    /**
     * Over-the-air update start.
     */
    static void onStart(void);

    /**
     * Over-the-air update end.
     */
    static void onEnd(void);

    /**
     * On progress of over-the-air update.
     * 
     * @param[in] progress  Number of written bytes.
     * @param[in] total     Total size of the whole binary, which to update.
     */
    static void onProgress(unsigned int progress, unsigned int total);

    /**
     * On error of over-the-air update.
     * 
     * @param[in] error Error information
     */
    static void onError(ota_error_t error);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __UPDATEMGR_H__ */

/** @} */