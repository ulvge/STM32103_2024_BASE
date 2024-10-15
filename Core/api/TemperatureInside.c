
#define GLOBALS_TEMPINSIDE_DRV

#include "FreeRTOS.h"
#include "Types.h"
#include "debug_print.h"
#include "initcall.h"
#include "main.h"
#include "shell.h"
#include "stm32f1xx.h"
#include "task.h"

#define ADCx ADC1
#define TEMPTURE_RAWDATA_LENTH 8
static INT16U Temp_RawData[TEMPTURE_RAWDATA_LENTH];
/*
* STM32F103VET6处理器内部内置了一个温度传感器，该温度传感器的在内部和ADC1_IN16输入通道相连接，此通道把传感器输出的电压转换成数字值。
需要注意的是，内部温度传感器更适用于检测温度的变化，而不是测量绝对的温度，如果需要测量精确的温度，应该使用外置的温度传感器。
ADC输出的数字值和温度之间的对应关系如下
――――――――――――――――
版权声明：本文为CSDN博主「梦影樱飞」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/aricchen77/article/details/112443564
*/

DMA_HandleTypeDef hdma_adc1;
static ADC_HandleTypeDef AdcHandle;
void ADC_init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    /* USER CODE BEGIN ADC1_Init 1 */

    /* USER CODE END ADC1_Init 1 */

    /** Common config
     */
    AdcHandle.Instance = ADC1;
    AdcHandle.Init.ScanConvMode = ADC_SCAN_DISABLE;
    AdcHandle.Init.ContinuousConvMode = ENABLE;
    AdcHandle.Init.DiscontinuousConvMode = DISABLE;
    AdcHandle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    AdcHandle.Init.NbrOfConversion = 1;
    if (HAL_ADC_Init(&AdcHandle) != HAL_OK) {
        Error_Handler();
    }

    /** Configure Regular Channel
     */
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
    if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK) {
        Error_Handler();
    }
    /* ### - 3 - Start ADC calibaration ########################### */
    if (HAL_ADCEx_Calibration_Start(&AdcHandle) != HAL_OK) {
        Error_Handler();
    }
}

__attribute__((unused)) static bool g_adcConvertFinished = false;
__attribute__((unused)) void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    g_adcConvertFinished = true;
}
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        /* USER CODE BEGIN ADC1_MspInit 0 */

        /* USER CODE END ADC1_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_ADC1_CLK_ENABLE();

        /* ADC1 DMA Init */
        /* ADC1 Init */
        hdma_adc1.Instance = DMA1_Channel1;
        hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
        hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;   // ADC data 16bit
        hdma_adc1.Init.MemDataAlignment = (sizeof(Temp_RawData[0]) == 4) ? DMA_MDATAALIGN_WORD : DMA_MDATAALIGN_HALFWORD;          // Temp_RawData data 32bit
        hdma_adc1.Init.Mode = DMA_CIRCULAR;
        hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
        if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);

        /* NVIC configuration for DMA Input data interrupt */
        // HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, IRQHANDLER_PRIORITY_ADC, 0);
        // HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    }
}
static INT32U TemperatureInside_GetAverageAdcVal(INT32U sampleTotalCount)
{
    INT32U adcValSum = 0;
    INT32U adcAverageVal = 0;
    for (INT32U i = 0; i < sampleTotalCount; i++) {
        adcValSum += Temp_RawData[i];
    }
    adcAverageVal = adcValSum / sampleTotalCount;
    return adcAverageVal;
}
float TemperatureInside_GetADCHuman(void)
{
    INT32U averageVal = TemperatureInside_GetAverageAdcVal(TEMPTURE_RAWDATA_LENTH);

    //while (!__HAL_ADC_GET_FLAG(&AdcHandle, ADC_FLAG_EOC)) ; // 等待传输完成
        
    // INT32U tmpVal = (1.43 - ADC_ConvertedValue * 3.3 / 4096) * 1000 / 4.35 + 25;
    // INT32U tmpVal = ((1430 - (ADC_ConvertedValue * 3300 / 4096)) / 4.35 + 25) * 100;
    INT32 vol = 1430 - (averageVal * 3300 / 4096);
    INT32 tmpVal = ((vol * 100) / 4.35) + 2500;

    return tmpVal;
}
static void TemperatureInside_Init()
{
    ADC_init();

    if (HAL_ADC_Start_DMA(&AdcHandle,
                          (uint32_t *)Temp_RawData,
                          TEMPTURE_RAWDATA_LENTH) != HAL_OK) {
    }
}

CoreInitCall(TemperatureInside_Init);

static int GetMcuTemperature(int argc, char *argv[])
{
    INT32 temp = TemperatureInside_GetADCHuman();
    float tempHuman = (float)temp / 100.0;
    LOG_RAW("MCU temperature: %.2f\n", tempHuman);
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) | SHELL_CMD_DISABLE_RETURN, temp, GetMcuTemperature, get Temperature of mcu);
