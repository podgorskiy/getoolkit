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

import anntoolkit


class App:
    def __init__(self, width=600, height=600, title="Hello"):
        self._ctx = anntoolkit.Context()
        self._ctx.init(width, height, title)

        def mouse_button(down, x, y, lx, ly):
            self.on_mouse_button(down, x, y, lx, ly)

        self._ctx.set_mouse_button_callback(mouse_button)

        def mouse_pos(x, y, lx, ly):
            self.on_mouse_position(x, y, lx, ly)

        self._ctx.set_mouse_position_callback(mouse_pos)

        def keyboard(key, action, mods):
            if key < 255:
                key = chr(key)
            if action == 1:
                self.keys[key] = 1
            elif action == 0:
                if key in self.keys:
                    del self.keys[key]
            self.on_keyboard(key, action == 1, mods)

        self._ctx.set_keyboard_callback(keyboard)
        self.keys = {}

    def run(self):
        while not self._ctx.should_close():
            with self._ctx:
                for k, v in self.keys.items():
                    self.keys[k] += 1
                    if v > 5:
                        self.on_keyboard(k, True, 0)
                        self.keys[k] = 1

                self.on_update()

    def on_update(self):
        pass

    def on_mouse_button(self, down, x, y, lx, ly):
        pass

    def on_mouse_position(self, x, y, lx, ly):
        pass

    def on_keyboard(self, key, down, mods):
        pass
        # print(chr(key), down, mods)

    def set_image(self, image):
        # m = anntoolkit.generate_mipmaps(image)
        # self._ctx.set(anntoolkit.Image(m))
        self._ctx.set(anntoolkit.Image([image]))

    def text(self, s, x, y):
        self._ctx.text(s, x, y)

    def point(self, x, y, color):
        self._ctx.point(x, y, color)
