#include <i2copt3001_header.h>


#include "ti_drivers_config.h"
#include <ti/sail/bme280/bme280.h>
#include <string.h>

s32    g_s32ActualTemp   = 0;
u32    g_u32ActualPress  = 0;
u32    g_u32ActualHumity = 0;

I2C_Handle      i2c;
I2C_Params      i2cParams;

pthread_t alertTask;
pthread_attr_t       pAttrs;
int             retc;

extern s32 bme280_data_readout_template(I2C_Handle i2cHndl);

/*
 *  ======== mainThread ========
 */

float Raw_Temperature;
float Raw_lux;

float lux;
float temperature;

enum thresholds {
    max_temperature = 34,
    min_temperature = 30,
    max_lux = 25,
    min_lux = 15
};

#define BUFFER_LENGTH_TEMPERATURE 10 // length of buffer for temperature values
#define BUFFER_LENGTH_LUX 10 // length of buffer for lux values

uint8_t current_measurement;

float moving_average_filter(float raw_value, float buffer[], uint8_t length_of_buffer) {
    uint8_t i = 0;
    float current_value = 0;
    buffer[length_of_buffer - 1] = raw_value;

    for(; i < length_of_buffer; i++) {
        current_value += buffer[i];
    }
    current_value /= length_of_buffer;
    for(i = 0; i < length_of_buffer; i++) {
        buffer[i] = buffer[i+1];
    }
    return current_value;
}

void trigger_func() {

    uint8_t lux_already_on = 0;
    uint8_t temp_already_on = 0;


    if (lux > max_lux && lux_already_on) {
        GPIO_write(GREEN_LED, CONFIG_GPIO_LED_OFF);
        lux_already_on = 0;
    }
    else if (lux < min_lux && !lux_already_on) {
        GPIO_write(GREEN_LED, CONFIG_GPIO_LED_ON);
        lux_already_on = 1;
    }
    if (temperature > max_temperature && temp_already_on) {
        GPIO_write(RED_LED, CONFIG_GPIO_LED_OFF);
        temp_already_on = 0;
    }
    else if (temperature < min_temperature && !temp_already_on) {
        GPIO_write(RED_LED, CONFIG_GPIO_LED_ON);
        temp_already_on = 1;
    }
}

void init_opt30001() {
    OPT3001_init();
    OPT3001_Params_init(&opt3001Params);
    opt3001Handle = OPT3001_open(CONFIG_OPT3001_LIGHT, i2c, &opt3001Params);
    if(opt3001Handle == NULL) {
        while(1);
    }
    sleep(1);
}

void init_display() {
    Display_init();

    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
        while (1);
    }
}

void init_i2c() {
    I2C_init();

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(CONFIG_I2C_OPT, &i2cParams);
    if (i2c == NULL) {
        Display_print0(display, 0, 0, "Error Initializing I2C\n");
        while (1);
    }
}

void init_bme280() {
    /* Initialize the BME Sensor */
    if(BME280_INIT_VALUE != bme280_data_readout_template(i2c))
    {
       Display_print0(display, 0, 0, "Error Initializing bme280\n");
    }
    bme280_set_power_mode(BME280_NORMAL_MODE);
}

void *mainThread(void *arg0) {
    float Filter_Buffer_Temperature[BUFFER_LENGTH_TEMPERATURE] = {0};
    float Filter_Buffer_Lux[BUFFER_LENGTH_LUX] = {0};


    GPIO_init();

    init_display();
    init_i2c();
    init_opt30001();
    init_bme280();

    while(1) {
        if (!OPT3001_getLux(opt3001Handle, &Raw_lux))
            Display_print0(display, 0, 0, "OPT3001 sensor read failed");
        Display_print1(display, 0, 0, "Raw Lux: %f", Raw_lux);

        if(BME280_INIT_VALUE == bme280_read_uncomp_temperature(&g_s32ActualTemp))
        {
            Raw_Temperature = (float)bme280_compensate_temperature_int32(g_s32ActualTemp)/100;
            Display_print1(display, 0, 0, "Raw Temperature: %f\n", Raw_Temperature);
        }
        else
        {
            Display_print0(display, 0, 0, "Error reading from the bme280 sensor\n");
        }
        temperature = moving_average_filter(Raw_Temperature, Filter_Buffer_Temperature, BUFFER_LENGTH_TEMPERATURE);
        lux = moving_average_filter(Raw_lux, Filter_Buffer_Lux, BUFFER_LENGTH_LUX);
        trigger_func();
        sleep(2);
    }
}

