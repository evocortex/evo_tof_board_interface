//###############################################################
//# Copyright (C) 2019, Evocortex GmbH, All rights reserved.    #
//# Further regulations can be found in LICENSE file.           #
//###############################################################

/**
 * @file ToFBoard.cpp
 * @author MBA (info@evocortex.com)
 *
 * @brief Source ToF Board
 *
 * @version 1.0
 * @date 2019-10-15
 *
 * @copyright Copyright (c) 2019
 *
 */

/* Includes ----------------------------------------------------------------------*/
#include <evo_tof_interface/ToFBoard.h>
#include <evo_tof_interface/ToFSensor.h>
#include <evo_mbed/tools/Logging.h>
/*--------------------------------------------------------------------------------*/

using namespace evo_mbed;

/* Public Class Functions --------------------------------------------------------*/

ToFBoard::ToFBoard(const uint8_t node_id, std::shared_ptr<ComServer> com_server,
                   const double update_rate_hz, const bool logging) :
    _com_server(com_server),
    _com_node_id(node_id), _update_rate_hz(update_rate_hz), _logging(logging)
{}

ToFBoard::~ToFBoard(void)
{
   release();
}

const bool ToFBoard::init(void)
{
   if(_is_initialized)
   {
      LOG_ERROR("Class of Node: " << +_com_node_id << " is already initialized");
      return false;
   }

   if(!_com_server)
   {
      LOG_ERROR("Class of Node: " << +_com_node_id
                                  << " com server pointer is null!");
      return false;
   }

   if(_com_node_id < 1 && _com_node_id > 127)
   {
      LOG_ERROR("Node-ID: " << +_com_node_id << " is not valid!");
      return false;
   }

   if(_update_rate_hz <= 0.1)
   {
      LOG_ERROR("Update rate has to be >= 0.1 (" << _update_rate_hz << ")");
      return false;
   }

   // register ID
   if(RES_OK != _com_server->registerNode(_com_node_id))
   {
      return false;
   }

   // Timeout for initializeation phase
   std::this_thread::sleep_for(std::chrono::milliseconds(1));

   if(!readConstObject(_device_type))
      return false;

   // Check type
   if(3u != (uint8_t) _device_type)
   {
      LOG_ERROR("ToF Board error: Type of Node is '" << +(uint8_t) _device_type
                                                     << "' which is not a"
                                                     << " ToF Board (=3)!");
      return false;
   }

   if(!readConstObject(_fw_version))
      return false;
   if(!readConstObject(_com_version))
      return false;

   // Check communication version -> Check if com version fits
   // the supported stack
   if(TOF_COM_VER != (float) (_com_version))
   {

      LOG_ERROR("Node-ID: " << +_com_node_id << " com version is "
                            << (float) (_com_version) << " but only version: "
                            << TOF_COM_VER << " is supported!");

      return false;
   }

   if(!readConstObject(_fw_build_date))
      return false;

   // Create and intialize sensors
   unsigned int id = 0u;
   for(auto& sensor : _sensor_list)
   {
      sensor = std::shared_ptr<ToFSensor>(new ToFSensor(id++, *this, _logging));

      if(!sensor->init())
      {
         LOG_ERROR("Failed to initialized sensor: " << +id << " of node: "
                                                    << +_com_node_id);
         return false;
      }
   }

   // Create update thread
   _update_thread = std::make_unique<std::thread>(&ToFBoard::updateHandler, this);
   auto timer_ms  = 0u;
   while(timer_ms < 10u && !_run_update)
   {
      timer_ms++;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
   }

   _is_initialized = true;

   return true;
}

void ToFBoard::release(void)
{
   if(!_is_initialized)
      return;

   _run_update = false;
   _update_thread->join();

   for(auto& sensor : _sensor_list)
   {
      if(sensor)
      {
         sensor->release();
      }
   }

   _is_initialized = false;
}

const bool ToFBoard::resetDevice(const bool reset_to_bootl)
{
   if(!_is_initialized)
   {
      LOG_ERROR("Class is not initialized!");
      return false;
   }

   if(!reset_to_bootl)
   {
      _reset_device = (uint32_t) 1;
   }
   else
   {
      _reset_device = (uint32_t) 0x45564F42;
   }

   return writeDataObject(_reset_device, "Reset Device!");
}

std::shared_ptr<ToFSensor> ToFBoard::getSensor(const unsigned int id)
{
   if(!_is_initialized)
   {
      LOG_ERROR("Class is not initialized!");
      return std::shared_ptr<ToFSensor>();
   }

   if(id >= TOF_BOARD_SENSORS)
   {
      LOG_ERROR("Requested sensor id is invalid [0;1]");
      return std::shared_ptr<ToFSensor>();
   }

   return _sensor_list[id];
}

const bool ToFBoard::isInitialized(void) const
{
   return _is_initialized;
}

/* !Public Class Functions -------------------------------------------------------*/

/* Private Class Functions -------------------------------------------------------*/

void ToFBoard::updateHandler(void)
{
   _run_update = true;

   const std::chrono::duration<double, std::micro> loop_time_usec(1e6 /
                                                                  _update_rate_hz);

   while(_run_update)
   {
      const auto timestamp_start = std::chrono::high_resolution_clock::now();

      for(auto& sensor : _sensor_list)
      {
         sensor->update();
      }

      const auto timestamp_stop = std::chrono::high_resolution_clock::now();
      const std::chrono::duration<double, std::micro> exec_time_usec =
          timestamp_stop - timestamp_start;
      const auto sleep_time_usec = loop_time_usec - exec_time_usec;
      std::this_thread::sleep_for(sleep_time_usec);
   }
}

const bool ToFBoard::readConstObject(ComDataObject& object)
{
   ComMsgErrorCodes error_code;

   if(RES_OK !=
      _com_server->readDataObject(_com_node_id, object, error_code, 0u, 1u))
   {
      return false;
   }

   if(COM_MSG_ERR_NONE != error_code)
   {
      return false;
   }

   return true;
}

const bool ToFBoard::writeDataObject(ComDataObject& object, const std::string name)
{
   ComMsgErrorCodes error_code = COM_MSG_ERR_NONE;

   const std::string log_info =
       " (Object-ID: " + std::to_string(object.getID()) +
       ", Raw-Value: " + std::to_string(object.getRawValue()) + ", Desc: " + name +
       ")";

   // Write data with timeout threshold = default and 2 retries
   const Result com_result =
       _com_server->writeDataObject(_com_node_id, object, error_code, 0, 2u);

   switch(com_result)
   {
   case RES_OK:
   {

      if(COM_MSG_ERR_NONE == error_code)
      {
         object.setDataReaded();
         return true;
      }

      if(_logging)
      {
         switch(error_code)
         {
         case COM_MSG_ERR_INVLD_CMD:
         {
            LOG_ERROR("Failed to write object: Invalid command" << log_info);
         }
         break;
         case COM_MSG_ERR_READ_ONLY:
         {
            LOG_ERROR("Failed to write object: Read-Only" << log_info);
         }
         break;
         case COM_MSG_ERR_OBJCT_INVLD:
         {
            LOG_ERROR("Failed to write object: Object unknown" << log_info);
         }
         break;
         case COM_MSG_ERR_INVLD_DATA_TYPE:
         {
            LOG_ERROR("Failed to write object: Invalid data type" << log_info);
         }
         break;
         case COM_MSG_ERR_VALUE_RANGE_EXCD:
         {
            LOG_ERROR("Failed to write object: Value out of range" << log_info);
         }
         break;
         case COM_MSG_ERR_COND_NOT_MET:
         {
            LOG_ERROR("Failed to write object: Conditions not met to write"
                      << log_info);
         }
         break;
         }
      }

      if(RES_OK !=
         _com_server->readDataObject(_com_node_id, object, error_code, 0u, 2u))
         LOG_ERROR("Failed to read data from device" << log_info);

      return false;
   }
   break;

   case RES_TIMEOUT: { return false;
   }
   break;

   default:
   {
      // General error
      return false;
   }
   break;
   }

   return false;
}

/* !Private Class Functions ------------------------------------------------------*/
