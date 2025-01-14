/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <Falcor.h>
#include "PointData.slang"
#include <random>

namespace split_rendering {

class SeededRandom {
 public:
  SeededRandom() {
    // random_engine.seed(std::chrono::system_clock::now().time_since_epoch().count());
    random_engine.seed(42);
  };
  std::default_random_engine random_engine;
};

class MeshPointGenerator {
 public:
  void generatePointsInScene(Falcor::Scene::SharedPtr& scene, Falcor::RenderContext* renderContext);

  const std::vector<uint32_t>& getNumSamplesPerInstance() const {
    return numSamplesPerInstance_;
  }

  const std::vector<uint32_t>& getSampleOffsetPerInstance() const {
    return sampleOffsetPerInstance_;
  }

  const std::vector<float>& getDiskRadiusPerInstance() const {
    return diskRadiusPerInstance_;
  }

  const std::vector<Falcor::PointData>& getCPUPointData() const {
    return pointData_;
  }

  const Falcor::Buffer::SharedPtr& getGPUPointData() const {
    return gpuPointData_;
  }

  const Falcor::Buffer::SharedPtr& getGPUDiskRadiusPerInstance() const {
    return gpuDiskRadiusPerInstance_;
  }

  const Falcor::Buffer::SharedPtr& getGPUSampleOffsetPerInstance() const {
    return gpuSampleOffsetPerInstance_;
  }

  std::vector<uint32_t> cpuTriangleVisibility_;
  std::vector<uint32_t> cpuTriangleVisibilityOffsets_;

  
  // Constants for point generation
  uint32_t kNumSamplesPerUnitSquaredEliminated = 2048;
  uint32_t kMinSamplesPerInstance = 4096;
  uint32_t kSamplesEliminatedFactor = 8;

 private:
  uint32_t samplePoissonDiskOnMeshOffsets(
      const Falcor::PointData* uniformPointData,
      uint32_t uniformDataOffset,
      uint32_t numTriangles,
      uint32_t numSamplesUniform,
      uint32_t numSamplesAfterElimination,
      float totalSurfaceArea,
      bool isDoubleSided,
      float& instanceDiskRadius,
      uint32_t outputOffset,
      std::vector<Falcor::PointData>& output);

  float getTotalSurfaceArea(
      const std::vector<uint32_t>& indexBuffer,
      std::vector<Falcor::PackedStaticVertexData>& vertexBuffer,
      uint32_t indexOffset,
      uint32_t vertexOffset,
      uint32_t numTriangles,
      bool use16BitIndices,
      uint32_t instanceID,
      std::vector<float>& outTriangleAreas);

  std::vector<uint32_t> getNumSamplesPerTriangle(
      const std::vector<uint32_t>& indexBuffer,
      std::vector<Falcor::PackedStaticVertexData>& vertexBuffer,
      uint32_t indexOffset,
      uint32_t vertexOffset,
      uint32_t numTriangles,
      uint32_t numSamples,
      const std::vector<float>& triangleAreas,
      bool use16BitIndices,
      bool isDoubleSided,
      float totalSurfaceArea,
      uint32_t instanceID,
      uint32_t* numPlacedSamples = nullptr);

  void computeSampleCounts(
      Falcor::Scene::SharedPtr& scene,
      std::vector<float>& totalSurfaceAreas,
      std::vector<std::vector<uint32_t>>& triangleSampleCountsPerInstance,
      std::vector<std::vector<uint32_t>>& triangleSampleOffsetsPerInstance,
      std::vector<uint32_t>& uniformSamplesCount,
      std::vector<uint32_t>& uniformSamplesOffset,
      uint32_t& numSamplesTotal,
      uint32_t& numUniformSamplesTotal);

  void computeUniformSamples(
      Falcor::Scene::SharedPtr& scene,
      Falcor::RenderContext* renderContext,
      Falcor::Buffer::SharedPtr& uniformPointsGpu,
      std::vector<uint32_t>& instanceIds,
      std::vector<uint32_t>& uniformSamplesOffset,
      std::vector<std::vector<uint32_t>>& triangleSampleCountsPerInstance,
      std::vector<std::vector<uint32_t>>& triangleSampleOffsetsPerInstance);

  std::string getPoissonSampleFileName(Falcor::Scene::SharedPtr& scene, uint32_t instanceID);

  bool checkIfPoissonSamplesExist(Falcor::Scene::SharedPtr& scene, uint32_t instanceID);

  void loadPoissonSamples(std::string filePath, std::vector<Falcor::PointData>& output,
    uint32_t outputOffset);

  void savePoissonSamples(std::string filePath, std::vector<Falcor::PointData>& output,
    uint32_t outputOffset, uint32_t numSamples);

  uint32_t
  getIndex(const std::vector<uint32_t>& indexBuffer, const size_t i, bool use16BitIndices) {
    return use16BitIndices ? reinterpret_cast<const uint16_t*>(indexBuffer.data())[i]
                           : indexBuffer[i];
  }

  Falcor::float3 interpolateBarycentric(
      const Falcor::float3& a,
      const Falcor::float3& b,
      const Falcor::float3& c,
      float b0,
      float b1) {
    return a * b0 + b * b1 + c * (1.0f - b0 - b1);
  }

  void setupGPUUniformPointGenerationPipeline(Falcor::Scene::SharedPtr& scene);



  std::vector<uint32_t> numSamplesPerInstance_;
  std::vector<uint32_t> sampleOffsetPerInstance_;
  std::vector<float> diskRadiusPerInstance_;
  std::vector<Falcor::PointData> pointData_;
  Falcor::Buffer::SharedPtr gpuPointData_;
  Falcor::Buffer::SharedPtr gpuDiskRadiusPerInstance_;
  Falcor::Buffer::SharedPtr gpuSampleOffsetPerInstance_;
  Falcor::RtProgramVars::SharedPtr pointGenRtVars_;
  Falcor::RtProgram::SharedPtr pointGenRtProgram_;

};

} // namespace split_rendering
