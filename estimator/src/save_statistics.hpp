/*******************************************************
 * Copyright (C) 2020, RAM-LAB, Hong Kong University of Science and Technology
 *
 * This file is part of M-LOAM (https://ram-lab.com/file/jjiao/m-loam).
 * If you use this code, please cite the respective publications as
 * listed on the above websites.
 *
 * Licensed under the GNU General Public License v3.0;
 * you may not use this file except in compliance with the License.
 *
 * Author: Jianhao JIAO (jiaojh1994@gmail.com)
 *******************************************************/
#pragma once

#include <iostream>
#include <fstream>

#include <nav_msgs/Odometry.h>
#include <nav_msgs/Path.h>

#include "estimator/estimator.h"
#include "estimator/parameters.h"

class SaveStatistics
{
public:
    void saveSensorPath(const std::string &filename, const nav_msgs::Path &sensor_path);

    void saveOdomStatistics(const string &calib_eig_filename, 
                            const string &calib_result_filename, 
                            const string &odom_filename,
                            const Estimator &estimator);
    void saveOdomTimeStatistics(const string &filename, const Estimator &estimator);

    void saveMapStatistics(const string &map_filename,
                                           const string &map_factor_filename,
                                           const string &map_eig_filename,
                                           const string &map_pose_uct_filename,
                                           const nav_msgs::Path &laser_aft_mapped_path,
                                           const std::vector<Eigen::Matrix<double, 1, 6>> &d_factor_list,
                                           const std::vector<Eigen::Matrix<double, 6, 6>> &d_eigvec_list,
                                           const std::vector<double> &cov_mapping_list); 
    void saveMapTimeStatistics(const string &filename, const double &total_time, const int &frame_cnt);
};

void SaveStatistics::saveSensorPath(const string &filename, const nav_msgs::Path &sensor_path)
{
    if (sensor_path.poses.size() == 0)
        return;
    std::ofstream fout(filename.c_str(), std::ios::out);
    fout.setf(ios::fixed, ios::floatfield);
    for (size_t i = 0; i < sensor_path.poses.size(); i++)
    {
        const geometry_msgs::PoseStamped &sensor_pose = sensor_path.poses[i];
        fout.precision(15);
        fout << sensor_pose.header.stamp.toSec() << " ";
        fout.precision(8);
        fout << sensor_pose.pose.position.x << " "
             << sensor_pose.pose.position.y << " "
             << sensor_pose.pose.position.z << " "
             << sensor_pose.pose.orientation.x << " "
             << sensor_pose.pose.orientation.y << " "
             << sensor_pose.pose.orientation.z << " "
             << sensor_pose.pose.orientation.w << std::endl;
    }
}

// odom format: timestamp tx ty tz qx qy qz qw
void SaveStatistics::saveOdomStatistics(const string &calib_eig_filename, 
                                        const string &calib_result_filename, 
                                        const string &odom_filename,
                                        const Estimator &estimator)
{
    if (estimator.solver_flag_ != Estimator::SolverFlag::NON_LINEAR)
        return;
    ofstream fout(calib_eig_filename.c_str(), ios::out);
    fout.setf(ios::fixed, ios::floatfield);
    fout.precision(5);

    for (auto i = 0; i < NUM_OF_LASER; i++)
        fout << estimator.cur_eig_calib_[i] << ", ";
    fout << std::endl;
    fout.close();

    fout.open(calib_result_filename.c_str(), ios::out);
    for (auto i = 0; i < NUM_OF_LASER; i++)
    {
        fout << estimator.cur_time_ << ", "
                << estimator.tbl_[i](0) << ", "
                << estimator.tbl_[i](1) << ", "
                << estimator.tbl_[i](2) << ", "
                << estimator.qbl_[i].x() << ", "
                << estimator.qbl_[i].y() << ", "
                << estimator.qbl_[i].z() << ", "
                << estimator.qbl_[i].w() << std::endl;
    }
    fout.close();

    fout.open(odom_filename.c_str(), ios::out);
    for (auto i = 0; i < estimator.v_laser_path_[IDX_REF].poses.size(); i++)
    {
        const geometry_msgs::PoseStamped &sensor_pose = estimator.v_laser_path_[IDX_REF].poses[i];
        fout.precision(15);
        fout << sensor_pose.header.stamp.toSec() << " ";
        fout.precision(8);
        fout << sensor_pose.pose.position.x << " "
                << sensor_pose.pose.position.y << " "
                << sensor_pose.pose.position.z << " "
                << sensor_pose.pose.orientation.x << " "
                << sensor_pose.pose.orientation.y << " "
                << sensor_pose.pose.orientation.z << " "
                << sensor_pose.pose.orientation.w << std::endl;
    }
    fout.close();
}

void SaveStatistics::saveOdomTimeStatistics(const string &filename, const Estimator &estimator)
{
    std::ofstream fout(filename.c_str(), std::ios::out);
    fout.precision(15);
    fout << "frame, total_mea_pre_time, total_opt_odom_time, total_corner_feature, total_surf_feature, mean_opt_odom_time" << std::endl;
    fout << estimator.frame_cnt_ << ", " << estimator.total_measurement_pre_time_
         << ", " << estimator.total_opt_odom_time_
         << ", " << estimator.total_corner_feature_
         << ", " << estimator.total_surf_feature_ 
         << ", " << estimator.total_opt_odom_time_ / estimator.frame_cnt_ << std::endl;
    fout.close();
    printf("******* Frame: %d, mean measurement preprocess time: %fms, mean optimize odometry time: %fms\n", estimator.frame_cnt_,
           estimator.total_measurement_pre_time_ / estimator.frame_cnt_, estimator.total_opt_odom_time_ / estimator.frame_cnt_);
    printf("******* Frame: %d, mean corner feature: %f, mean surf feature: %f\n", estimator.frame_cnt_,
           estimator.total_corner_feature_ * 1.0 / estimator.frame_cnt_, estimator.total_surf_feature_ * 1.0 / estimator.frame_cnt_);

    LOG(INFO) << "Frame: " << estimator.frame_cnt_ << ", mean measurement preprocess time: " << estimator.total_measurement_pre_time_ / estimator.frame_cnt_
              << "ms, mean optimize odometry time: " << estimator.total_surf_feature_ * 1.0 / estimator.frame_cnt_;
    LOG(INFO) << "Frame: " << estimator.frame_cnt_ << ", mean surf feature: " << estimator.total_surf_feature_ * 1.0 / estimator.frame_cnt_
              << "mean corner feature: " << estimator.total_corner_feature_ * 1.0 / estimator.frame_cnt_;
}

void SaveStatistics::saveMapStatistics(const string &map_filename,
                                       const string &map_factor_filename,
                                       const string &map_eig_filename,
                                       const string &map_pose_uct_filename,
                                       const nav_msgs::Path &laser_aft_mapped_path,
                                       const std::vector<Eigen::Matrix<double, 1, 6>> &d_factor_list,
                                       const std::vector<Eigen::Matrix<double, 6, 6>> &d_eigvec_list,
                                       const std::vector<double> &cov_mapping_list)
{
    printf("Saving mapping statistics\n");
    std::ofstream fout(map_filename.c_str(), std::ios::out);
    for (size_t i = 0; i < laser_aft_mapped_path.poses.size(); i++)
    {
        const geometry_msgs::PoseStamped &laser_pose = laser_aft_mapped_path.poses[i];
        fout.precision(15);
        fout << laser_pose.header.stamp.toSec() << " ";
        fout.precision(8);
        fout << laser_pose.pose.position.x << " "
                << laser_pose.pose.position.y << " "
                << laser_pose.pose.position.z << " "
                << laser_pose.pose.orientation.x << " "
                << laser_pose.pose.orientation.y << " "
                << laser_pose.pose.orientation.z << " "
                << laser_pose.pose.orientation.w << std::endl;
    }
    fout.close();

    fout.open(map_factor_filename.c_str(), std::ios::out);
    fout.precision(8);
    for (size_t i = 0; i < d_factor_list.size(); i++)
        fout << d_factor_list[i] << std::endl;
    fout.close();

    fout.open(map_eig_filename.c_str(), std::ios::out);
    fout.precision(8);
    for (size_t i = 0; i < d_eigvec_list.size(); i++)
        fout << d_eigvec_list[i] << std::endl;
    fout.close();

    fout.open(map_pose_uct_filename.c_str(), std::ios::out);
    fout << "cov_mapping_uncertainty" << std::endl;
    fout.precision(8);
    for (size_t i = 0; i < cov_mapping_list.size(); i++)
        fout << cov_mapping_list[i] << std::endl;
    fout.close();
}

void SaveStatistics::saveMapTimeStatistics(const string &filename, const double &total_time, const int &frame_cnt)
{
    std::ofstream fout(filename.c_str(), std::ios::out);
    fout.precision(15);
    fout << "frame, total_mapping_time, mean_mapping_time" << std::endl;
    fout << frame_cnt << ", " << total_time << ", " << total_time / frame_cnt << std::endl;
    fout.close();
    printf("******* Frame: %d, mean mapping time: %fms\n", frame_cnt, total_time / frame_cnt);
    LOG(INFO) << "Frame: " << frame_cnt << ", mean mapping time: " << total_time / frame_cnt << "ms";
}