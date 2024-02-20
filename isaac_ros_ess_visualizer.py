#!/usr/bin/env python3

# SPDX-FileCopyrightText: NVIDIA CORPORATION & AFFILIATES
# Copyright (c) 2022-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

# This script loads images from a folder and sends them to the ESSDisparityNode for inference,
# then saves the output prediction to spcified location as an image.

import argparse
import subprocess

import cv2
import cv_bridge
from isaac_ros_test import JSONConversion
import numpy as np
import time
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import CameraInfo, Image
from stereo_msgs.msg import DisparityImage
from nav_msgs.msg import Odometry
from tf2_msgs.msg import TFMessage

def get_args():
    parser = argparse.ArgumentParser(description='ESS Disparity Node Visualizer')
    parser.add_argument('--save_image', action='store_true', help='Save output or display it.')
    parser.add_argument('--result_path', default='/workspaces/isaac_ros-dev/results/',
                        help='Absolute path to save your result.')
    parser.add_argument('--r', default='10.0',type=float,
                        help='Subcribe rate')
    parser.add_argument('--raw_inputs', action='store_true',
                        help='Use rosbag as inputs or raw image and camera info files as inputs.')
    parser.add_argument('--enable_rosbag', action='store_true', help='Save output or display it',
                        default=False)
    parser.add_argument('--rosbag_path',
                        default='/workspaces/isaac_ros-dev/src/isaac_ros_dnn_stereo_depth/'
                                'resources/rosbags/ess_rosbag',
                        help='Absolute path to your rosbag.')
    parser.add_argument('--left_image_path',
                        default='/workspaces/isaac_ros-dev/src/isaac_ros_dnn_stereo_depth/'
                                'resources/examples/left.png',
                        help='Absolute path your left image.')
    parser.add_argument('--right_image_path',
                        default='/workspaces/isaac_ros-dev/src/isaac_ros_dnn_stereo_depth/'
                                'resources/examples/right.png',
                        help='Absolute path your right image.')
    parser.add_argument('--camera_info_path',
                        default='/workspaces/isaac_ros-dev/src/isaac_ros_dnn_stereo_depth/'
                                'resources/examples/camera.json',
                        help='Absolute path your camera info json file.')
    args = parser.parse_args()
    return args

i = 0
disp_img = None
tf_data = None
odom_data = None   




class ESSVisualizer(Node):

    def __init__(self, args):
        super().__init__('ess_visualizer')
        self.args = args
        self.encoding = 'rgb8'
        self._bridge = cv_bridge.CvBridge()

        # self.odom_data = None
        # self.tf_data = None

        self._disp_sub = self.create_subscription(
            DisparityImage, 'disparity', self.ess_callback, 10)
        
        self._tf_sub = self.create_subscription(TFMessage,'tf',self.tf_callback,10)
        self._odom_sub = self.create_subscription(Odometry,'odom',self.odom_callback,10)

        if self.args.raw_inputs:
            self._prepare_raw_inputs()
        elif self.args.enable_rosbag:
            self._prepare_rosbag_inputs()

    def _prepare_rosbag_inputs(self):
        subprocess.Popen('ros2 bag play -l ' + self.args.rosbag_path, shell=True)

    def _prepare_raw_inputs(self):
        self._img_left_pub = self.create_publisher(
            Image, 'left/image_rect', 10)
        self._img_right_pub = self.create_publisher(
            Image, 'right/image_rect', 10)
        self._camera_left_pub = self.create_publisher(
            CameraInfo, 'left/camera_info', 10)
        self._camera_right_pub = self.create_publisher(
            CameraInfo, 'right/camera_info', 10)

        self.create_timer(5, self.timer_callback)

        left_img = cv2.imread(self.args.left_image_path)
        right_img = cv2.imread(self.args.right_image_path)
        self.left_msg = self._bridge.cv2_to_imgmsg(np.array(left_img), self.encoding)
        self.right_msg = self._bridge.cv2_to_imgmsg(np.array(right_img), self.encoding)

        self.camera_info = JSONConversion.load_camera_info_from_json(self.args.camera_info_path)

    def timer_callback(self):
        self._img_left_pub.publish(self.left_msg)
        self._img_right_pub.publish(self.right_msg)
        self._camera_left_pub.publish(self.camera_info)
        self._camera_right_pub.publish(self.camera_info)
        self.get_logger().info('Inputs were published.')

    def ess_callback(self, disp_msg):
        global i
        global disp_img
        # self.get_logger().info('Result was received.')
        # self.get_logger().info('-=========================-')
        disp_img = self._bridge.imgmsg_to_cv2(disp_msg.image)
        # Normalize and convert to colormap for visalization
        disp_img = (disp_img.max()-disp_img) / disp_img.max() * 255
        # print(disp_img)
        # print(tf_data)
        # print(odom_data)
        self.save_data()
            
        cv2.waitKey(1)

    def tf_callback(self, tf_msg):
        global tf_data
        tf_data = tf_msg
        # self.get_logger().info('TF was received.')
        self.save_data()
        

    def odom_callback(self, odom_msg):
        global odom_data
        odom_data = odom_msg
        # self.get_logger().info('Odometry was received.')
        # print(odom_msg)
        self.save_data()

    def save_data(self):
        color_map = cv2.applyColorMap(disp_img.astype(np.uint8), cv2.COLORMAP_VIRIDIS)
        if odom_data is not None and tf_data is not None and disp_img is not None:
            pass
        if self.args.save_image:
            cv2.imwrite(self.args.result_path + 'data_depth' + str(i) + '.png', color_map)
            np.save(self.args.result_path + 'data_depth' + str(i) + '.npy', disp_img)
            with open(self.args.result_path + 'data_tf_' + str(i) + '.txt', 'w') as f:
                f.write(str(tf_data))
            with open(self.args.result_path + 'data_odom_' + str(i) + '.txt', 'w') as f:
                f.write(str(odom_data))
            print(tf_data)
            # print(odom_data)
            i += 1
            time.sleep(5.0)
            
        else:
            cv2.imshow('ess_output', color_map)
        

def main():

    args = get_args()
    rclpy.init()
    while KeyboardInterrupt:

        rclpy.spin(ESSVisualizer(args))
        
    rclpy.shutdown()

if __name__ == '__main__':
    main()
