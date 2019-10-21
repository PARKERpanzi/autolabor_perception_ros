/******************************************************************************
 * Copyright 2017 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#ifndef _APOLLO_PERCEPTION_STANDALONE_FUSION_ASYNC_FUSION_ASYNC_FUSION_H_
#define _APOLLO_PERCEPTION_STANDALONE_FUSION_ASYNC_FUSION_ASYNC_FUSION_H_

#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "fusion/proto/async_fusion_config.pb.h"

#include "common/object.h"
#include "fusion/interface/base_fusion.h"
#include "fusion/probabilistic_fusion/pbf_base_track_object_matcher.h"
#include "fusion/probabilistic_fusion/pbf_hm_track_object_matcher.h"
#include "fusion/probabilistic_fusion/pbf_track_manager.h"

namespace apollo_perception_standalone {

class AsyncFusion : public BaseFusion {
 public:
  AsyncFusion() = default;
  ~AsyncFusion();

  virtual bool Init();

  bool Init(bool log_fusion);

  bool Init(bool log_fusion, std::string &file_path_config);

  // @brief: fuse objects from multi sensors(64-lidar, 16-lidar, radar...)
  // @param [in]: multi sensor objects.
  // @param [out]: fused objects.
  // @return true if fuse successfully, otherwise return false
  virtual bool Fuse(const std::vector<SensorObjects> &multi_sensor_objects,
                    std::vector<std::shared_ptr<Object>> *fused_objects,
                    FusionOptions *options);

  virtual std::string name() const;

 protected:
  void FuseFrame(PbfSensorFramePtr frame);

  /**@brief create new tracks for objects not assigned to current tracks*/
  void CreateNewTracks(
      const std::vector<std::shared_ptr<PbfSensorObject>> &sensor_objects,
      const std::vector<int> &unassigned_ids, 
      const SensorType &sensor_type);

  /**@brief update current tracks with matched objects*/
  void UpdateAssignedTracks(
      const std::vector<std::shared_ptr<PbfSensorObject>> &sensor_objects,
      const std::vector<std::pair<int, int>> &assignments,
      const std::vector<double> &track_objects_dist,
      std::vector<PbfTrackPtr> const *tracks);

  /**@brief update current tracks which cannot find matched objects*/
  void UpdateUnassignedTracks(const std::vector<int> &unassigned_tracks,
                              const std::vector<double> &track_object_dist,
                              const SensorType &sensor_type,
                              const std::string &sensor_id,
                              const double timestamp,
                              std::vector<PbfTrackPtr> *tracks);

  void CollectFusedObjects(double timestamp,
                           std::vector<std::shared_ptr<Object>> *fused_objects);

  void DecomposeFrameObjects(
      const std::vector<std::shared_ptr<PbfSensorObject>> &frame_objects,
      std::vector<std::shared_ptr<PbfSensorObject>> *foreground_objects,
      std::vector<std::shared_ptr<PbfSensorObject>> *background_objects);

  void FuseForegroundObjects(
      const Eigen::Vector3d &ref_point, const SensorType &sensor_type,
      const std::string &sensor_id, const double timestamp,
      std::vector<std::shared_ptr<PbfSensorObject>> *foreground_objects);

  PbfSensorFramePtr ConstructFrame(const SensorObjects &obj);

 protected:
  bool log_fusion_ = false;
  
  bool started_ = false;
  std::unique_ptr<PbfBaseTrackObjectMatcher> matcher_;
  PbfTrackManager *track_manager_ = nullptr;
  std::mutex fusion_mutex_;

  async_fusion_config::ModelConfigs config_;

};

}  // namespace apollo

#endif  // MODULES_PERCEPTION_OBSTACLE_FUSION_ASYNC_FUSION_ASYNC_FUSION_H_
