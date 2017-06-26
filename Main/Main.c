#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event_loop.h"
struct wifi_save_config{
	char wifi_name[40];
	char wifi_pass[100];
	char wifi_ip[15];
	char wifi_gateway[15];
	char wifi_subnet[15];
} wifi_save;

#if CONFIG_POWER_SAVE_MODEM
#define DEFAULT_PS_MODE WIFI_PS_MODEM
#elif CONFIG_POWER_SAVE_NONE
#define DEFAULT_PS_MODE WIFI_PS_NONE
#else
#define DEFAULT_PS_MODE WIFI_PS_NONE
#endif /*CONFIG_POWER_SAVE_MODEM*/
static const char *TAG = "power_save";

void Save_data(char *name, char *data);
char* load_data(char *name);

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {

		case SYSTEM_EVENT_AP_START: {
	        ESP_LOGD(TAG, "SYSTEM_EVENT_AP_START");
	        break;
	    }
	    case SYSTEM_EVENT_AP_STOP: {
	        ESP_LOGD(TAG, "SYSTEM_EVENT_AP_STOP");
	        break;
	    }
	    case SYSTEM_EVENT_AP_STACONNECTED: {
	        system_event_ap_staconnected_t *staconnected = &event->event_info.sta_connected;
	        ESP_LOGD(TAG, "SYSTEM_EVENT_AP_STACONNECTED, mac:" MACSTR ", aid:%d", \
	                   MAC2STR(staconnected->mac), staconnected->aid);
	        break;
	    }
	    case SYSTEM_EVENT_AP_STADISCONNECTED: {
	        system_event_ap_stadisconnected_t *stadisconnected = &event->event_info.sta_disconnected;
	        ESP_LOGD(TAG, "SYSTEM_EVENT_AP_STADISCONNECTED, mac:" MACSTR ", aid:%d", \
	                   MAC2STR(stadisconnected->mac), stadisconnected->aid);
	        break;
	    }
	    case SYSTEM_EVENT_STA_START:
			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
			ESP_ERROR_CHECK(esp_wifi_connect());
			break;
	    case SYSTEM_EVENT_STA_GOT_IP:
			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
			ESP_LOGI(TAG, "got ip:%s\n",
			ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
	        break;
	    case SYSTEM_EVENT_STA_DISCONNECTED:
			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
			ESP_ERROR_CHECK(esp_wifi_connect());
			break;
	    default:
	        break;
    }
    return ESP_OK;
}

/*init wifi as sta and set power save mode*/
static void wifi_power_save(void)
{
    tcpip_adapter_init();
    	ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
	tcpip_adapter_ip_info_t info;
	memset(&info, 0, sizeof(info));
	IP4_ADDR(&info.ip, 192, 168, 10, 1);
	IP4_ADDR(&info.gw, 192, 168, 10, 1);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info));
	ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
	
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_sta_config = {
		.sta = {
	    	.ssid = "tam",
	    	.password = "tam"		
	    },
	};

		
	//memset(&wifi_ap_config, 0, sizeof(wifi_ap_config));
   //strcpy((char *)wifi_ap_config.ap.ssid, "ESP32_mHome\0");
    strncpy((char *)wifi_sta_config.sta.ssid, wifi_save.wifi_name,sizeof(wifi_save.wifi_name));
    strncpy((char *)wifi_sta_config.sta.password, wifi_save.wifi_pass,sizeof(wifi_save.wifi_pass));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    wifi_config_t wifi_ap_config  = {
	    .ap = {
			.ssid = "ESP32\0",
			.ssid_len = 0,
			.password = "123789456",
			.channel = 6,
			.authmode = WIFI_AUTH_WPA2_PSK,
			.ssid_hidden = 0,
			.beacon_interval = 100,
			.max_connection = 4,
			},
	};
	//wifi_ap_config.ap.ssid_len=sizeof(wifi_ap_config.ap.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config) );
   // ESP_LOGI(TAG, esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    //ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    printf("Starting access point, SSID=%s\n", wifi_ap_config.ap.ssid);
	//ESP_ERROR_CHECK( esp_wifi_connect() );
   // ESP_LOGI(TAG, "esp_wifi_set_ps().");
    //esp_wifi_set_ps(DEFAULT_PS_MODE);
}

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

    strncpy(wifi_save.wifi_name, load_data("wifi_name"),sizeof(wifi_save.wifi_name));
    strncpy(wifi_save.wifi_pass, load_data("wifi_pass"),sizeof(wifi_save.wifi_pass));

    printf("Restart counter = %s\n", wifi_save.wifi_name);
    printf("Restart counter = %s\n", wifi_save.wifi_pass);
    Save_data("wifi_name","mHomeRD");
    Save_data("wifi_pass","123789456");
    wifi_power_save();
    for (int i = 120; i >= 0; i--) {
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