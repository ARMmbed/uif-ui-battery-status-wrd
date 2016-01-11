/*
 * Copyright (c) 2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __UIF_UI_REMAINING_TIME_VIEW_H__
#define __UIF_UI_REMAINING_TIME_VIEW_H__

#include "wrd-battery-gauge/BatteryGauge.h"

#include "UIFramework/UIView.h"
#include "UIFramework/UITextView.h"

#include <stdio.h>

#undef printf
#if 0
#include "swo/swo.h"
#define printf(...) { swoprintf(__VA_ARGS__); }
#else
#define printf(...)
#endif

class RemainingTimeView : public UIView
{
public:
    /**
     * @brief UIView class for displaying remaining battery life.
     * @details Uses wrd-battery-gauge to read battery level.
     *
     * @param FontData UIFramework font to display text in.
     */
    RemainingTimeView(const struct FontData* _font)
        :   UIView(),
            variableString()
    {
        font = (_font == NULL) ? &Font_Menu : _font;

        BatteryGauge::setPerMilleChangeCallbackTask(this, &RemainingTimeView::updateImage);
        updateImage(BatteryGauge::getPerMille());
    }

    virtual ~RemainingTimeView(void)
    {
        BatteryGauge::cancelCallbackTask(this, &RemainingTimeView::updateImage);
    }

    /**
     * @brief Fill frame buffer with the content of the UIView-object.
     *
     * @param canvas FrameBuffer-object wrapped in a SharedPointer.
     * @param xOffset Number of pixels the camera has been translated along the
     *        horizontal axis.
     * @param yOffset Number of pixels the camera has been translated along the
     *        vertical axis.
     * @return The time in milliseconds to when the object wants to be called
     *         again. This is a lower-bound, meaning calling the function sooner
     *         will only result in the same data being filled into the frame
     *         buffer again.
     */
    virtual uint32_t fillFrameBuffer(SharedPointer<FrameBuffer>& canvas, int16_t xOffset, int16_t yOffset)
    {
        /* copy image to canvas */
        if (variableCell)
        {
            variableCell->setInverse(inverse);
            variableCell->setHorizontalAlignment(align);
            variableCell->setVerticalAlignment(valign);
            variableCell->setWidth(width);
            variableCell->setHeight(height);

            variableCell->fillFrameBuffer(canvas, xOffset, yOffset);
        }

        return ULONG_MAX;
    }

private:
    /**
     * @brief Helper function for creating a static Text image using the passed
     *        argument as value.
     *
     * @param value Battery capacity in permille.
     */
    void updateImage(int16_t value)
    {
        if (value >= 0)
        {
            const uint32_t capacity = BatteryGauge::getTotalCapacity();
            const uint32_t current = BatteryGauge::getAverageCurrent();
            const uint32_t hours = (uint32_t) value * capacity / 1000 / current; // value is in percent

            const char* format;
            uint32_t remaining = 0;

            if (hours > 24 * 30 * 2)
            {
                // units in months
                remaining = hours / (24 * 30);
                format = "%lu months";
            }
            else if (hours > 24 * 7 * 2)
            {
                // units in weeks
                remaining = hours / (24 * 7);
                format = "%lu weeks";
            }
            else if (hours > 24 * 2)
            {
                // units in days
                remaining = hours / 24;
                format = "%lu days";
            }
            else
            {
                // units in hours
                format = "%lu hours";
            }

            /* create formatted string from argument */
            char buffer[12] = {0};
            snprintf(buffer, 12, format, remaining);
            variableString = std::string(buffer);
        }
        else
        {
            variableString = "N/A";
        }

        variableCell = SharedPointer<UITextView>(new UITextView(variableString.c_str(), font));

        minar::Scheduler::postCallback(wakeupCallback);
    }

private:
    const struct FontData* font;

    std::string variableString;
    SharedPointer<UITextView> variableCell;
};

#endif // __UIF_UI_REMAINING_TIME_VIEW_H__
