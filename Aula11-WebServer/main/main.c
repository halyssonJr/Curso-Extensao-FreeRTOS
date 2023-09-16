#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <esp_http_server.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>

#define SSID "Roadhouse Blues - 2.4G"
#define PASSWORD "senhaittnet2021"
#define ESP_MAXIMUM_RETRY 5

#define TAG "Aula 11"
#define RETRY_NUM 5

int wifi_connect_status = 0;
int retry_num = 0;
float temp;
float hum;

char html_page[] = "<!DOCTYPE HTML><html>\n"
                   "<head>\n"
                   "  <title>Web Server</title>\n"
                   "  <meta http-equiv=\"refresh\" content=\"10\">\n"
                   "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                   "</head>\n"
                   "<body>\n"
                   "  <div class=\"topnav\">\n"
                   "    <h3>Cursor de Extensao Web Server</h3>\n"
                   "  </div>\n"
                   "  <div class=\"content\">\n"
                   "    <div class=\"cards\">\n"
                   "      <div class=\"card temperatura\">\n"
                   "        <h4>Temperatura</h4><p><span>%.2f&deg;C</span></p>\n"
                   "      </div>\n"
                   "      <div class=\"card umidade\">\n"
                   "        <h4>Umidade</h4><p><span>%.2f</span> &percnt;</span></p>\n"
                   "      </div>\n"
                   "    </div>\n"
                   "  </div>\n"
                   "</body>\n"
                   "</html>";

void readings()
{
    temp = 29.0;
    hum = 47.1;
    vTaskDelay(pdMS_TO_TICKS(2000));	
}

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{

    if (event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Conectando ...");
        esp_wifi_connect();
    }

    else if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(TAG, "Conectado a rede local");
        wifi_connect_status = 1;
    }

    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "Desconectado a rede local");
        wifi_connect_status = 0;

        if (retry_num < RETRY_NUM)
        {
            esp_wifi_connect();
            retry_num++;
            ESP_LOGI(TAG, "Reconectando ...");
        }
    }
    
    else if (event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG, "IP: " IPSTR,IP2STR(&event->ip_info.ip));
        retry_num = 0;
    }
}


void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    wifi_config_t wifi_config = 
    {
        .sta = 
        {
            .ssid = SSID,
            .password = PASSWORD,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "SSID:%s  password:%s",SSID,PASSWORD);
}

esp_err_t web_page(httpd_req_t *req)
{
    
    readings();
    char response_data[sizeof(html_page) + 50];
    memset(response_data, 0x00, sizeof(response_data));
    
    sprintf(response_data, html_page, temp, hum);
    return httpd_resp_send(req, response_data, HTTPD_RESP_USE_STRLEN);
}

esp_err_t req_handler(httpd_req_t *req)
{
    return web_page(req);
}

httpd_uri_t uri_get = 
{
    .uri = "/",
    .method = HTTP_GET, // Método de requisição
    .handler = req_handler, // Handler 
    .user_ctx = NULL // Pointeiro usado para receber o contexto (dados) oferecido pelo handler  
};

httpd_handle_t setup_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) // Inicializa o server
    {
        httpd_register_uri_handler(server, &uri_get); // Regitra o handler e a URI
    }

    return server;
}

void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init ();
    
    vTaskDelay(pdMS_TO_TICKS(10000));

    if (wifi_connect_status)
    {
        setup_server();
        ESP_LOGI(TAG, "Web Server inicializado");
    }
    else
    {
        ESP_LOGI(TAG, "Falha ao conectar à rede local");
    }
}                  
