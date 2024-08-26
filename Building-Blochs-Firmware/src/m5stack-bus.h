// #ifdef M5STACK_CORE
// #endif

// #ifdef M5STACK_CORE2
// #endif

#ifdef ARDUINO_M5STACK_CORES3
    #define M5_BUS_ESP_BOOT         GPIO_NUM_0
    #define M5_BUS_PA_SCL           GPIO_NUM_1
    #define M5_BUS_PA_SDA           GPIO_NUM_2
    #define M5_LCD_CS               GPIO_NUM_3
    #define M5_TF_CS                GPIO_NUM_4
    #define M5_BUS_G5               GPIO_NUM_5
    #define M5_BUS_G6               GPIO_NUM_6
    #define M5_BUS_G7               GPIO_NUM_7
    #define M5_BUS_PB_IN            GPIO_NUM_8
    #define M5_BUS_PB_OUT           GPIO_NUM_9
    #define M5_BUS_ADC1             GPIO_NUM_10
    #define M5_BUS_I2C_SYS_SCL      GPIO_NUM_11
    #define M5_BUS_I2C_SYS_SDA      GPIO_NUM_12
    #define M5_BUS_I2S_DAT0         GPIO_NUM_13
    #define M5_BUS_I2S_DAT1         GPIO_NUM_14
    #define M5_BUS_CAM_D6           GPIO_NUM_15
    #define M5_BUS_CAM_D7           GPIO_NUM_16
    #define M5_BUS_PC_TX            GPIO_NUM_17
    #define M5_BUS_PC_RX            GPIO_NUM_18
    #define M5_USB_DU_N             GPIO_NUM_19
    #define M5_USB_DU_P             GPIO_NUM_20
    #define M5_I2C_INT              GPIO_NUM_21
    #define M5_FLASH_CS1            GPIO_NUM_22
    // 23, 24 = 
    // 26 - 32 = FLASH
    #define M5_BUS_I2S_WCK          GPIO_NUM_33
    #define M5_BUS_I2S_BCK          GPIO_NUM_34
    #define M5_BUS_SPI_MISO         GPIO_NUM_35
    #define M5_BUS_SPI_SCK          GPIO_NUM_36
    #define M5_BUS_SPI_MOSI         GPIO_NUM_37
    #define M5_BUS_CAM_HREF         GPIO_NUM_38
    #define M5_BUS_CAM_D2           GPIO_NUM_39
    #define M5_BUS_CAM_D3           GPIO_NUM_40
    #define M5_BUS_CAM_D4           GPIO_NUM_41
    #define M5_BUS_CAM_D5           GPIO_NUM_42
    #define M5_BUS_U0TXD            GPIO_NUM_43
    #define M5_BUS_U0RXD            GPIO_NUM_44
    #define M5_BUS_CAM_PCLK         GPIO_NUM_45
    #define M5_BUS_CAM_VSYNC        GPIO_NUM_46
    #define M5_BUS_CAM_D9           GPIO_NUM_47
    #define M5_BUS_CAM_D8           GPIO_NUM_48
    
    #define M5_BUS_1		        GPIO_NUM_10 // M5_BUS_ADC1
    // #define M5_BUS_2             GND
    #define M5_BUS_3 	            GPIO_NUM_8  // M5_BUS_PB_IN
    // #define M5BUS_4              GND
    // #define M5BUS_5              AXP_PG
    // #define M5BUS_6              GND
    #define M5_BUS_7 	            GPIO_NUM_5  // M5_BUS_G5
    #define M5_BUS_8 	            GPIO_NUM_37 // M5_BUS_SPI_MOSI 	
    #define M5_BUS_9	            GPIO_NUM_9  // M5_BUS_PB_OUT
    #define M5_BUS_10 	            GPIO_NUM_35 // M5_BUS_SPI_MISO
    // #define M5BUS_11	            VCC
    #define M5_BUS_12	            GPIO_NUM_36 // M5_BUS_SPI_SCK
    #define M5_BUS_13	            GPIO_NUM_43 // M5_BUS_U0TXD
    #define M5_BUS_14	            GPIO_NUM_44 // M5_BUS_U0RXD
    #define M5_BUS_15	            GPIO_NUM_17 // M5_BUS_PC_TX
    #define M5_BUS_16	            GPIO_NUM_18 // M5_BUS_PC_RX
    #define M5_BUS_17	            GPIO_NUM_11 // M5_BUS_I2C_SYS_SCL
    #define M5_BUS_18	            GPIO_NUM_12 // M5_BUS_I2C_SYS_SDA
    #define M5_BUS_19	            GPIO_NUM_1  // M5_BUS_PA_SCL
    #define M5_BUS_20	            GPIO_NUM_2  // M5_BUS_PA_SDA
    #define M5_BUS_21	            GPIO_NUM_7  // M5_BUS_G7
    #define M5_BUS_22	            GPIO_NUM_6  // M5_BUS_G6
    #define M5_BUS_23	            GPIO_NUM_0  // M5_BUS_ESP_BOOT
    #define M5_BUS_24	            GPIO_NUM_13 // M5_BUS_I2S_DAT0
    #define M5_BUS_25	            GPIO_NUM_ M5_BUS_I2S_DAT1
    // #define M5BUS_26	            NC
    // #define M5BUS_27	            BUS_OUT
    // #define M5BUS_28	            NC
    // #define M5BUS_29	            VBAT
    // #define M5BUS_30	            NC
#endif // ARDUINO_M5CORE_CORES3

// STEPMOTOR DRIVER V1.1 pins on MBUS
#define M5_STEPMOTORDRIVER_DIR_X    M5_BUS_15   // S1:GPIO17 S2:GPIO14 S3:GPIO17 
#define M5_STEPMOTORDRIVER_STP_X    M5_BUS_16   // S1:GPIO16 S2:GPIO13 S3:GPIO18 
#define M5_STEPMOTORDRIVER_DIR_Y    M5_BUS_21   // S1:GPIO13 S2:GPIO19 S3:GPIO7 
#define M5_STEPMOTORDRIVER_STP_Y    M5_BUS_22   // S1:GPIO12 S2:GPIO27 S3:GPIO6
#define M5_STEPMOTORDRIVER_DIR_Z    M5_BUS_23   // S1:GPIO0  S2:GPIO0  S3:GPIO0
#define M5_STEPMOTORDRIVER_STP_Z    M5_BUS_24   // S1:GPIO15 S2:GPIO2  S3:GPIO13
#define M5_STEPMOTORDRIVER_SDA      M5_BUS_18   // S1:GPIO21 S2:GPIO   S3:GPIO11
#define M5_STEPMOTORDRIVER_SCL      M5_BUS_17   // S1:GPIO22 S2:GPIO   S3:GPIO12
