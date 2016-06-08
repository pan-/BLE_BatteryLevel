/* mbed Microcontroller Library
 * Copyright (c) 2006-2014 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "BLE.h"
#include "BatteryService.h"

DigitalOut led1(LED1, 1);
Ticker t;
BatteryService *batteryService = NULL;
uint8_t batteryLevel = 50;

const static char     DEVICE_NAME[] = "BATTERY";
static const uint16_t uuid16_list[] = {GattService::UUID_BATTERY_SERVICE};

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *disconnectionParams)
{
    printf("Disconnected handle %u!\n\r", disconnectionParams->handle);
    printf("Restarting the advertising process\n\r");
    BLE::Instance(BLE::DEFAULT_INSTANCE).gap().startAdvertising(); // restart advertising
}

void blink(void)
{
    led1 = !led1;
}

void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
    BLE &ble          = params->ble;
    ble_error_t error = params->error;
    Gap& gap = ble.gap();

    if (error != BLE_ERROR_NONE) {
        return;
    }

    gap.onDisconnection(disconnectionCallback);

    batteryService = new BatteryService(ble, batteryLevel);

    /* setup advertising */
    gap.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    gap.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *) uuid16_list, sizeof(uuid16_list));
    gap.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *) DEVICE_NAME, sizeof(DEVICE_NAME));
    gap.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    gap.setAdvertisingInterval(1000); /* 1000ms */
    gap.startAdvertising();
}

int main(void)
{
    t.attach(blink, 1.0f);

    printf("Initialising the nRF51822\n\r");

    BLE& ble = BLE::Instance(BLE::DEFAULT_INSTANCE);
    ble.init(bleInitComplete);

    /* SpinWait for initialization to complete. This is necessary because the
     * BLE object is used in the main loop below. */
    while (ble.hasInitialized()  == false) { /* spin loop */ }

    while (true) {
        ble.waitForEvent(); // this will return upon any system event (such as an interrupt or a ticker wakeup)

        // the magic battery processing
        batteryLevel++;
        if (batteryLevel > 100) {
            batteryLevel = 20;
        }

        batteryService->updateBatteryLevel(batteryLevel);
    }
}
