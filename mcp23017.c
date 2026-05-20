/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : MCP23017 Control - STM32F411 Black Pill
  *                   SCL: PB6 | SDA: PB7
  *                   Inputs: PB0, PB1, PB2 (DIP Switches to GND)
  *                   Output: PB7 (LED Active-High)
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

I2C_HandleTypeDef hi2c1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);

/* USER CODE BEGIN 0 */
// 8-bit shifted I2C address for the default green module configuration (0x20 << 1)
#define MCP23017_ADDR     0x40

// MCP23017 Register Addresses for Port B
#define IODIRB            0x01  // Configuration Direction Register (0 = Output, 1 = Input)
#define GPPUB             0x0D  // Configuration Pull-Up Resistor Register (1 = Enabled)
#define GPIOB             0x13  // Data Register (Reads Inputs / Writes Outputs)

void MCP23017_Init(void)
{
    uint8_t config_data;

    // 1. Set Directions: PB0, PB1, PB2 as Inputs (1) | PB7 as Output (0)
    // Binary: 0b00000111 -> Hex: 0x07
    config_data = 0x07;
    HAL_I2C_Mem_Write(&hi2c1, MCP23017_ADDR, IODIRB, I2C_MEMADD_SIZE_8BIT, &config_data, 1, HAL_MAX_DELAY);

    // 2. Enable Internal Pull-Ups for the input pins (PB0, PB1, PB2)
    // Binary: 0b00000111 -> Hex: 0x07
    config_data = 0x07;
    HAL_I2C_Mem_Write(&hi2c1, MCP23017_ADDR, GPPUB, I2C_MEMADD_SIZE_8BIT, &config_data, 1, HAL_MAX_DELAY);
}
/* USER CODE END 0 */

int main(void)
{
    /* USER CODE BEGIN 1 */
    /* USER CODE END 1 */

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();

    /* USER CODE BEGIN 2 */
    HAL_Delay(500);     // Let the power rails settle
    MCP23017_Init();    // Send configuration data to the chip
    /* USER CODE END 2 */

    /* USER CODE BEGIN WHILE */
    uint8_t port_b_state = 0;
    uint8_t updated_output = 0;

    while (1)
    {
        // 1. Read the current status of all pins on Port B
        HAL_I2C_Mem_Read(&hi2c1, MCP23017_ADDR, GPIOB, I2C_MEMADD_SIZE_8BIT, &port_b_state, 1, HAL_MAX_DELAY);

        // 2. Control Logic: Check if the DIP switch on PB0 is turned ON
        // Since switches are connected to GND, an active switch pulls the pin to 0 (LOW)
        if ((port_b_state & 0x01) == 0)
        {
            // Turn the LED on PB7 ON (Set bit 7 to 1 using bitwise OR with 0x80)
            updated_output = port_b_state | 0x80;
        }
        else
        {
            // Turn the LED on PB7 OFF (Clear bit 7 to 0 using bitwise AND with 0x7F)
            updated_output = port_b_state & 0x7F;
        }

        // 3. Write the updated state back to the data register to toggle the LED
        HAL_I2C_Mem_Write(&hi2c1, MCP23017_ADDR, GPIOB, I2C_MEMADD_SIZE_8BIT, &updated_output, 1, HAL_MAX_DELAY);

        HAL_Delay(50);  // Small debounce and stability delay
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

static void MX_I2C1_Init(void)
{
    hi2c1.Instance             = I2C1;
    hi2c1.Init.ClockSpeed      = 100000;
    hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1     = 0;
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2     = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
        Error_Handler();
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin   = GPIO_PIN_13;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        Error_Handler();

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK

                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
        Error_Handler();
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif
