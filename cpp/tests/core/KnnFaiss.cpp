// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------
#include "open3d/core/nns/KnnFaiss.h"

#include <cmath>
#include <limits>

#include "core/CoreTest.h"
#include "open3d/core/Device.h"
#include "open3d/core/Dtype.h"
#include "open3d/core/SizeVector.h"
#include "open3d/core/Tensor.h"
#include "open3d/utility/Helper.h"
#include "tests/UnitTest.h"

using namespace open3d;
using namespace std;

namespace open3d {
namespace tests {

TEST(KnnFaiss, KnnSearch) {
    // KnnSearch test assumes system has at least one GPU device.
    // Set up faiss index.
    int size = 10;
    core::Device device("CUDA:0");
    std::vector<float> points{0.0, 0.0, 0.0, 0.0, 0.0, 0.1, 0.0, 0.0, 0.2, 0.0,
                              0.1, 0.0, 0.0, 0.1, 0.1, 0.0, 0.1, 0.2, 0.0, 0.2,
                              0.0, 0.0, 0.2, 0.1, 0.0, 0.2, 0.2, 0.1, 0.0, 0.0};
    core::Tensor ref(points, {size, 3}, core::Dtype::Float32, device);
    core::nns::KnnFaiss faiss_index(ref);

    core::Tensor query(std::vector<float>({0.064705, 0.043921, 0.087843}),
                       {1, 3}, core::Dtype::Float32);
    std::pair<core::Tensor, core::Tensor> result;
    core::Tensor indices;
    core::Tensor distances;

    // If k <= 0.
    EXPECT_THROW(faiss_index.SearchKnn(query, -1), std::runtime_error);
    EXPECT_THROW(faiss_index.SearchKnn(query, 0), std::runtime_error);

    // If k == 3.
    result = faiss_index.SearchKnn(query, 3);
    indices = result.first;
    distances = result.second;
    ExpectEQ(indices.ToFlatVector<int64_t>(), std::vector<int64_t>({1, 4, 9}));
    ExpectEQ(distances.ToFlatVector<float>(),
             std::vector<float>({0.00626358, 0.00747938, 0.0108912}));

    // If k > size.result.
    result = faiss_index.SearchKnn(query, 12);
    indices = result.first;
    distances = result.second;
    ExpectEQ(indices.ToFlatVector<int64_t>(),
             std::vector<int64_t>({1, 4, 9, 0, 3, 2, 5, 7, 6, 8}));
    ExpectEQ(distances.ToFlatVector<float>(),
             std::vector<float>({0.00626358, 0.00747938, 0.0108912, 0.0138322,
                                 0.015048, 0.018695, 0.0199108, 0.0286952,
                                 0.0362638, 0.0411266}));

    // Multiple points.
    query = core::Tensor(std::vector<float>({0.064705, 0.043921, 0.087843,
                                             0.064705, 0.043921, 0.087843}),
                         {2, 3}, core::Dtype::Float32);
    result = faiss_index.SearchKnn(query, 3);
    indices = result.first;
    distances = result.second;
    ExpectEQ(indices.ToFlatVector<int64_t>(),
             std::vector<int64_t>({1, 4, 9, 1, 4, 9}));
    ExpectEQ(distances.ToFlatVector<float>(),
             std::vector<float>({0.00626358, 0.00747938, 0.0108912, 0.00626358,
                                 0.00747938, 0.0108912}));
}

TEST(KnnFaiss, HybridSearch) {
    // HybridSearch test assumes system has at least one GPU device.
    // Set up nns.
    int size = 10;

    std::vector<float> points{0.0, 0.0, 0.0, 0.0, 0.0, 0.1, 0.0, 0.0, 0.2, 0.0,
                              0.1, 0.0, 0.0, 0.1, 0.1, 0.0, 0.1, 0.2, 0.0, 0.2,
                              0.0, 0.0, 0.2, 0.1, 0.0, 0.2, 0.2, 0.1, 0.0, 0.0};
    core::Tensor ref(points, {size, 3}, core::Dtype::Float32);
    core::nns::KnnFaiss faiss_index(ref);

    core::Tensor query(std::vector<float>({0.064705, 0.043921, 0.087843}),
                       {1, 3}, core::Dtype::Float32);

    std::pair<core::Tensor, core::Tensor> result =
            faiss_index.SearchHybrid(query, 0.1, 1);

    core::Tensor indices = result.first;
    core::Tensor distainces = result.second;
    ExpectEQ(indices.ToFlatVector<int64_t>(), std::vector<int64_t>({1}));
    ExpectEQ(distainces.ToFlatVector<float>(),
             std::vector<float>({0.00626358}));
}

}  // namespace tests
}  // namespace open3d
