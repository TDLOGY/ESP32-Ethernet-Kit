#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

// CH390 Driver Callbacks
#include "esp_eth_mac_ch390.h"
#include "esp_eth_phy_ch390.h"

static const char *TAG = "CH390_TEST";

// Define GPIO pins for CH390
#define CH390_MISO_GPIO    11
#define CH390_MOSI_GPIO    12
#define CH390_SCLK_GPIO    13
#define CH390_CS_GPIO      14
#define CH390_INT_GPIO     10   
#define CH390_RST_GPIO     -1   

#define CH390_SPI_HOST     SPI2_HOST 

// Handler link Ethernet 
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up!");
        ESP_LOGI(TAG, "Địa chỉ MAC: %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Driver Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Driver Stopped");
        break;
    default:
        break;
    }
}

// Hanedler event when got IP
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;
    ESP_LOGI(TAG, "~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    ESP_LOGI(TAG, "Successfully got IP Address!");
    ESP_LOGI(TAG, "IP:      " IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "Netmask: " IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "Gateway: " IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

void app_main(void)
{
    ESP_LOGI(TAG, "Test hardware IC CH390...");

    // initialize TCP/IP network interface 
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);

    // create new default event loop that running in background
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    // install ISR service 
    gpio_install_isr_service(0);

    // initize SPI Bus
    spi_bus_config_t buscfg = {
        .miso_io_num = CH390_MISO_GPIO,
        .mosi_io_num = CH390_MOSI_GPIO,
        .sclk_io_num = CH390_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(CH390_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // configure SPI device interface for CH390
    spi_device_interface_config_t devcfg = {
        .mode = 0, 
        .clock_speed_hz = 15 * 1000 * 1000, 
        .spics_io_num = CH390_CS_GPIO,
        .queue_size = 20
    };

    eth_ch390_config_t ch390_config = ETH_CH390_DEFAULT_CONFIG(CH390_SPI_HOST, &devcfg);
    ch390_config.int_gpio_num = CH390_INT_GPIO; 
    
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    mac_config.rx_task_stack_size = 4096; 

    esp_eth_mac_t *mac = esp_eth_mac_new_ch390(&ch390_config, &mac_config);

    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = -1; 
    phy_config.reset_gpio_num = CH390_RST_GPIO;

    esp_eth_phy_t *phy = esp_eth_phy_new_ch390(&phy_config);

    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));

    void *glue = esp_eth_new_netif_glue(eth_handle);
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, glue));

    ESP_LOGI(TAG, "Starting Ethernet driver... Waiting for IP allocation");
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}