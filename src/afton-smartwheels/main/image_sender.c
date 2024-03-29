#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_camera.h"

static const char *TAG = "HTTP_CLIENT";

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    // Implement event handling
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
        break;
         // handle other casez 
        default:
            ESP_LOGI(TAG, "Other event id: %d", evt->event_id);
            break;
    }
    return ESP_OK;
}

void send_image_to_server(const char* image_path)
{
    FILE* file = fopen(image_path, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(file_size);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for image buffer");
        fclose(file);
        return;
    }

    fread(buffer, file_size, 1, file);
    fclose(file);

    esp_http_client_config_t config = {
        .url = "http://192.168.1.2:5000/upload",
        .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // POST Request
    esp_http_client_set_url(client, "http://192.168.1.2:5000/upload");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "image/jpeg");
    esp_http_client_set_post_field(client, buffer, file_size);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int status_code = (int) esp_http_client_get_status_code(client);
        int content_length = (int) esp_http_client_get_content_length(client);
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
                 status_code,
                 content_length);
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    free(buffer);
}
