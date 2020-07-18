# !/bin/bash

export data_path=/Monster/dataset/oxford_radar_dataset/2019-01-18-15-20-12-radar-oxford-10k/data_ds_250_800.bag
export rpg_path=/home/jjiao/catkin_ws/src/localization/rpg_trajectory_evaluation
export result_path=$rpg_path/results/real_vehicle/oxford/20190118_debug/

roslaunch mloam mloam_realvehicle_oxford.launch \
    run_mloam:=true \
    result_save:=true \
    gf_method:=rnd \
    gf_ratio_ini:=0.2 \
    gnc:=true \
    loss_mode:=gmc \
    bag_file:=$data_path \
    output_path:=$result_path
sleep 5

roslaunch mloam mloam_realvehicle_oxford.launch \
    run_mloam:=true \
    result_save:=true \
    gf_method:=fps \
    gf_ratio_ini:=0.2 \
    gnc:=true \
    loss_mode:=gmc \
    bag_file:=$data_path \
    output_path:=$result_path
sleep 5

roslaunch mloam mloam_realvehicle_oxford.launch \
    run_mloam:=true \
    result_save:=true \
    gf_method:=gd_fix \
    gf_ratio_ini:=0.2 \
    gnc:=true \
    loss_mode:=gmc \
    bag_file:=$data_path \
    output_path:=$result_path
sleep 5

# roslaunch mloam mloam_realvehicle_oxford.launch \
#     run_mloam:=true \
#     result_save:=true \
#     gf_method:=gd_float \
#     gf_ratio_ini:=0.2 \
#     gnc:=true \
#     loss_mode:=gmc \
#     bag_file:=$data_path \
#     output_path:=$result_path
# sleep 5

python2 $rpg_path/scripts/analyze_trajectory_single_mloam.py \
--est_type \
    M-LOAM-rnd-0.2-gmc-gnc M-LOAM-fps-0.2-gmc-gnc \
    M-LOAM-gd-float-0.2-gmc M-LOAM-gd-float-0.2-gmc-gnc \
    LEGO-LOAM \
--compare \
    $result_path