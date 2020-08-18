# Copyright 2020 Stanislav Pidhorskyi
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================

import scipy.ndimage
import numpy as np
import warnings
# import texture_tool


# static inline double sinc_filter(double x)
# {
#     if (x == 0.0)
#         return 1.0;
#     x = x * M_PI;
#     return sin(x) / x;
# }
#
# static inline double lanczos_filter(double x)
# {
#     /* truncated sinc */
#     if (-3.0 <= x && x < 3.0)
#         return sinc_filter(x) * sinc_filter(x/3);
#     return 0.0;
# }


def _downsample2x_area_average(input):
    input = np.asarray(input)
    r = []
    for plane in range(input.shape[-1]):
        _input = input[..., plane]
        for i in range(_input.ndim):
            zoom = [1.0] * (_input.ndim - 1) + [0.5]
            output_shape = [int(ii * jj) for ii, jj in zip(_input.shape, zoom)]
            output_shape = [1 if x == 0 else x for x in output_shape]
            output = _input.reshape(output_shape + [-1]).mean(axis=-1)
            _input = np.moveaxis(output, range(_input.ndim), range(-1, _input.ndim-1))
        r.append(_input)
    return np.stack(r, axis=-1)


def _downsample2x_bspline(input, order=3):
    if order < 0 or order > 5:
        raise RuntimeError('spline order not supported')
    input = np.asarray(input)
    r = []
    for plane in range(input.shape[-1]):
        _input = input[..., plane]
        if order > 1:
            filtered = scipy.ndimage.spline_filter(_input, order, output=np.float64)
        else:
            filtered = _input
        zoom = [0.5] * _input.ndim
        output_shape = tuple([int(ii * jj) for ii, jj in zip(_input.shape, zoom)])
        output_shape = [1 if x == 0 else x for x in output_shape]
        zoom_div = np.array(output_shape, float) - 1
        zoom = np.divide(np.array(_input.shape) - 1, zoom_div, out=np.ones_like(_input.shape, dtype=np.float64), where=zoom_div != 0)[:]
        output = np.zeros(output_shape, dtype=_input.dtype.name)
        scipy.ndimage.interpolation._nd_image.zoom_shift(filtered, zoom, None, output, order, 3, 0)
        r.append(output)
    return np.stack(r, axis=-1)


def downsample2x(input, type):
    if not texture_tool.is_power_of_two(input.shape[:-1]):
        if type == 'area_average':
            warnings.warn("area_average is not permitted for generating mipmaps for non power of two images. "
                          "Forcing bspline instead", UserWarning)
            type = 'bspline'

    if type == 'area_average':
        return _downsample2x_area_average(input)
    elif type == 'bspline':
        return _downsample2x_bspline(input)
    else:
        raise RuntimeError('Unknown downscaling algorithm')
