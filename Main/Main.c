#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "nvs.h"
struct wifi_save_config{
	char wifi_name[40];
	char wifi_pass[100];
	char wifi_ip[15];
	char wifi_gateway[15];
	char wifi_subnet[15];
} wifi_save;
void Save_data(char *name, char *data);
char* load_data(char *name);
void app_main()
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        const esp_partition_t* nvs_partition = esp_partition_find_first(
                ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
        assert(nvs_partition && "partition table must have an NVS partition");
        ESP_ERROR_CHECK( esp_partition_erase_range(nvs_partition, 0, nvs_partition->size) );
        // Retry nvs_flash_init
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    char *trave=load_data("wifi_name");
    int size=sizeof(trave);
    	strncpy(wifi_save.wifi_name, trave,size);
    	printf("Restart counter = %s\n", wifi_save.wifi_name);
        Save_data("wifi_name","ssss");
    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
char* load_data(char *name){
	esp_err_t err;
	size_t required_size;
    char* tam = "";
	nvs_handle my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);  // Nếu thành công dữ liệu sẽ được lưu vào trong biến my_handle
    if (err != ESP_OK) {
        printf("Error (%d) opening NVS handle!\n", err);
    } else {
        printf("Done\n");
        err = nvs_get_str(my_handle, name, NULL, &required_size);  // Đọc biến my_handle và tìm biến có tên là "restart_conter" lưu vào biến restart_counter
        switch (err) {
            case ESP_OK:
                tam = malloc(required_size);
            	err = nvs_get_str(my_handle, name, tam, &required_size);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default :
                printf("Error (%d) reading!\n", err);
        }
        nvs_close(my_handle); // Đóng biến my_handle
    }

    return(tam);
}
void Save_data(char *name, char *data){
    nvs_handle my_handle;
    esp_err_t err;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);  // Nếu thành công dữ liệu sẽ được lưu vào trong biến my_handle
    if (err != ESP_OK) {
        printf("Error (%d) opening NVS handle!\n", err);
    } else {
        err = nvs_set_str(my_handle, name, data);  //Set biến restart_conter
        err = nvs_commit(my_handle);  // Save lại biến my_handle
        // Close
        nvs_close(my_handle); // Đóng biến my_handle
	}
}