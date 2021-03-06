/* Copyright 2020 The TensorFlow Quantum Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow_quantum/core/qsim/state_space.h"

#include <cmath>
#include <complex>
#include <memory>

#include "gtest/gtest.h"
#include "tensorflow_quantum/core/qsim/mux.h"

namespace tfq {
namespace qsim {
namespace {

TEST(StatSpaceTest, SampleOneSample) {
  auto equal = std::unique_ptr<StateSpace>(GetStateSpace(1, 1));
  equal->CreateState();
  equal->SetAmpl(0, std::complex<float>(1.0, 0.));
  equal->SetAmpl(1, std::complex<float>(0.0, 0.));

  std::vector<uint64_t> samples;
  equal->SampleState(1, &samples);
  ASSERT_EQ(samples.size(), 1);
}

TEST(StateSpaceTest, SampleZeroSamples) {
  auto equal = std::unique_ptr<StateSpace>(GetStateSpace(1, 1));
  equal->CreateState();
  equal->SetAmpl(0, std::complex<float>(1.0, 0.));
  equal->SetAmpl(1, std::complex<float>(0.0, 0.));

  std::vector<uint64_t> samples;
  equal->SampleState(0, &samples);
  ASSERT_EQ(samples.size(), 0);
}

TEST(StateSpaceTest, SampleEqual) {
  auto equal = std::unique_ptr<StateSpace>(GetStateSpace(1, 1));
  equal->CreateState();
  equal->SetAmpl(0, std::complex<float>(0.707, 0.));
  equal->SetAmpl(1, std::complex<float>(0.707, 0.));

  std::vector<uint64_t> samples;
  const int m = 100000;
  equal->SampleState(m, &samples);

  float num_ones = 0.0;
  for (int i = 0; i < m; i++) {
    if (samples[i]) {
      num_ones++;
    }
  }
  ASSERT_EQ(samples.size(), m);
  EXPECT_NEAR(num_ones / static_cast<float>(m), 0.5, 1E-2);
}

TEST(StateSpaceTest, SampleSkew) {
  auto skew = std::unique_ptr<StateSpace>(GetStateSpace(1, 1));
  skew->CreateState();

  std::vector<float> rots = {0.1, 0.3, 0.5, 0.7, 0.9};
  for (int t = 0; t < 5; t++) {
    float z_amp = std::sqrt(rots[t]);
    float o_amp = std::sqrt(1.0 - rots[t]);
    skew->SetAmpl(0, std::complex<float>(z_amp, 0.));
    skew->SetAmpl(1, std::complex<float>(o_amp, 0.));

    std::vector<uint64_t> samples;
    const int m = 100000;
    skew->SampleState(m, &samples);
    float num_z = 0.0;
    for (int i = 0; i < m; i++) {
      if (samples[i] == 0) {
        num_z++;
      }
    }
    ASSERT_EQ(samples.size(), m);
    EXPECT_NEAR(num_z / static_cast<float>(m), rots[t], 1E-2);
  }
}

TEST(StateSpaceTest, SampleComplexDist) {
  auto state = std::unique_ptr<StateSpace>(GetStateSpace(3, 1));
  state->CreateState();

  std::vector<float> probs = {0.05, 0.2, 0.05, 0.2, 0.05, 0.2, 0.05, 0.2};
  for (int i = 0; i < 8; i++) {
    state->SetAmpl(i, std::complex<float>(std::sqrt(probs[i]), 0.0));
  }

  std::vector<uint64_t> samples;
  const int m = 100000;
  state->SampleState(m, &samples);

  std::vector<float> measure_probs = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  for (int i = 0; i < m; i++) {
    measure_probs[samples[i]] += 1.0;
  }
  for (int i = 0; i < 8; i++) {
    EXPECT_NEAR(measure_probs[i] / static_cast<float>(m), probs[i], 1E-2);
  }
  ASSERT_EQ(samples.size(), m);
}

}  // namespace
}  // namespace qsim
}  // namespace tfq
