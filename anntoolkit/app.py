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
        self.image = None

    def run(self):
        while not self._ctx.should_close():
            with self._ctx:
                for k, v in self.keys.items():
                    self.keys[k] += 1
                    if v > 50:
                        self.on_keyboard(k, True, 0)
                        self.keys[k] = 45

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

    def set_image(self, image, recenter=True):
        # m = anntoolkit.generate_mipmaps(image)
        # self._ctx.set(anntoolkit.Image(m))
        self.image = image
        if recenter:
            self._ctx.set(anntoolkit.Image([image]))
        else:
            self._ctx.set_without_recenter(anntoolkit.Image([image]))

    def recenter(self):
        self._ctx.recenter()

    def set_roi(self, roi, scale=0):
        scale = 1.0 / scale
        x0, y0 = roi.left(), roi.top()
        x1, y1 = roi.left() + roi.width(), roi.top() + roi.height()
        self._ctx.set_roi(x0 * scale, y0 * scale, x1 * scale, y1 * scale)

    def text(self, s, x, y, color=None, color_bg=None):
        if color is None and color_bg is None:
            self._ctx.text(s, x, y)
        else:
            if color is None:
                raise ValueError
            if color_bg is None:
                color_bg = (0, 0, 0, 255)
            self._ctx.text(s, x, y, color, color_bg)

    def text_loc(self, s, lx, ly, color=None, color_bg=None):
        if color is None and color_bg is None:
            self._ctx.text_loc(s, lx, ly)
        else:
            if color is None:
                raise ValueError
            if color_bg is None:
                color_bg = (0, 0, 0, 255)
            self._ctx.text_loc(s, lx, ly, color, color_bg)

    def point(self, x, y, color):
        self._ctx.point(x, y, color)

    def box(self, box, color_stroke, color_fill):
        minx, miny = box[0]
        maxx, maxy = box[1]
        self._ctx.box(minx, miny, maxx, maxy, color_stroke, color_fill)
