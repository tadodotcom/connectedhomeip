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

#include "ContactSensorCommandDelegate.h"

#include <app/clusters/boolean-state-server/CodegenIntegration.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app::Clusters;

namespace {

// The pending request, handed to the CHIP task. Only one is in flight at a time: the
// pipe is drained serially, and each request is consumed before the next is read.
struct PendingSet
{
    EndpointId endpointId;
    bool newState;
};

PendingSet gPendingSet;

} // namespace

void ContactSensorCommandDelegate::SetBooleanStateHandler(intptr_t context)
{
    auto * request = reinterpret_cast<PendingSet *>(context);
    auto * booleanState = BooleanState::FindClusterOnEndpoint(request->endpointId);
    if (booleanState == nullptr)
    {
        ChipLogError(NotSpecified, "No BooleanState cluster on endpoint %u", request->endpointId);
        return;
    }
    booleanState->SetStateValue(request->newState);
    ChipLogProgress(NotSpecified, "BooleanState set to %s on endpoint %u",
                    request->newState ? "true" : "false", request->endpointId);
}

void ContactSensorCommandDelegate::OnEventCommandReceived(const char * json)
{
    Json::Reader reader;
    Json::Value value;

    if (!reader.parse(json, value))
    {
        ChipLogError(NotSpecified, "Command is not valid JSON: %s", json);
        return;
    }

    if (!value.isMember("Name") || !value["Name"].isString())
    {
        ChipLogError(NotSpecified, "Command has no Name: %s", json);
        return;
    }

    std::string name = value["Name"].asString();
    if (name != "SetBooleanState")
    {
        ChipLogError(NotSpecified, "Unhandled command: %s", name.c_str());
        return;
    }

    if (!value.isMember("EndpointId") || !value["EndpointId"].isUInt() || !value.isMember("NewState") ||
        !value["NewState"].isBool())
    {
        ChipLogError(NotSpecified, "SetBooleanState needs a numeric EndpointId and a boolean NewState: %s", json);
        return;
    }

    gPendingSet.endpointId = static_cast<EndpointId>(value["EndpointId"].asUInt());
    gPendingSet.newState   = value["NewState"].asBool();

    // The pipe is read on its own thread; cluster state must only be touched on the
    // CHIP task.
    CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().ScheduleWork(SetBooleanStateHandler,
                                                                   reinterpret_cast<intptr_t>(&gPendingSet));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Could not schedule the BooleanState change: %" CHIP_ERROR_FORMAT, err.Format());
    }
}
