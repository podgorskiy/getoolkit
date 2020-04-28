# Copyright 2019-2020 Stanislav Pidhorskyi
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

import sys
import os


def _handle_debugging():
    # if running debug session
    if os.path.exists("cmake-build-debug/"):
        print('Running Debugging session!')
        # sys.path.insert(0, "cmake-build-debug/")
        sys.path.insert(0, "cmake-build-release/")


_handle_debugging()


from _anntoolkit import *
from anntoolkit.generate_mipmaps import generate_mipmaps
from anntoolkit.app import App

# A hack to force sphinx to do the right thing
if 'sphinx' in sys.modules:
    print('Sphinx detected!!!')
    del os
    del sys
    __all__ = dir()
else:
    del os
    del sys


KeyEscape = 256
KeyEnter = 257
KeyTab = 258
KeyBackspace = 259
KeyInsert = 260
KeyDelete = 261
KeyRight = 262
KeyLeft = 263
KeyDown = 264
KeyUp = 265
