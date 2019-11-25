//###############################################################
//# Copyright (C) 2019, Evocortex GmbH, All rights reserved.    #
//# Further regulations can be found in LICENSE file.           #
//###############################################################

/**
 * @file ToFBoard.h
 * @author MBA (info@evocortex.com)
 *
 * @brief ToF Board Representation
 *
 * @version 1.0
 * @date 2019-10-15
 *
 * @copyright Copyright (c) 2019 Evocortex GmbH
 *
 */

#ifndef EVO_TOF_BOARD_H_
#define EVO_TOF_BOARD_H_

/* Includes ----------------------------------------------------------------------*/
#include <evo_mbed/Utils.h>
#include <evo_mbed/tools/com/ComServer.h>
#include <evo_tof_interface/ToFSensor.h>
/*--------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** @addtogroup evocortex
 * @{
 */

namespace evo_mbed {

/*--------------------------------------------------------------------------------*/
/** @addtogroup evocortex_ToFSensor
 * @{
 */

/** \brief Supported communication version */
constexpr float TOF_COM_VER = 1.0f;

/** \brief Number of mounted sensors on one board */
constexpr unsigned int TOF_BOARD_SENSORS = 2u;

/**
 * @brief Communication objects of the ToF board
 */
enum ToFBoardObjects : uint16_t
{
   /* General data */
   TOF_DEV_TYPE      = 10001u, //!< Device Type ID
   TOF_FW_VER        = 10002u, //!< Firmware Version of motor controller
   TOF_FW_COM_VER    = 10003u, //!< Communication stack version
   TOF_FW_BUILD_DATE = 10004u, //!< Build date of the firmware

   /* General settings */
   TOF_COM_RESET = 10101u, //!< Reset device if set to true

   /* Start of ToF objects */
   TOF_SENS_PARAM_BASE_IDX = 11000u, //!< Base index of drive parameters

   MCO_OBJ_SIZE = 9u, //!< Num of communication objects
};

/**
 * @brief ToF Sensor Data Objects
 *
 */
enum ToFSensorObjects : uint16_t
{
   TOF_STS_DISTANCE_MM = 101u, //!< Measured distance in mm and status
   TOF_SIGMA_FXP_MM,           //!< Measurement sigma in mm 16 bit fixed point value
};

/**
 * @brief ToF Board Representation
 *
 */
class ToFBoard
{
 public:
   /**
    * @brief Constructor of ToF Board
    *
    * @param node_id node_id Communication ID of the motor shield [1;127]
    * @param com_server com_server Pointer to communication server
    * @param logging true Enable logging output (default=false)
    */
   ToFBoard(const uint8_t node_id, std::shared_ptr<ComServer> com_server,
            const double update_rate_hz = 10u, const bool logging = false);

   /** \brief Destructor */
   ~ToFBoard(void);

   /**
    * @brief Initializes the ToF board
    *        Checks if board is reachable, if communication
    *        version is supported and starts async update thread.
    *
    * @return true Success
    * @return false Error
    */
   const bool init(void);

   /**
    * @brief Releases the object stops threads and releases
    *        memory
    */
   void release(void);

   /**
    * @brief Sends request to reset the device
    *
    * @param reset_to_bootl True: resets device to bootloader
    *
    * @return true Success
    * @return false Error
    */
   const bool resetDevice(const bool reset_to_bootl = false);

   /**
    * @brief Get the motor
    *
    * @param id ID of the motor 0: Right sensor 1: Left sensor
    *
    * @return std::shared_ptr<Motor> Requested object
    */
   std::shared_ptr<ToFSensor> getSensor(const unsigned int id);

   /** \brief Check if class is initialized */
   const bool isInitialized(void) const;

 private:
   /**
    * @brief Updates the ToF sensors
    */
   void updateHandler(void);

   /**
    * @brief Reads a constant data object
    *
    * @param object Object to read
    *
    * @return true Reading data was successful
    * @return false Failed reading data
    */
   const bool readConstObject(ComDataObject& object);

   /**
    * @brief Writes a data object via can
    *
    * @param object Object to write
    * @param name Name of the object for logging
    *
    * @return true Successfully written value
    * @return false Error during writting
    */
   const bool writeDataObject(ComDataObject& object, const std::string name);

   /** \brief Used communication server */
   std::shared_ptr<ComServer> _com_server;

   /** \brief Node ID of the client */
   const unsigned int _com_node_id = 0u;

   /** \brief Update rate of the async data in hz */
   const double _update_rate_hz = 20.0f;

   /** Communication objects */
   ComDataObject _device_type   = ComDataObject(TOF_DEV_TYPE, false, uint8_t(0));
   ComDataObject _fw_version    = ComDataObject(TOF_FW_VER, false, 0.0f);
   ComDataObject _com_version   = ComDataObject(TOF_FW_COM_VER, false, 0.0f);
   ComDataObject _fw_build_date = ComDataObject(TOF_FW_BUILD_DATE, false, 0.0f);

   /** General settings */
   ComDataObject _reset_device = ComDataObject(TOF_COM_RESET, true, uint32_t(0));

   /** \brief List containing ToF sensor objects */
   std::array<std::shared_ptr<ToFSensor>, TOF_BOARD_SENSORS> _sensor_list;

   std::unique_ptr<std::thread> _update_thread;
   std::atomic<bool> _run_update;

   /** \brief Logging option: set to true to enable logging */
   const bool _logging = false;

   /** \brief Logging module name */
   const std::string _log_module = "ToFBoard";

   /** \brief True class is initialized */
   bool _is_initialized = false;

   friend ToFSensor;
};

/**
 * @}
 */ // evocortex_ToFSensor
/*--------------------------------------------------------------------------------*/

}; // namespace evo_mbed

/**
 * @}
 */ // evocortex
/*--------------------------------------------------------------------------------*/

#endif /* EVO_TOF_BOARD_H_ */