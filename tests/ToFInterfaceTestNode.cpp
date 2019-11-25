//###############################################################
//# Copyright (C) 2019, Evocortex GmbH, All rights reserved.    #
//# Further regulations can be found in LICENSE file.           #
//###############################################################

/**
 * @file ToFInterfaceTestNode.cpp
 * @author MBA (info@evocortex.com)
 *
 * @brief Interface Test Node
 *
 * @version 1.0
 * @date 2019-08-02
 *
 * @copyright Copyright (c) 2019 Evocortex GmbH
 *
 */

/* Includes ----------------------------------------------------------------------*/
#include <ros/ros.h>
#include <std_msgs/Float32.h>
#include <std_msgs/UInt8.h>

#include <istream>

#include <evo_mbed/tools/com/ComServer.h>
#include <evo_mbed/tools/Logging.h>
#include "evo_tof_interface/ToFBoard.h"

using namespace evo_mbed;
/*--------------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
   const std::string _log_module = "main";

   ros::init(argc, argv, "tof_interface_test_node");
   ros::NodeHandle nh;
   ros::Rate loop_rate(30.0f);

   std::shared_ptr<ComServer> com_server(new ComServer(true));
   if(RES_OK != com_server->init("can_tof", 100u))
   {
      return -1;
   }

   unsigned int com_id           = 20u;
   const unsigned int NUM_BOARDS = 10u;
   std::array<std::shared_ptr<ToFBoard>, NUM_BOARDS> board_list;
   std::array<ros::Publisher, NUM_BOARDS * 2u> position_pub_list;
   std::array<ros::Publisher, NUM_BOARDS * 2u> range_status_list;

   for(auto idx = 0u; idx < NUM_BOARDS; idx++)
   {
      board_list[idx] = std::make_shared<ToFBoard>(com_id++, com_server, 30.0, true);

      if(!board_list[idx]->init())
      {
         std::cout << "Failed to intialize board with com-id: " << +(com_id - 1)
                   << std::endl;
         return -2;
      }

      std::stringstream sensor_name, sensor_name_left, sensor_name_right;
      sensor_name << "/ToF/" << +idx << "/";
      sensor_name_left << sensor_name.str() << "left/";
      sensor_name_right << sensor_name.str() << "right/";

      position_pub_list[idx * 2 + 0] =
          nh.advertise<std_msgs::Float32>(sensor_name_left.str() + "position", 5u);
      position_pub_list[idx * 2 + 1] =
          nh.advertise<std_msgs::Float32>(sensor_name_right.str() + "position", 5u);

      range_status_list[idx * 2 + 0] =
          nh.advertise<std_msgs::UInt8>(sensor_name_left.str() + "status", 5u);
      range_status_list[idx * 2 + 1] =
          nh.advertise<std_msgs::UInt8>(sensor_name_right.str() + "status", 5u);
   }

   while(ros::ok())
   {
      for(auto idx = 0u; idx < NUM_BOARDS; idx++)
      {
         std_msgs::Float32 position_left, position_right;
         std_msgs::UInt8 range_status_left, range_status_right;
         position_left.data      = board_list[idx]->getSensor(0)->getDistanceMM();
         position_right.data     = board_list[idx]->getSensor(1)->getDistanceMM();
         range_status_left.data  = board_list[idx]->getSensor(0)->getRangeStatus();
         range_status_right.data = board_list[idx]->getSensor(1)->getRangeStatus();

         position_pub_list[idx * 2 + 0].publish(position_left);
         position_pub_list[idx * 2 + 1].publish(position_right);

         range_status_list[idx * 2 + 0].publish(range_status_left);
         range_status_list[idx * 2 + 1].publish(range_status_right);
      }

      ros::spinOnce();
      loop_rate.sleep();
   }

   // board_list[0]->resetDevice();

   return 0;
}
