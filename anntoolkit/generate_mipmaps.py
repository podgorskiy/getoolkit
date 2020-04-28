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

import numpy as np
import warnings
from anntoolkit.downsample2x import downsample2x


def is_power_of_two(n):
    """Return True if n is a power of two."""

    if isinstance(n, list) or isinstance(n, tuple):
        return all(is_power_of_two(x) for x in n)
    if n <= 0:
        return False
    else:
        return n & (n - 1) == 0


def generate_mipmaps(image, gamma=2.2):
    img = image.astype(np.float32)
    img = np.power(img, gamma)

    max_val = img.max()
    img /= max_val

    downsample_type = 'bspline'
    if is_power_of_two(image.shape[:-1]) and image.dtype == np.float32:
        downsample_type = 'area_average'
        warnings.warn("bspline may produce artifacts with hdr. Texture is POT, so using area_average instead",
                      UserWarning)

    mipmaps = [image]
    for _ in range(100):
        img = downsample2x(img, type=downsample_type)
        img_to_save = img * max_val
        img_to_save = np.clip(img_to_save, 0, None)
        img_to_save = np.power(img_to_save, 1.0 / gamma)
        img_to_save = img_to_save.astype(image.dtype)
        mipmaps.append(img_to_save)
        if img_to_save.shape[0] == 1 and img_to_save.shape[1] == 1:
            break
    return mipmaps
