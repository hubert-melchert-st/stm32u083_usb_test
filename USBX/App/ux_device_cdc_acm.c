/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_cdc_acm.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "ux_device_cdc_acm.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CDC_ECHO_BUFFER_SIZE  64U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static UX_SLAVE_CLASS_CDC_ACM *cdc_acm = UX_NULL;
static UCHAR echo_rx[CDC_ECHO_BUFFER_SIZE];
static UCHAR echo_tx[CDC_ECHO_BUFFER_SIZE];
static ULONG echo_tx_len;
static ULONG echo_actual;
static UINT echo_write_pending;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID cdc_echo_reset(VOID);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_CDC_ACM_Activate
  *         This function is called when insertion of a CDC ACM device.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_Activate(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_Activate */
  cdc_acm = (UX_SLAVE_CLASS_CDC_ACM *)cdc_acm_instance;
  cdc_echo_reset();
  HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
  /* USER CODE END USBD_CDC_ACM_Activate */

  return;
}

/**
  * @brief  USBD_CDC_ACM_Deactivate
  *         This function is called when extraction of a CDC ACM device.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_Deactivate(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_Deactivate */
  UX_PARAMETER_NOT_USED(cdc_acm_instance);
  cdc_acm = UX_NULL;
  cdc_echo_reset();
  HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
  /* USER CODE END USBD_CDC_ACM_Deactivate */

  return;
}

/**
  * @brief  USBD_CDC_ACM_ParameterChange
  *         This function is invoked to manage the CDC ACM class requests.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_ParameterChange(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_ParameterChange */
  UX_PARAMETER_NOT_USED(cdc_acm_instance);
  /* USER CODE END USBD_CDC_ACM_ParameterChange */

  return;
}

/* USER CODE BEGIN 1 */
static VOID cdc_echo_reset(VOID)
{
  echo_tx_len = 0U;
  echo_actual = 0U;
  echo_write_pending = 0U;
}

/**
  * @brief  Cooperative CDC ACM echo: read one FS packet, write it back.
  *         Must be called after ux_device_stack_tasks_run() in the main loop.
  */
VOID USBD_CDC_ACM_WriteReadProcess(VOID)
{
  UINT status;

  if (cdc_acm == UX_NULL)
  {
    return;
  }

  if (echo_write_pending == 0U)
  {
    status = ux_device_class_cdc_acm_read_run(cdc_acm, echo_rx, CDC_ECHO_BUFFER_SIZE, &echo_actual);

    if (status == UX_STATE_NEXT)
    {
      if (echo_actual > 0U)
      {
        if (echo_actual > CDC_ECHO_BUFFER_SIZE)
        {
          echo_actual = CDC_ECHO_BUFFER_SIZE;
        }

        ux_utility_memory_copy(echo_tx, echo_rx, echo_actual);
        echo_tx_len = echo_actual;
        echo_write_pending = 1U;
      }
    }
    else if (status < UX_STATE_NEXT)
    {
      cdc_echo_reset();
    }
  }

  if (echo_write_pending != 0U)
  {
    status = ux_device_class_cdc_acm_write_run(cdc_acm, echo_tx, echo_tx_len, &echo_actual);

    if (status == UX_STATE_NEXT)
    {
      HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
      echo_write_pending = 0U;
      echo_tx_len = 0U;
    }
    else if (status < UX_STATE_NEXT)
    {
      cdc_echo_reset();
    }
  }
}
/* USER CODE END 1 */
