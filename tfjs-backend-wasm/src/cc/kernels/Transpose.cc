/* Copyright 2019 Google Inc. All Rights Reserved.
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
 * ===========================================================================*/

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "src/cc/backend.h"
#include "src/cc/util.h"

namespace {

template <typename T>
void transpose_2d(const T* x_data, const std::vector<int>& x_shape,
                  T* out_data) {
  const int d0 = x_shape[0];
  const int d1 = x_shape[1];
  const T* input = x_data;
  for (int i = 0; i < d0; ++i) {
    T* output = out_data + i;
    for (int j = 0; j < d1; ++j) {
      *output = *input;
      output += d0;
      ++input;
    }
  }
}

// Reference:
// https://github.com/tensorflow/tensorflow/blob/87388b7b6040bbf0baa67e4ef1ddc3e930ff6edd/tensorflow/lite/kernels/internal/optimized/optimized_ops.h#L7248
template <typename T>
void transpose_3d(const T* x_data, const std::vector<int>& x_shape,
                  const std::vector<int>& perm, T* out_data) {
  int s1, s2, s3;
  s1 = x_shape[0];
  s2 = x_shape[1];
  s3 = x_shape[2];

  int p1, p2, p3;
  if (perm[0] == 2) {
    p1 = 1;
  } else if (perm[1] == 2) {
    p2 = 1;
  } else {
    p3 = 1;
  }

  if (perm[0] == 1) {
    p1 = s3;
  } else if (perm[1] == 1) {
    p2 = s3;
  } else {
    p3 = s3;
  }

  if (perm[0] == 0) {
    p1 = s2 * s3;
  } else if (perm[1] == 0) {
    p2 = s2 * s3;
  } else {
    p3 = s2 * s3;
  }

  int o_s[3];
  o_s[0] = x_shape[perm[0]];
  o_s[1] = x_shape[perm[1]];
  o_s[2] = x_shape[perm[2]];

  for (int i1 = 0; i1 < o_s[0]; ++i1) {
    for (int i2 = 0; i2 < o_s[1]; ++i2) {
      for (int i3 = 0; i3 < o_s[2]; ++i3) {
        const int i = i1 * p1 + i2 * p2 + i3 * p3;
        const int o = i1 * o_s[1] * o_s[2] + i2 * o_s[2] + i3;
        out_data[o] = x_data[i];
      }
    }
  }
}

template <typename T>
void transpose(const T* x_data, const std::vector<int>& x_shape,
               const std::vector<int>& perm, T* out_data) {
  if (x_shape.size() == 2) {
    transpose_2d(x_data, x_shape, out_data);
  } else if (x_shape.size() == 3) {
    transpose_3d(x_data, x_shape, perm, out_data);
  } else {
    tfjs::util::warn("WASM Transpose kernel does not yet support rank %d",
                     x_shape.size());
  }
}
}  // namespace

namespace tfjs {
namespace wasm {
// We use C-style API to interface with Javascript.
extern "C" {

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void Transpose(int x_id, int* x_shape_ptr, int x_shape_length, int out_id,
               int* out_shape_ptr, int out_shape_length, int* perm_ptr,
               int perm_length) {
  auto x_shape = std::vector<int>(x_shape_ptr, x_shape_ptr + x_shape_length);
  auto out_shape =
      std::vector<int>(out_shape_ptr, out_shape_ptr + out_shape_length);
  auto perm = std::vector<int>(perm_ptr, perm_ptr + perm_length);
  const TensorInfo x_info = backend::get_tensor_info(x_id);
  const TensorInfo out_info = backend::get_tensor_info(out_id);

  switch (x_info.dtype) {
    case DType::float32:
      transpose<float>(x_info.buf.f32, x_shape, perm, out_info.buf.f32);
      break;
    case DType::int32:
      transpose<int>(x_info.buf.i32, x_shape, perm, out_info.buf.i32);
      break;
    case DType::boolean:
      transpose<bool>(x_info.buf.b, x_shape, perm, out_info.buf.b);
      break;
    default:
      util::warn("Transpose for tensor id %d failed. Unknown dtype %d", x_id,
                 x_info.dtype);
  }
}

}  // extern "C"
}  // namespace wasm
}  // namespace tfjs