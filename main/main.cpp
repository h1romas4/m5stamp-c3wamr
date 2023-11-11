#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "Arduino.h"
#include "esp_log.h"
#include "wasm_c_api.h"

static const char *TAG = "main.cpp";

/**
 * @see https://github.com/WebAssembly/wasm-c-api/blob/main/example/hello.c
 */
void setup(void)
{
    ESP_LOGI(TAG, "heap_caps_get_free_size: %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));

    ESP_LOGI(TAG, "WAMR Initialize");
    wasm_engine_t* engine = wasm_engine_new();
    wasm_store_t* store = wasm_store_new(engine);

    ESP_LOGI(TAG, "WAMR Shutdown");
    wasm_store_delete(store);
    wasm_engine_delete(engine);

    ESP_LOGI(TAG, "heap_caps_get_free_size: %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

void loop(void)
{

}
