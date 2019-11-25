//###############################################################
//# Copyright (C) 2019, Evocortex GmbH, All rights reserved.    #
//# Further regulations can be found in LICENSE file.           #
//###############################################################

/**
 * @file ToFSensor.cpp
 * @author MBA (info@evocortex.com)
 *
 * @brief Source ToF Sensor
 *
 * @version 1.0
 * @date 2019-10-15
 *
 * @copyright Copyright (c) 2019
 *
 */

/* Includes ----------------------------------------------------------------------*/
#include <evo_tof_interface/ToFSensor.h>
#include <evo_tof_interface/ToFBoard.h>
#include <evo_mbed/tools/Logging.h>
/*--------------------------------------------------------------------------------*/

using namespace evo_mbed;

/* Public Class Functions --------------------------------------------------------*/

ToFSensor::~ToFSensor(void)
{
   release();
}

void ToFSensor::release(void)
{
   if(!_is_initialized)
      return;

   _is_initialized = false;
}

/* !Public Class Functions -------------------------------------------------------*/

/* Private Class Functions -------------------------------------------------------*/

ToFSensor::ToFSensor(const unsigned int id, ToFBoard& board, const bool logging) :
    _id(id), _board(board),
    _com_sts_distance(TOF_SENS_PARAM_BASE_IDX + (id * 1000u) + TOF_STS_DISTANCE_MM,
                      false, uint32_t(0)),
    _com_sigma_mm(TOF_SENS_PARAM_BASE_IDX + (id * 1000u) + TOF_SIGMA_FXP_MM, false,
                  uint32_t(0)),
    _logging(logging)
{}

const bool ToFSensor::init(void)
{
   if(_is_initialized)
   {
      LOG_ERROR("Class is already initialized!");
      return false;
   }

   _is_initialized = true;

   return true;
}

const bool ToFSensor::update(void)
{
   readDistanceAndStatus();
   readSigma();

   return true;
}

const bool ToFSensor::readDistanceAndStatus(void)
{
   ComMsgErrorCodes error_code;

   if(RES_OK != _board._com_server->readDataObject(
                    _board._com_node_id, _com_sts_distance, error_code, 20u, 1u))
   {
      return false;
   }

   // Check error codes
   if(COM_MSG_ERR_NONE != error_code)
   {
      LOG_ERROR("Error reading data object: " << +_com_sts_distance.getID()
                                              << " of node " << _board._com_node_id);
      return true;
   }

   // Update values
   const uint32_t sts_distance = (uint32_t)(_com_sts_distance);
   _distance_mm  = static_cast<float>((uint16_t)(sts_distance & 0xFFFFu));
   _range_status = static_cast<ToFRangeStatus>((uint8_t)(sts_distance >> 16u));

   return true;
}

const bool ToFSensor::readSigma(void)
{
   ComMsgErrorCodes error_code;

   if(RES_OK != _board._com_server->readDataObject(
                    _board._com_node_id, _com_sigma_mm, error_code, 20u, 1u))
   {
      return false;
   }

   // Check error codes
   if(COM_MSG_ERR_NONE != error_code)
   {
      LOG_ERROR("Error reading data object: " << +_com_sts_distance.getID()
                                              << " of node " << _board._com_node_id);
      return true;
   }

   // Update values
   // TODO

   return true;
}

/* !Private Class Functions ------------------------------------------------------*/