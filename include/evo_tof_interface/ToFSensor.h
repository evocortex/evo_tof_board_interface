//###############################################################
//# Copyright (C) 2019, Evocortex GmbH, All rights reserved.    #
//# Further regulations can be found in LICENSE file.           #
//###############################################################

/**
 * @file ToFSensor.h
 * @author MBA (info@evocortex.com)
 *
 * @brief Interface to ToF Sensor Board
 *
 * @version 1.0
 * @date 2019-10-15
 *
 * @copyright Copyright (c) 2019 Evocortex GmbH
 *
 */

#ifndef EVO_TOF_SENSOR_H_
#define EVO_TOF_SENSOR_H_

/* Includes ----------------------------------------------------------------------*/
#include <atomic>

#include <evo_mbed/Utils.h>
#include <evo_mbed/tools/com/ComServer.h>
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

// Predefine
class ToFBoard;

/**
 * @brief ToF Range Status
 */
enum ToFRangeStatus : uint8_t
{
   TOF_RSTS_VLD            = 0u, //!< Measurement valid
   TOF_RSTS_SIGMA_FAILURE  = 1u, //!< Sigma estimator check above internal threshold
   TOF_RSTS_SIGNAL_FAILURE = 2u, //!< Signal value is below internal threshold
   TOF_RSTS_OUT_OF_BOUNDS  = 4u, //!< Signal phase is out of bounds
   TOF_RSTS_HARDWARE_FAIL  = 5u, //!< Hardware VCSEL failure
   TOF_RSTS_WRAP_TARGET_FAIL = 7u, //!< Warning: Wrapped target not matching phases
   TOF_RSTS_PROC_FAIL        = 8u, //!< Internal algorithm over- or underflow
   TOF_RSTS_RANGE_INVLD      = 14u //!< Reported range is invalid
};

/**
 * @brief ToF Sensor Representation
 *
 */
class ToFSensor
{
 public:
   /**
    * @brief Destructor of ToFSensor
    */
   ~ToFSensor(void);

   /**
    * @brief Stops runnings threads and releases the components
    */
   void release(void);

   /* Getters */
   const float getDistanceMM(void) const { return _distance_mm; }
   const float getSigmaMM(void) const { return _sigma_mm; }
   const ToFRangeStatus getRangeStatus(void) const { return _range_status; }

 private:
   /**
    * @brief Constructs a new ToF sensor
    *
    * @param board Reference to the board
    * @param id ID of the board
    * @param logging Set to true to enable logging output
    */
   ToFSensor(const unsigned int id, ToFBoard& board, const bool logging = false);

   /**
    * @brief Initialize the ToF sensor
    *
    * @return true Success
    * @return false Error
    */
   const bool init(void);

   /**
    * @brief Handler which reads data from sensor
    *
    * @return true Success
    * @return false Error
    */
   const bool update(void);

   /**
    * @brief Reads the distance value and the range status
    *        Called by update handler
    *
    * @return true Success
    * @return false Error
    */
   const bool readDistanceAndStatus(void);

   /**
    * @brief Reads the sigma value of the measurment
    *
    * @return true Success
    * @return false Error
    */
   const bool readSigma(void);

   const unsigned int _id = 0u; //!< ID of the sensor

   ToFBoard& _board; //!< Reference of board instance holding sensor

   ComDataObject _com_sts_distance; //!< Measured distance in mm including status
   ComDataObject _com_sigma_mm;     //!< Sigma value of measurement in mm

   std::atomic<float> _distance_mm;           //!< Measured distance in mm
   std::atomic<float> _sigma_mm;              //!< Quality of measurement in percent
   std::atomic<ToFRangeStatus> _range_status; //!< Range status of measurement

   /** \brief Logging option: set to true to enable logging */
   const bool _logging = false;

   /** \brief Logging module name */
   const std::string _log_module = "ToFSensor";

   /** \brief True class is initialized */
   bool _is_initialized = false;

   friend evo_mbed::ToFBoard;
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

#endif /* EVO_TOF_SENSOR_H_ */