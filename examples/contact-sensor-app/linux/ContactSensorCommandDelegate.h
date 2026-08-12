/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <NamedPipeCommands.h>
#include <json/json.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

/**
 * Out-of-band control of the contact sensor, over the app pipe.
 *
 * The imgui window can already drive the sensor's state, but that needs a display and
 * a person. A test harness runs the app headless, so without this the reported state
 * is whatever the device booted with and no test can tell a working reporting path
 * from a stuck one.
 *
 * Accepts: {"Name":"SetBooleanState","EndpointId":1,"NewState":true}
 * matching the command all-clusters-app already accepts, so the two agree.
 */
class ContactSensorCommandDelegate : public NamedPipeCommandDelegate
{
public:
    void OnEventCommandReceived(const char * json) override;

private:
    static void SetBooleanStateHandler(intptr_t context);
};
