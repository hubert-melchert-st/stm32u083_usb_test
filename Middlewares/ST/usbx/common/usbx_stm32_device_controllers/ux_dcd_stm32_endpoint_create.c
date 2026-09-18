/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** USBX Component                                                        */
/**                                                                       */
/**   STM32 Controller Driver                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define UX_SOURCE_CODE
#define UX_DCD_STM32_SOURCE_CODE


/* Include necessary system files.  */

#include "ux_api.h"
#include "ux_dcd_stm32.h"
#include "ux_device_stack.h"

volatile ULONG dbg_usb_ep_create_calls;
volatile ULONG dbg_usb_last_ep_create_addr;
volatile ULONG dbg_usb_last_ep_create_mps;
volatile ULONG dbg_usb_last_ep_create_type;
volatile ULONG dbg_usb_ep_open_calls;
volatile ULONG dbg_usb_last_ep_open_addr;


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_dcd_stm32_endpoint_create                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function will create a physical endpoint.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_stm32                             Pointer to device controller  */
/*    endpoint                              Pointer to endpoint container */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    HAL_PCD_EP_Open                       Open endpoint                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    STM32 Controller Driver                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s), used ST  */
/*                                            HAL library to drive the    */
/*                                            controller,                 */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _ux_dcd_stm32_endpoint_create(UX_DCD_STM32 *dcd_stm32, UX_SLAVE_ENDPOINT *endpoint)
{

UX_DCD_STM32_ED     *ed;
ULONG               stm32_endpoint_index;
    dbg_usb_ep_create_calls++;
    dbg_usb_last_ep_create_addr = endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress;
    dbg_usb_last_ep_create_mps = endpoint -> ux_slave_endpoint_descriptor.wMaxPacketSize;
    dbg_usb_last_ep_create_type = endpoint -> ux_slave_endpoint_descriptor.bmAttributes & UX_MASK_ENDPOINT_TYPE;


    /* The endpoint index in the array of the STM32 must match the endpoint number.  */
    stm32_endpoint_index =  endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & ~UX_ENDPOINT_DIRECTION;

    /* Get STM32 ED.  */
    ed = _stm32_ed_get(dcd_stm32, endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress);

    if (ed == UX_NULL)
        return(UX_NO_ED_AVAILABLE);

    /* Check the endpoint status, if it is free, reserve it. If not reject this endpoint.  */
    if ((ed -> ux_dcd_stm32_ed_status & UX_DCD_STM32_ED_STATUS_USED) == 0)
    {

        /* We can use this endpoint.  */
        ed -> ux_dcd_stm32_ed_status |=  UX_DCD_STM32_ED_STATUS_USED;

        /* Keep the physical endpoint address in the endpoint container.  */
        endpoint -> ux_slave_endpoint_ed =  (VOID *) ed;

        /* Save the endpoint pointer.  */
        ed -> ux_dcd_stm32_ed_endpoint =  endpoint;

        /* And its index.  */
        ed -> ux_dcd_stm32_ed_index =  stm32_endpoint_index;

        /* And its direction.  */
        ed -> ux_dcd_stm32_ed_direction =  endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & UX_ENDPOINT_DIRECTION;

        /* Check if it is non-control endpoint.  */
        if (stm32_endpoint_index != 0)
        {

            /* Open the endpoint.  */
            dbg_usb_ep_open_calls++;
            dbg_usb_last_ep_open_addr = endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress;
            HAL_PCD_EP_Open(dcd_stm32 -> pcd_handle, endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress,
                            endpoint -> ux_slave_endpoint_descriptor.wMaxPacketSize,
                            endpoint -> ux_slave_endpoint_descriptor.bmAttributes & UX_MASK_ENDPOINT_TYPE);
        }

        /* Return successful completion.  */
        return(UX_SUCCESS);
    }

    /* Return an error.  */
    return(UX_NO_ED_AVAILABLE);
}
