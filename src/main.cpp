#include "ssd1306.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "dht.h"
#include <stdio.h>
#include "driver/i2c.h" // Asegúrate de incluir la biblioteca correcta para I2C

#define DHT11_PIN 18
#define OLED_CLK 17
#define OLED_SDA 21
#define I2C_MASTER_FREQ_HZ 100000 // Configura la frecuencia del reloj I2C a 100kHz

extern "C" void app_main(void);

static void init_hw(void)
{
   // Configurar los parámetros del I2C
   i2c_config_t conf;
   conf.mode = I2C_MODE_MASTER;
   conf.sda_io_num = (gpio_num_t)OLED_SDA;
   conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
   conf.scl_io_num = (gpio_num_t)OLED_CLK;
   conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
   conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
   i2c_param_config(I2C_NUM_0, &conf);
   i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);

   ssd1306_128x64_i2c_initEx(OLED_CLK, OLED_SDA, 0);
   printf("OLED initialized\n"); // Mensaje de depuración

   // Agregar un pequeño retardo para asegurar que la pantalla OLED esté lista
   vTaskDelay(100 / portTICK_PERIOD_MS);
}

static void draw_screen(void)
{
   ssd1306_clearScreen();
   ssd1306_setFixedFont(ssd1306xled_font8x16);
   ssd1306_printFixed(0, 0, "Temp", STYLE_NORMAL);
   ssd1306_printFixed(0, 32, "Hum", STYLE_NORMAL);
   printf("Screen drawn\n"); // Mensaje de depuración
}

static void display_reading(int temp, int hum)
{
   char buff[10];
   ssd1306_setFixedFont(ssd1306xled_font6x8);
   sprintf(buff, "%d", temp);
   ssd1306_printFixedN(48, 0, buff, STYLE_BOLD, 2);
   sprintf(buff, "%d", hum);
   ssd1306_printFixedN(48, 32, buff, STYLE_BOLD, 2);
   printf("Displayed temp: %d, hum: %d\n", temp, hum); // Mensaje de depuración
}

static void read_dht11(void* arg)
{
   int16_t humidity = 0, temperature = 0;
   while(1)
   {
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      dht_read_data(DHT_TYPE_DHT11, (gpio_num_t)DHT11_PIN, 
      &humidity, &temperature);
      display_reading(temperature / 10, humidity / 10);  
   }
}

void app_main()
{
   init_hw();
   draw_screen();
   xTaskCreate(read_dht11, "dht11", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL);
}
   