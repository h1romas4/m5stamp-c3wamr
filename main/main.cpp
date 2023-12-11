#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "Arduino.h"
#include "SPIFFS.h"
#include "esp_log.h"
#include "c3dev_board.h"

#include "wasm_c_api.h"

#define IWASM_MAIN_STACK_SIZE 4096
#define own

static const char *TAG = "main.cpp";

/**
 * for FreeRTOS
 */
extern "C" {
void app_main(void);
}

/**
 * SPI member
 */
SPIClass *spi = &SPI;

/**
 * SPIFFS member
 */
fs::SPIFFSFS SPIFFS_WASM;

/**
 * LCD member
 */
Adafruit_ST7735 tft = Adafruit_ST7735(spi, C3DEV_LCD_CS, C3DEV_LCD_DC, C3DEV_LCD_RST);

/**
 * wasm_functype_new_5_0 (5 agrs function)
 */
static inline own wasm_functype_t* wasm_functype_new_5_0(
  own wasm_valtype_t* p1,
  own wasm_valtype_t* p2,
  own wasm_valtype_t* p3,
  own wasm_valtype_t* p4,
  own wasm_valtype_t* p5
) {
    wasm_valtype_t* ps[5] = {p1, p2, p3, p4, p5};
    wasm_valtype_vec_t params, results;
    wasm_valtype_vec_new(&params, 5, ps);
    wasm_valtype_vec_new_empty(&results);
    return wasm_functype_new(&params, &results);
}

/**
 * start_write (A function to be called from Wasm code)
 */
own wasm_trap_t* start_write_callback(
  const wasm_val_vec_t* args, wasm_val_vec_t* results
) {
    printf("Calling back...\n");
    printf("> Hello World!\n");
    return NULL;
}

/**
 * end_write (A function to be called from Wasm code)
 */
own wasm_trap_t* end_write_callback(
  const wasm_val_vec_t* args, wasm_val_vec_t* results
) {
    printf("Calling back...\n");
    printf("> Hello World!\n");
    return NULL;
}

/**
 * draw_line_callback (A function to be called from Wasm code)
 */
own wasm_trap_t* draw_line_callback(
  const wasm_val_vec_t* args, wasm_val_vec_t* results
) {
    printf("Calling back...\n");
    printf("> Hello World!\n");
    return NULL;
}

/**
 * iwasm_main
 *
 * @see https://github.com/WebAssembly/wasm-c-api/blob/main/example/hello.c
 */
void * iwasm_main(void *arg)
{
    // Initialize.
    ESP_LOGI(TAG, "WAMR Initialize...");
    wasm_engine_t* engine = wasm_engine_new();
    wasm_store_t* store = wasm_store_new(engine);

    // Load binary.
    ESP_LOGI(TAG, "Load Wasm binary...");
    SPIFFS_WASM.begin(false, "/wasm", 4, "wasm");
    File wasm_file = SPIFFS_WASM.open("/3dcube.wasm", "rb");
    size_t file_size = wasm_file.size();
    ESP_LOGI(TAG, ".wasm size: %d", file_size);
    // load wasm binary
    wasm_byte_vec_t binary;
    wasm_byte_vec_new_uninitialized(&binary, file_size);
    if(wasm_file.read((uint8_t *)binary.data, file_size) != file_size) {
        ESP_LOGE(TAG, "Error loading module!");
        return NULL;
    }
    // close file
    wasm_file.close();
    SPIFFS_WASM.end();

    // Validate.
    ESP_LOGI(TAG, "Validating module...");
    if (!wasm_module_validate(store, &binary)) {
        ESP_LOGE(TAG, "Error validating module!");
        return NULL;
    }

    // Compile.
    ESP_LOGI(TAG, "Compiling module...");
    own wasm_module_t* module = wasm_module_new(store, &binary);
    if (!module) {
        ESP_LOGE(TAG, "Error compiling module!");
        return NULL;
    }

    // delete local binary
    wasm_byte_vec_delete(&binary);

    // Create external functions.
    ESP_LOGI(TAG, "Creating callback...");
    // import "c3dev" "start_write"
    own wasm_functype_t* start_write_type = wasm_functype_new_0_0();
    own wasm_func_t* start_write_func = wasm_func_new(store, start_write_type, start_write_callback);
    wasm_functype_delete(start_write_type);
    // import "c3dev" "end_write"
    own wasm_functype_t* end_write_type = wasm_functype_new_0_0();
    own wasm_func_t* end_write_func = wasm_func_new(store, end_write_type, end_write_callback);
    wasm_functype_delete(end_write_type);
    // import "c3dev" "draw_line"
    own wasm_functype_t* draw_line_type = wasm_functype_new_5_0(
        wasm_valtype_new_i32(),
        wasm_valtype_new_i32(),
        wasm_valtype_new_i32(),
        wasm_valtype_new_i32(),
        wasm_valtype_new_i32()
    );
    own wasm_func_t* draw_line_func = wasm_func_new(store, draw_line_type, draw_line_callback);
    wasm_functype_delete(draw_line_type);

    // Instantiate.
    ESP_LOGI(TAG, "Instantiating module...");
    wasm_extern_t* externs[] = {
        wasm_func_as_extern(start_write_func),
        wasm_func_as_extern(draw_line_func),
        wasm_func_as_extern(end_write_func)
    };
    wasm_extern_vec_t imports = WASM_ARRAY_VEC(externs);
    own wasm_instance_t* instance = wasm_instance_new(store, module, &imports, NULL);
    if(!instance) {
        ESP_LOGE(TAG, "Error instantiating module!");
        return NULL;
    }

    // delete local function
    wasm_func_delete(start_write_func);
    wasm_func_delete(end_write_func);
    wasm_func_delete(draw_line_func);

    // Extract export.
    ESP_LOGI(TAG, "Extracting export...");
    own wasm_extern_vec_t exports;
    wasm_instance_exports(instance, &exports);
    if (exports.size == 0) {
        ESP_LOGE(TAG, "Error accessing exports!");
        return NULL;
    }
    const wasm_func_t* run_func = wasm_extern_as_func(exports.data[0]);
    if(run_func == NULL) {
        ESP_LOGE(TAG, "Error accessing export!");
        return NULL;
    }

    // delete local module
    wasm_module_delete(module);
    // delete local instance
    wasm_instance_delete(instance);

    // Call.
    ESP_LOGI(TAG, "Calling export...");
    wasm_val_vec_t args = WASM_EMPTY_VEC;
    wasm_val_vec_t results = WASM_EMPTY_VEC;
    if(wasm_func_call(run_func, &args, &results)) {
        ESP_LOGE(TAG, "Error calling function!");
        return NULL;
    }

    // detele local exports
    wasm_extern_vec_delete(&exports);

    ESP_LOGI(TAG, "Shutting down...");
    wasm_store_delete(store);
    wasm_engine_delete(engine);

    // All done.
    ESP_LOGI(TAG, "Done.");

    return NULL;
}

/**
 * app_main
 *
 * @see https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/product-mini/platforms/esp-idf/main/main.c
 */
void app_main(void)
{
    pthread_t t;
    int res;

    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&tattr, IWASM_MAIN_STACK_SIZE);

    ESP_LOGI(TAG, "heap_caps_get_free_size: %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));

    res = pthread_create(&t, &tattr, iwasm_main, (void *)NULL);
    assert(res == 0);

    res = pthread_join(t, NULL);
    assert(res == 0);

    ESP_LOGI(TAG, "heap_caps_get_free_size: %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}
