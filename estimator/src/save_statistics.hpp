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
                           const string &map_sp_filename,
                           const string &map_logdet_filename,
                           const nav_msgs::Path &laser_aft_mapped_path,
                           const std::vector<Eigen::Matrix<double, 1, 6>> &d_factor_list,
                           const std::vector<Eigen::Matrix<double, 6, 6>> &d_eigvec_list,
                           const std::vector<std::vector<double> > &mapping_sp_list,
                           const std::vector<double> &logdet_H_list);

    void saveMapTimeStatistics(const string &map_time_filename,
                               const std::vector<double> &total_feat_time,
                               const std::vector<double> &total_solver_time,
                               const std::vector<double> &total_mapping_time);
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

    for (size_t i = 0; i < NUM_OF_LASER; i++)
        fout << estimator.cur_eig_calib_[i] << ", ";
    fout << std::endl;
    fout.close();

    fout.open(calib_result_filename.c_str(), ios::out);
    for (size_t i = 0; i < NUM_OF_LASER; i++)
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
    for (size_t i = 0; i < estimator.v_laser_path_[IDX_REF].poses.size(); i++)
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

void SaveStatistics::saveOdomTimeStatistics(const string &filename,
                                            const Estimator &estimator)
{
    std::ofstream fout(filename.c_str(), std::ios::out);
    fout.precision(15);
    fout << "frame, mean_corner_feature, mean_surf_feature, mean_mea_pre_time, mean_matching_time, mean_solver_time, mean_marginalization_time, mean_opt_odom_time" << std::endl;
    fout << estimator.frame_cnt_
         << ", " << estimator.total_corner_feature_ / estimator.frame_cnt_
         << ", " << estimator.total_surf_feature_ / estimator.frame_cnt_
         << ", " << std::accumulate(estimator.total_measurement_pre_time_.begin(), estimator.total_measurement_pre_time_.end(), 0.0) / estimator.total_measurement_pre_time_.size()
         << ", " << std::accumulate(estimator.total_feat_matching_time_.begin(), estimator.total_feat_matching_time_.end(), 0.0) / estimator.total_feat_matching_time_.size()
         << ", " << std::accumulate(estimator.total_solver_time_.begin(), estimator.total_solver_time_.end(), 0.0) / estimator.total_solver_time_.size()
         << ", " << std::accumulate(estimator.total_marginalization_time_.begin(), estimator.total_marginalization_time_.end(), 0.0) / estimator.total_marginalization_time_.size()
         << ", " << std::accumulate(estimator.total_whole_odom_time_.begin(), estimator.total_whole_odom_time_.end(), 0.0) / estimator.total_whole_odom_time_.size()
         << std::endl;
    for (size_t i = 0; i < estimator.total_measurement_pre_time_.size(); i++) 
    {
        if (i >= estimator.total_solver_time_.size() ||
            i >= estimator.total_marginalization_time_.size() ||
            i >= estimator.total_whole_odom_time_.size()) 
                break;
        fout << estimator.total_measurement_pre_time_[i] << ", "
             << estimator.total_feat_matching_time_[i] << ", "
             << estimator.total_solver_time_[i] << ", " 
             << estimator.total_marginalization_time_[i] << ", "
             << estimator.total_whole_odom_time_[i] << std::endl;
    }
    fout.close();

    printf("Frame: %d, mean measurement preprocess time: %fms, mean optimize odometry time: %fms\n", estimator.frame_cnt_,
           std::accumulate(estimator.total_measurement_pre_time_.begin(), estimator.total_measurement_pre_time_.end(), 0.0) / estimator.total_measurement_pre_time_.size(),
           std::accumulate(estimator.total_whole_odom_time_.begin(), estimator.total_whole_odom_time_.end(), 0.0) / estimator.total_whole_odom_time_.size());
    printf("Frame: %d, mean corner feature: %f, mean surf feature: %f\n", estimator.frame_cnt_,
           estimator.total_corner_feature_ * 1.0 / estimator.frame_cnt_, 
           estimator.total_surf_feature_ * 1.0 / estimator.frame_cnt_);

    LOG(INFO) << "Frame: " << estimator.frame_cnt_
              << ", mean measurement preprocess time: "
              << std::accumulate(estimator.total_measurement_pre_time_.begin(), estimator.total_measurement_pre_time_.end(), 0.0) / estimator.total_measurement_pre_time_.size()
              << "ms, mean optimize odometry time: "
              << std::accumulate(estimator.total_whole_odom_time_.begin(), estimator.total_whole_odom_time_.end(), 0.0) / estimator.total_whole_odom_time_.size();

    LOG(INFO) << "Frame: " << estimator.frame_cnt_
              << ", mean surf feature: " 
              << estimator.total_surf_feature_ * 1.0 / estimator.frame_cnt_
              << "mean corner feature: " 
              << estimator.total_corner_feature_ * 1.0 / estimator.frame_cnt_;
}

void SaveStatistics::saveMapStatistics(const string &map_filename,
                                       const string &map_factor_filename,
                                       const string &map_eig_filename,
                                       const string &map_sp_filename,
                                       const string &map_logdet_filename,
                                       const nav_msgs::Path &laser_aft_mapped_path,
                                       const std::vector<Eigen::Matrix<double, 1, 6>> &d_factor_list,
                                       const std::vector<Eigen::Matrix<double, 6, 6>> &d_eigvec_list,
                                       const std::vector<std::vector<double> > &mapping_sp_list,
                                       const std::vector<double> &logdet_H_list)
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
    fout << "d_factor" << std::endl;
    fout.precision(8);
    for (size_t i = 0; i < d_factor_list.size(); i++)
        fout << d_factor_list[i] << std::endl;
    fout.close();

    fout.open(map_eig_filename.c_str(), std::ios::out);
    fout << "d_eigvec" << std::endl;
    fout.precision(8);
    for (size_t i = 0; i < d_eigvec_list.size(); i++)
        fout << d_eigvec_list[i] << std::endl;
    fout.close();

    fout.open(map_sp_filename.c_str(), std::ios::out);
    fout << "mapping spectral property: trace, logdet, minimun eigenvalue (first row as the mean)" << std::endl;
    fout.precision(8);
    double m_trace = 0, m_logdet = 0, m_eig = 0;
    for (size_t i = 0; i < mapping_sp_list.size(); i++)
    {
        m_trace += mapping_sp_list[i][0] / mapping_sp_list.size();
        m_logdet += mapping_sp_list[i][1] / mapping_sp_list.size();
        m_eig += mapping_sp_list[i][2] / mapping_sp_list.size();
    }
    fout << m_trace << ", " << m_logdet << ", " << m_eig << std::endl;
    for (size_t i = 0; i < mapping_sp_list.size(); i++)
    {
        for (const double data : mapping_sp_list[i]) fout << data << ", ";
        fout << std::endl;
    }    
    fout.close();

    fout.open(map_logdet_filename.c_str(), std::ios::out);
    fout << "logdet_H_list" << std::endl;
    fout.precision(8);
    for (size_t i = 0; i < logdet_H_list.size(); i++)
        fout << logdet_H_list[i] << std::endl;
    fout.close();
}

void SaveStatistics::saveMapTimeStatistics(const string &map_time_filename,
                                           const std::vector<double> &total_feat_time,
                                           const std::vector<double> &total_solver_time,
                                           const std::vector<double> &total_mapping_time)
{
    std::ofstream fout(map_time_filename.c_str(), std::ios::out);
    fout.precision(15);
    fout << "frame, mean_feat_matching_time, mean_solver_time, mean_mapping_time" << std::endl;
    fout << total_mapping_time.size() << ", "
         << std::accumulate(total_feat_time.begin(), total_feat_time.end(), 0.0) / total_feat_time.size()
         << std::accumulate(total_solver_time.begin(), total_solver_time.end(), 0.0) / total_solver_time.size() << ", "
         << std::accumulate(total_mapping_time.begin(), total_mapping_time.end(), 0.0) / total_mapping_time.size() << std::endl;
    for (size_t i = 0; i < total_mapping_time.size(); i++)
    {
        if (i >= total_feat_time.size() || 
            i >= total_solver_time.size()) 
                break;
        fout << total_feat_time[i] << ", "
             << total_solver_time[i] << ", "
             << total_mapping_time[i] << std::endl;
    }
    fout.close();

    printf("Frame: %d, mean mapping time: %fms\n",
           total_mapping_time.size(),
           std::accumulate(total_mapping_time.begin(), total_mapping_time.end(), 0.0) / total_mapping_time.size());

    LOG(INFO) << "Frame: " << total_mapping_time.size()
              << ", mean mapping time: " 
              << std::accumulate(total_mapping_time.begin(), total_mapping_time.end(), 0.0) / total_mapping_time.size() 
              << "ms";
}