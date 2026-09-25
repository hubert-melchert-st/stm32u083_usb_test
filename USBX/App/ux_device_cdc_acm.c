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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
typedef struct
{
  UX_SLAVE_CLASS_CDC_ACM *instance;
  UCHAR rx_buffer[64];
  ULONG rx_length;
  ULONG tx_length;
  ULONG tx_actual_length;
  UINT tx_pending;
} usbd_cdc_acm_echo_context_t;

static usbd_cdc_acm_echo_context_t g_cdc_echo_ctx;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

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
  g_cdc_echo_ctx.instance = (UX_SLAVE_CLASS_CDC_ACM *)cdc_acm_instance;
  g_cdc_echo_ctx.rx_length = 0U;
  g_cdc_echo_ctx.tx_length = 0U;
  g_cdc_echo_ctx.tx_actual_length = 0U;
  g_cdc_echo_ctx.tx_pending = 0U;
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
  if (g_cdc_echo_ctx.instance == (UX_SLAVE_CLASS_CDC_ACM *)cdc_acm_instance)
  {
    g_cdc_echo_ctx.instance = UX_NULL;
  }
  g_cdc_echo_ctx.rx_length = 0U;
  g_cdc_echo_ctx.tx_length = 0U;
  g_cdc_echo_ctx.tx_actual_length = 0U;
  g_cdc_echo_ctx.tx_pending = 0U;
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
VOID USBD_CDC_ACM_Process(VOID *arg)
{
  UINT status;

  UX_PARAMETER_NOT_USED(arg);

  if (g_cdc_echo_ctx.instance == UX_NULL)
  {
    return;
  }

  if (g_cdc_echo_ctx.tx_pending == 0U)
  {
    status = ux_device_class_cdc_acm_read_run(g_cdc_echo_ctx.instance,
                                              g_cdc_echo_ctx.rx_buffer,
                                              sizeof(g_cdc_echo_ctx.rx_buffer),
                                              &g_cdc_echo_ctx.rx_length);
    if ((status == UX_STATE_NEXT) && (g_cdc_echo_ctx.rx_length > 0U))
    {
      g_cdc_echo_ctx.tx_length = g_cdc_echo_ctx.rx_length;
      g_cdc_echo_ctx.tx_actual_length = 0U;
      g_cdc_echo_ctx.tx_pending = 1U;
    }
    else if (status < UX_STATE_NEXT)
    {
      g_cdc_echo_ctx.rx_length = 0U;
      g_cdc_echo_ctx.tx_length = 0U;
      g_cdc_echo_ctx.tx_actual_length = 0U;
      g_cdc_echo_ctx.tx_pending = 0U;
      return;
    }
  }

  if (g_cdc_echo_ctx.tx_pending != 0U)
  {
    status = ux_device_class_cdc_acm_write_run(g_cdc_echo_ctx.instance,
                                               g_cdc_echo_ctx.rx_buffer,
                                               g_cdc_echo_ctx.tx_length,
                                               &g_cdc_echo_ctx.tx_actual_length);
    if (status == UX_STATE_NEXT)
    {
      g_cdc_echo_ctx.rx_length = 0U;
      g_cdc_echo_ctx.tx_length = 0U;
      g_cdc_echo_ctx.tx_actual_length = 0U;
      g_cdc_echo_ctx.tx_pending = 0U;
    }
    else if (status < UX_STATE_NEXT)
    {
      g_cdc_echo_ctx.rx_length = 0U;
      g_cdc_echo_ctx.tx_length = 0U;
      g_cdc_echo_ctx.tx_actual_length = 0U;
      g_cdc_echo_ctx.tx_pending = 0U;
    }
  }
}
/* USER CODE END 1 */
