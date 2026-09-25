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
  UX_SLAVE_CLASS_CDC_ACM_CALLBACK_PARAMETER callbacks;
  UCHAR tx_buffer[64];
  UCHAR pending_buffer[64];
  ULONG pending_length;
  UINT tx_busy;
} usbd_cdc_acm_echo_context_t;

static usbd_cdc_acm_echo_context_t g_cdc_echo_ctx;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static UINT USBD_CDC_ACM_WriteCallback(struct UX_SLAVE_CLASS_CDC_ACM_STRUCT *cdc_acm, UINT status, ULONG length);
static UINT USBD_CDC_ACM_ReadCallback(struct UX_SLAVE_CLASS_CDC_ACM_STRUCT *cdc_acm, UINT status, UCHAR *data_pointer, ULONG length);
static void USBD_CDC_ACM_TrySendPending(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static UINT USBD_CDC_ACM_WriteCallback(struct UX_SLAVE_CLASS_CDC_ACM_STRUCT *cdc_acm, UINT status, ULONG length)
{
  UX_PARAMETER_NOT_USED(cdc_acm);
  UX_PARAMETER_NOT_USED(status);
  UX_PARAMETER_NOT_USED(length);
  g_cdc_echo_ctx.tx_busy = 0U;
  USBD_CDC_ACM_TrySendPending();
  return UX_SUCCESS;
}

static UINT USBD_CDC_ACM_ReadCallback(struct UX_SLAVE_CLASS_CDC_ACM_STRUCT *cdc_acm, UINT status, UCHAR *data_pointer, ULONG length)
{
  UINT write_status;
  ULONG copy_len;

  if ((status != UX_SUCCESS) || (length == 0U) || (data_pointer == UX_NULL))
  {
    return UX_SUCCESS;
  }

  copy_len = length;
  if (copy_len > sizeof(g_cdc_echo_ctx.tx_buffer))
  {
    copy_len = sizeof(g_cdc_echo_ctx.tx_buffer);
  }

  if (g_cdc_echo_ctx.tx_busy != 0U)
  {
    if (g_cdc_echo_ctx.pending_length == 0U)
    {
      _ux_utility_memory_copy(g_cdc_echo_ctx.pending_buffer, data_pointer, copy_len); /* Use case of memcpy is verified. */
      g_cdc_echo_ctx.pending_length = copy_len;
    }
    return UX_SUCCESS;
  }

  _ux_utility_memory_copy(g_cdc_echo_ctx.tx_buffer, data_pointer, copy_len); /* Use case of memcpy is verified. */
  write_status = ux_device_class_cdc_acm_write_with_callback(cdc_acm, g_cdc_echo_ctx.tx_buffer, copy_len);
  if (write_status == UX_SUCCESS)
  {
    g_cdc_echo_ctx.tx_busy = 1U;
  }
  else if (write_status == UX_ERROR)
  {
    _ux_utility_memory_copy(g_cdc_echo_ctx.pending_buffer, data_pointer, copy_len); /* Use case of memcpy is verified. */
    g_cdc_echo_ctx.pending_length = copy_len;
  }
  else
  {
    g_cdc_echo_ctx.pending_length = 0U;
  }

  return UX_SUCCESS;
}

static void USBD_CDC_ACM_TrySendPending(void)
{
  UINT status;

  if ((g_cdc_echo_ctx.instance == UX_NULL) ||
      (g_cdc_echo_ctx.tx_busy != 0U) ||
      (g_cdc_echo_ctx.pending_length == 0U))
  {
    return;
  }

  _ux_utility_memory_copy(g_cdc_echo_ctx.tx_buffer,
                          g_cdc_echo_ctx.pending_buffer,
                          g_cdc_echo_ctx.pending_length); /* Use case of memcpy is verified. */
  status = ux_device_class_cdc_acm_write_with_callback(g_cdc_echo_ctx.instance,
                                                       g_cdc_echo_ctx.tx_buffer,
                                                       g_cdc_echo_ctx.pending_length);
  if (status == UX_SUCCESS)
  {
    g_cdc_echo_ctx.pending_length = 0U;
    g_cdc_echo_ctx.tx_busy = 1U;
  }
  else if (status != UX_ERROR)
  {
    g_cdc_echo_ctx.pending_length = 0U;
  }
}

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
  UINT status;

  g_cdc_echo_ctx.instance = (UX_SLAVE_CLASS_CDC_ACM *)cdc_acm_instance;
  g_cdc_echo_ctx.callbacks.ux_device_class_cdc_acm_parameter_write_callback = USBD_CDC_ACM_WriteCallback;
  g_cdc_echo_ctx.callbacks.ux_device_class_cdc_acm_parameter_read_callback = USBD_CDC_ACM_ReadCallback;
  g_cdc_echo_ctx.pending_length = 0U;
  g_cdc_echo_ctx.tx_busy = 0U;

  status = ux_device_class_cdc_acm_ioctl(g_cdc_echo_ctx.instance,
                                         UX_SLAVE_CLASS_CDC_ACM_IOCTL_TRANSMISSION_START,
                                         &g_cdc_echo_ctx.callbacks);
  if (status != UX_SUCCESS)
  {
    g_cdc_echo_ctx.instance = UX_NULL;
  }
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
    (void)ux_device_class_cdc_acm_ioctl(g_cdc_echo_ctx.instance,
                                        UX_SLAVE_CLASS_CDC_ACM_IOCTL_TRANSMISSION_STOP,
                                        UX_NULL);
    g_cdc_echo_ctx.instance = UX_NULL;
  }
  g_cdc_echo_ctx.pending_length = 0U;
  g_cdc_echo_ctx.tx_busy = 0U;

  UX_PARAMETER_NOT_USED(cdc_acm_instance);
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
  UX_PARAMETER_NOT_USED(arg);

  if (g_cdc_echo_ctx.instance != UX_NULL)
  {
    USBD_CDC_ACM_TrySendPending();
    (void)ux_device_class_cdc_acm_tasks_run(g_cdc_echo_ctx.instance);
  }
}
/* USER CODE END 1 */
