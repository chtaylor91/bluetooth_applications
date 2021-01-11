/**
 * The Bluetooth Developer Studio auto-generated code
 *
 ***************************************************************************************************
 * <b> (C) Copyright 2016 Silicon Labs, http://www.silabs.com </b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 */


/*******************************************************************************
 *******************************   INCLUDES   **********************************
 ******************************************************************************/
#include <string.h>
#include <stdio.h>
#include "ble_att_handler.h"
#include "gatt_db.h"
#include "sl_bt_api.h"
#include "pulse_oximeter.h"
#include "hrm_app.h"
#include "app_timer.h"

#define ATT_WRITE_NOT_PERMITTED 0x03

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
// TODO:: Fill const values


/*******************************************************************************
 *******************************   TYPEDEFS   **********************************
 ******************************************************************************/
typedef struct
{
  plxSpotcheckMeasurement_t plx_spot_check_measurement;
  plxContinuousMeasurement_t plx_continuous_measurement;
  plxFeatures_t plx_features;
  plxProcedure_t plx_record_access_control_point;
} pulse_oximeter_t;

/*******************************************************************************
 *****************************   LOCAL DATA   **********************************
 ******************************************************************************/
static pulse_oximeter_t service_data;
static bool notifications_enabled = false;

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * @brief
 *   Service Pulse Oximeter initialization
 * @return
 *   None
 ******************************************************************************/
void pulse_oximeter_init(void)
{
  //TODO:: Add suitable initialization for service

  //ctsDateTime_t time = {2018, 7, 31, 15, 27, 40};
  //service_data.plx_spot_check_measurement.timestamp = time;
  service_data.plx_spot_check_measurement.flags = 0;  // Only present SpO2 and PR
  service_data.plx_spot_check_measurement.SpO2PRSpotcheck.SpO2 = 0;
  service_data.plx_spot_check_measurement.SpO2PRSpotcheck.PR = 0;

  service_data.plx_continuous_measurement.flags = 0;  // Only present Normal SpO2 and PR
  service_data.plx_continuous_measurement.SpO2PRNormal.SpO2 = 0;
  service_data.plx_continuous_measurement.SpO2PRNormal.PR = 0;

  service_data.plx_features.supportedFeatures = 0;  // None

  service_data.plx_record_access_control_point.opCode = 0;
  service_data.plx_record_access_control_point.operand.numberOfRecords = 0;
  service_data.plx_record_access_control_point.operand.responseCode.reqOpCode = 0;
  service_data.plx_record_access_control_point.operand.responseCode.rspCodeValue = 0;
  service_data.plx_record_access_control_point.plxOperator = 0;
}

/*******************************************************************************
 * @brief
 *   Function to handle read data
 * @param[in] evt
 *   Gecko event
 * @return
 *   None
 ******************************************************************************/
void pulse_oximeter_read_callback(sl_bt_msg_t *evt)
{
  uint16_t characteristicSize = 0;
  const uint8_t *characteristicPtr = NULL;

  // TODO:: Add your own code here.

  switch(evt->data.evt_gatt_server_user_read_request.characteristic)
  {

	// PLX features value read
	case gattdb_plx_features:
	{
	  characteristicSize = sizeof(service_data.plx_features);
	  characteristicPtr = (const uint8_t *)&service_data.plx_features;
	}
	  break;

	// Do nothing
	default:
	  break;
  }

  // Send response
  ble_att_send_data(evt->data.evt_gatt_server_user_read_request.connection,
					  evt->data.evt_gatt_server_user_read_request.characteristic,
					  characteristicPtr, characteristicSize);
}

/*******************************************************************************
 * @brief
 *   Function to handle write data
 * @param[in] evt
 *   Gecko event
 * @return
 *   None
 ******************************************************************************/
void pulse_oximeter_write_callback(sl_bt_msg_t *evt)
{
  uint8_t responseCode = 0;
  switch(evt->data.evt_gatt_server_user_write_request.characteristic)
  {
	// Record access control point characteristic written
	case gattdb_record_access_control_point:
	{
	  memcpy((uint8_t *)&service_data.plx_record_access_control_point, evt->data.evt_gatt_server_user_write_request.value.data, evt->data.evt_gatt_server_user_write_request.value.len);
	  // TODO:: Add your own code here.
	}
	break;

	// Write operation not permitted by default
	default:
	{
	  responseCode = ATT_WRITE_NOT_PERMITTED;
	}
	break;
  }

  // TODO:: Add your own code here.

  // Send response
  sl_bt_gatt_server_send_user_write_response(evt->data.evt_gatt_server_user_write_request.connection,
												 evt->data.evt_gatt_server_user_write_request.characteristic,
												 responseCode);
}

/*******************************************************************************
 * @brief
 *   Function to handle disconnect event
 * @param[in] evt
 *   Gecko event
 * @return
 *   None
 ******************************************************************************/
void pulse_oximeter_disconnect_event(sl_bt_msg_t *evt)
{
  (void)evt;
  // TODO:: Add your own code here.
  // stop timer for indicate and notify
  if (notifications_enabled == true)
  {
    notifications_enabled = false;
    sl_bt_system_set_soft_timer(0,PULSE_OXIMETER_TIMER,0);
  }
}

/*******************************************************************************
 * @brief
 *   Function to handle gecko_evt_gatt_server_characteristic_status_id event
 * @param[in] evt
 *   Gecko event
 * @return
 *   None
 ******************************************************************************/
void pulse_oximeter_characteristic_status(sl_bt_msg_t *evt)
{
  uint8_t send_data[5];

  send_data[0] = service_data.plx_continuous_measurement.flags; // 0


  service_data.plx_continuous_measurement.SpO2PRNormal.SpO2 = (uint16_t)hrm_get_spo2();
  service_data.plx_continuous_measurement.SpO2PRNormal.PR = (uint16_t)hrm_get_heart_rate();
  send_data[1] = service_data.plx_continuous_measurement.SpO2PRNormal.SpO2 & 0xff;
  send_data[2] = (service_data.plx_continuous_measurement.SpO2PRNormal.SpO2 >> 8) & 0xff;
  send_data[3] = service_data.plx_continuous_measurement.SpO2PRNormal.PR & 0xff;
  send_data[4] = (service_data.plx_continuous_measurement.SpO2PRNormal.PR >> 8) & 0xff;

  // Notification or Indication status changed for PLX continuous measurement
  if (evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_plx_continuous_measurement
      && evt->data.evt_gatt_server_characteristic_status.status_flags == gatt_server_client_config )
  {
    if (evt->data.evt_gatt_server_characteristic_status.client_config_flags) // Notification or Indication - enabled
    {
      //Start a software timer 500ms interval
        sl_bt_system_set_soft_timer(16384,PULSE_OXIMETER_TIMER,0);

      // TODO:: Add your own code here.
      //sl_bt_gatt_server_send_characteristic_notification(0xFF,
        sl_bt_gatt_server_send_notification(evt->data.evt_gatt_server_characteristic_status.connection,
               evt->data.evt_gatt_server_characteristic_status.characteristic,
               5,
               send_data);
      notifications_enabled = true;
    }
    else // Notification or Indication - disabled
    {
      // TODO:: Add your own code here.
      notifications_enabled = false;
      //Stop the software timer
      sl_bt_system_set_soft_timer(0,PULSE_OXIMETER_TIMER,0);
    }
  }
}

/*******************************************************************************
 * @brief
 *   Function to update SpO2 data
 * @return
 *   None
 ******************************************************************************/
void pulse_oximeter_send_new_data(uint8_t connect)
{
  uint8_t send_data[5];

  send_data[0] = service_data.plx_continuous_measurement.flags; // 0


  service_data.plx_continuous_measurement.SpO2PRNormal.SpO2 = (uint16_t)hrm_get_spo2();
  service_data.plx_continuous_measurement.SpO2PRNormal.PR = (uint16_t)hrm_get_heart_rate();
  send_data[1] = service_data.plx_continuous_measurement.SpO2PRNormal.SpO2 & 0xff;
  send_data[2] = (service_data.plx_continuous_measurement.SpO2PRNormal.SpO2 >> 8) & 0xff;
  send_data[3] = service_data.plx_continuous_measurement.SpO2PRNormal.PR & 0xff;
  send_data[4] = (service_data.plx_continuous_measurement.SpO2PRNormal.PR >> 8) & 0xff;

  if (notifications_enabled == true)
  {
      sl_bt_gatt_server_send_notification(connect,
		  gattdb_plx_continuous_measurement,
               5,
               send_data);
  }
}
