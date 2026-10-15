#include "bsp_dac.h"
#include <cmath>

extern "C"
{
#include "stm32f3xx_hal.h"
}

#include "dwt.h"

TIM_HandleTypeDef htim2;
DMA_HandleTypeDef hdma_tim2_up;



void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* TIM2 DMA Init */
    /* TIM2_UP Init */
    hdma_tim2_up.Instance = DMA1_Channel2;
    hdma_tim2_up.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tim2_up.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim2_up.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim2_up.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_tim2_up.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_tim2_up.Init.Mode = DMA_CIRCULAR;
    hdma_tim2_up.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_tim2_up) != HAL_OK)
    {
      BSP::Error_Handler();
    }

    __HAL_LINKDMA(tim_baseHandle,hdma[TIM_DMA_ID_UPDATE],hdma_tim2_up);

  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
}
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(timHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspPostInit 0 */

  /* USER CODE END TIM2_MspPostInit 0 */

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM2 GPIO Configuration
    PA0     ------> TIM2_CH1
    PA1     ------> TIM2_CH2
    PA2     ------> TIM2_CH3
    PA3     ------> TIM2_CH4
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM2_MspPostInit 1 */

  /* USER CODE END TIM2_MspPostInit 1 */
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /* TIM2 DMA DeInit */
    HAL_DMA_DeInit(tim_baseHandle->hdma[TIM_DMA_ID_UPDATE]);
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }
}

void BSP::DacGenerator::init(dmaCallbackType dmaCallback)
{

    float phase = 0.0f;
    float val = 0.0f;

    BSP::DacGenerator::dmaCallback = dmaCallback;

    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 5U, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);


    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 17999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
    {
        Error_Handler();
    }

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if ((&htim2)->Instance == TIM2)
    {

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**TIM2 GPIO Configuration
        PA0     ------> TIM2_CH1
        PA1     ------> TIM2_CH2
        PA2     ------> TIM2_CH3
        PA3     ------> TIM2_CH4
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }

    float arr = static_cast<float>(TIM2->ARR) + 1.0f;

    /* for (uint16_t n = 0; n < static_cast<uint16_t>(CONST::sine_samples); n++)
    {
        phase = 2.0f * M_PI * 5.0f * n / static_cast<float>(CONST::sample_rate);
        val = (arr / 2.0f) + (arr / 2.0f) * sinf(phase);
        signals[n * static_cast<uint16_t>(CONST::number_of_channel) + 0U] = (uint16_t)val;

        phase = 2.0f * M_PI * 10.0f * n / static_cast<float>(CONST::sample_rate);
        val = (arr / 2.0f) + (arr / 2.0f) * sinf(phase);
        signals[n * static_cast<uint16_t>(CONST::number_of_channel) + 1U] = (uint16_t)val;

        phase = 2.0f * M_PI * 15.0f * n / static_cast<float>(CONST::sample_rate);
        val = (arr / 2.0f) + (arr / 2.0f) * sinf(phase);
        signals[n * static_cast<uint16_t>(CONST::number_of_channel) + 2U] = (uint16_t)val;

        phase = 2.0f * M_PI * 15.0f * n / static_cast<float>(CONST::sample_rate);
        val = (arr / 2.0f) + (arr / 2.0f) * sinf(phase);
        signals[n * static_cast<uint16_t>(CONST::number_of_channel) + 3U] = (uint16_t)val;
    } */

    if (HAL_TIM_DMABurst_MultiWriteStart(
            &htim2,
            TIM_DMABASE_CCR1,
            TIM_DMA_UPDATE,
            (const uint32_t *)(&signals[0]),
            (static_cast<uint16_t>(CONST::number_of_channel) - 1) * 256U, // TIM_DMABURSTLENGTH_3TRANSFERS,
            static_cast<uint16_t>(CONST::sine_samples) * static_cast<uint16_t>(CONST::number_of_channel)) != HAL_OK)
    {
        Error_Handler();
    }
}

void BSP::DacGenerator::set_sine(uint8_t freq, uint16_t amp_mV, uint16_t mid_point_mV, CHANNEL ch)
{
    /* float phase = 0.0f;
    float val = 0.0f;

    float arr = static_cast<float>(TIM2->ARR) + 1.0f;

    float mid_point_pr = static_cast<float>(mid_point_mV) / static_cast<float>(CONST::max_out_voltage_mV);
    float amp_pr = static_cast<float>(amp_mV) / static_cast<float>(CONST::max_out_voltage_mV); */

    signals_param[static_cast<uint8_t>(ch)].freq = freq;
    signals_param[static_cast<uint8_t>(ch)].amp_mV = amp_mV;
    signals_param[static_cast<uint8_t>(ch)].mid_point_mV = mid_point_mV;
    signals_param[static_cast<uint8_t>(ch)].phase_step = TWO_PI * static_cast<float>(freq) / static_cast<float>(CONST::sample_rate);

    /* for (uint16_t n = 0; n < static_cast<uint16_t>(CONST::sine_samples); n++)
    {
        phase = 2.0f * M_PI * static_cast<float>(freq) * static_cast<float>(n) / static_cast<float>(CONST::sample_rate);
        val = mid_point_pr * arr + amp_pr * arr * sinf(phase);
        signals[n * static_cast<uint16_t>(CONST::number_of_channel) + static_cast<uint8_t>(ch)] = static_cast<uint16_t>(val);
    } */
}

void BSP::DacGenerator::set_voltage(uint16_t voltage_mV, CHANNEL ch)
{

    float arr = static_cast<float>(TIM2->ARR) + 1.0f;

    float mid_point_pr = static_cast<float>(voltage_mV) / static_cast<float>(CONST::max_out_voltage_mV);

    set_sine(0, 0, voltage_mV, ch);

    for (uint16_t n = 0; n < static_cast<uint16_t>(CONST::sine_samples); n++)
    {
        val = mid_point_pr * arr;
        signals[n * static_cast<uint16_t>(CONST::number_of_channel) + static_cast<uint8_t>(ch)] = static_cast<uint16_t>(val);
    }
}

void BSP::DacGenerator::begin(CHANNEL ch)
{
    if (HAL_TIM_PWM_Start(&htim2, static_cast<uint8_t>(ch) * 4U) != HAL_OK)
    {
        Error_Handler();
    }
}

void BSP::DacGenerator::stop(CHANNEL ch)
{
    if (HAL_TIM_PWM_Stop(&htim2, static_cast<uint8_t>(ch) * 4U) != HAL_OK)
    {
        Error_Handler();
    }
}

void BSP::DacGenerator::draw_buf1()
{
    float arr = static_cast<float>(TIM2->ARR) + 1.0f;

    float mid_point_pr_1 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_1)].mid_point_mV) / static_cast<float>(CONST::max_out_voltage_mV);
    float amp_pr_1 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_1)].amp_mV) / static_cast<float>(CONST::max_out_voltage_mV);

    float mid_point_pr_2 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_2)].mid_point_mV) / static_cast<float>(CONST::max_out_voltage_mV);
    float amp_pr_2 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_2)].amp_mV) / static_cast<float>(CONST::max_out_voltage_mV);

    float mid_point_pr_3 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_3)].mid_point_mV) / static_cast<float>(CONST::max_out_voltage_mV);
    float amp_pr_3 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_3)].amp_mV) / static_cast<float>(CONST::max_out_voltage_mV);

    float mid_point_pr_4 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_4)].mid_point_mV) / static_cast<float>(CONST::max_out_voltage_mV);
    float amp_pr_4 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_4)].amp_mV) / static_cast<float>(CONST::max_out_voltage_mV);


    /* volatile uint32_t count = 0;
    dwtReset(); */

    for (uint16_t i = 0; i < static_cast<uint16_t>(CONST::sine_samples) / 2U; i++)
    {
        signals[i * static_cast<uint16_t>(CONST::number_of_channel) + static_cast<uint16_t>(CHANNEL::CHANNEL_1)] = mid_point_pr_1 * arr + amp_pr_1 * arr * sinf(phase_1);
        phase_1 += signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_1)].phase_step;
        if (phase_1 >= TWO_PI)
            phase_1 -= TWO_PI;

        signals[i * static_cast<uint16_t>(CONST::number_of_channel) + static_cast<uint16_t>(CHANNEL::CHANNEL_2)] = mid_point_pr_2 * arr + amp_pr_2 * arr * sinf(phase_2);
        phase_2 += signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_2)].phase_step;
        if (phase_2 >= TWO_PI)
            phase_2 -= TWO_PI;

        signals[i * static_cast<uint16_t>(CONST::number_of_channel) + static_cast<uint16_t>(CHANNEL::CHANNEL_3)] = mid_point_pr_3 * arr + amp_pr_3 * arr * sinf(phase_3);
        phase_3 += signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_3)].phase_step;
        if (phase_3 >= TWO_PI)
            phase_3 -= TWO_PI;

        signals[i * static_cast<uint16_t>(CONST::number_of_channel) + static_cast<uint16_t>(CHANNEL::CHANNEL_4)] = mid_point_pr_4 * arr + amp_pr_4 * arr * sinf(phase_4);
        phase_4 += signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_4)].phase_step;
        if (phase_4 >= TWO_PI)
            phase_4 -= TWO_PI;
    }

    /* count = dwtGet(); */
    return;
}

void BSP::DacGenerator::draw_buf2()
{
    float arr = static_cast<float>(TIM2->ARR) + 1.0f;

    float mid_point_pr_1 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_1)].mid_point_mV) / static_cast<float>(CONST::max_out_voltage_mV);
    float amp_pr_1 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_1)].amp_mV) / static_cast<float>(CONST::max_out_voltage_mV);

    float mid_point_pr_2 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_2)].mid_point_mV) / static_cast<float>(CONST::max_out_voltage_mV);
    float amp_pr_2 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_2)].amp_mV) / static_cast<float>(CONST::max_out_voltage_mV);

    float mid_point_pr_3 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_3)].mid_point_mV) / static_cast<float>(CONST::max_out_voltage_mV);
    float amp_pr_3 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_3)].amp_mV) / static_cast<float>(CONST::max_out_voltage_mV);

    float mid_point_pr_4 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_4)].mid_point_mV) / static_cast<float>(CONST::max_out_voltage_mV);
    float amp_pr_4 = static_cast<float>(signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_4)].amp_mV) / static_cast<float>(CONST::max_out_voltage_mV);

    /* volatile uint32_t count = 0;
    dwtReset(); */

    for (uint16_t i = static_cast<uint16_t>(CONST::sine_samples) / 2U; i < static_cast<uint16_t>(CONST::sine_samples); i++)
    {
        signals[i * static_cast<uint16_t>(CONST::number_of_channel) + static_cast<uint16_t>(CHANNEL::CHANNEL_1)] = mid_point_pr_1 * arr + amp_pr_1 * arr * sinf(phase_1);
        phase_1 += signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_1)].phase_step;
        if (phase_1 >= TWO_PI)
            phase_1 -= TWO_PI;

        signals[i * static_cast<uint16_t>(CONST::number_of_channel) + static_cast<uint16_t>(CHANNEL::CHANNEL_2)] = mid_point_pr_2 * arr + amp_pr_2 * arr * sinf(phase_2);
        phase_2 += signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_2)].phase_step;
        if (phase_2 >= TWO_PI)
            phase_2 -= TWO_PI;

        signals[i * static_cast<uint16_t>(CONST::number_of_channel) + static_cast<uint16_t>(CHANNEL::CHANNEL_3)] = mid_point_pr_3 * arr + amp_pr_3 * arr * sinf(phase_3);
        phase_3 += signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_3)].phase_step;
        if (phase_3 >= TWO_PI)
            phase_3 -= TWO_PI;

        signals[i * static_cast<uint16_t>(CONST::number_of_channel) + static_cast<uint16_t>(CHANNEL::CHANNEL_4)] = mid_point_pr_4 * arr + amp_pr_4 * arr * sinf(phase_4);
        phase_4 += signals_param[static_cast<uint16_t>(CHANNEL::CHANNEL_4)].phase_step;
        if (phase_4 >= TWO_PI)
            phase_4 -= TWO_PI;
    }
    
    /* count = dwtGet(); */

    return;
}

extern "C"
{

    void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
    {
        BSP::DacGenerator::dmaCallback();
        return;
    }

    void HAL_TIM_PeriodElapsedHalfCpltCallback(TIM_HandleTypeDef *htim)
    {
        BSP::DacGenerator::dmaCallback();
        return;
    }
}