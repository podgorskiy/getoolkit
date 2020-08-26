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

import getoolkit
import bimpy


class App:
    r"""A base class for the application.

    .. note::
        You need to implement your own application class that derives from this class.

    .. warning::
        You will need to call `set_image` before you can run it

    Arguments:
        width (int): width of the window. Default: 600.
        height (int): height of the window. Default: 600.
        title (str): title of the window. Default: "Hello".


    Example:
        >>> import anntoolkit
        >>> import imageio
        >>>
        >>> class App(getoolkit.App):
        >>>     def __init__(self):
        >>>         # Calling constructor of base class
        >>>         super(App, self).__init__(title='Test')
        >>>
        >>>         # set image to view
        >>>         im = imageio.imread('test_image.jpg')
        >>>         self.set_image(im)
        >>>
        >>> # Run app
        >>> app = App()
        >>> app.run()
    """

    def __init__(self, width=600, height=600, title="Hello"):
        self._ctx = getoolkit.Context()
        self._ctx.init(width, height, title)
        self.world_width = 0
        self.world_height = 0

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
        bimpy.inject_imgui_context(self._ctx.get_imgui())

    def run(self):
        """Runs the application.

        .. note::
            This is a blocking call. It won't return until the window is closed

        Runs an event loop, where it process user input and updates window.
        Call callback method  :meth:`on_update`.

        Example:
            >>> app = App()
            >>> app.run()
        """
        while not self._ctx.should_close():
            with self._ctx:
                for k, v in self.keys.items():
                    self.keys[k] += 1
                    if v > 50:
                        self.on_keyboard(k, True, 0)
                        self.keys[k] = 45

                self.on_update()

    def on_update(self):
        """Is called each frame from the event loop that is run in :meth:`run` method

        Empty method, you need to overwrite it.
        Do all drawing from here.

        .. note::
            * Calls to draw API are valid only from within this method. Draw API: :meth:`point`,  :meth:`box`,  :meth:`text`, etc.

        .. warning::
            * Don't call it, this is callback

        """
        pass

    def on_mouse_button(self, down, x, y, lx, ly):
        """Is called on left mouse button event from event loop that is run in :meth:`run` method
        If the user presses left button on the mouse, this method is called

        Empty method, you need to overwrite it

        .. warning::
            * Don't call it, this is callback

        Arguments:
            down (bool): True if left mouse button is pressed. False if left mouse button is released.
            x (float): x coordinate of mouse cursor in window coordinate system
            y (float): y coordinate of mouse cursor in window coordinate system
            lx (float): x coordinate of mouse cursor in image coordinate system
            ly (float): y coordinate of mouse cursor in image coordinate system

        """
        pass

    def on_mouse_position(self, x, y, lx, ly):
        """Is called on mouse move event from event loop that is run in :meth:`run` method
        If the user moves mouse, this method is called

        Empty method, you need to overwrite it

        .. warning::
            * Don't call it, this is callback

        Arguments:
            x (float): x coordinate of mouse cursor in window coordinate system
            y (float): y coordinate of mouse cursor in window coordinate system
            lx (float): x coordinate of mouse cursor in image coordinate system
            ly (float): y coordinate of mouse cursor in image coordinate system

        """
        pass

    def on_keyboard(self, key, down, mods):
        """Is called on keybord key event from event loop that is run in :meth:`run` method

        Empty method, you need to overwrite it

        .. warning::
            * Don't call it, this is callback

        Arguments:
            key (int): The key that generated even. Can be `anntoolkit.KeyEscape`, `anntoolkit.KeyTab`,
                `anntoolkit.KeyBackspace`, `anntoolkit.KeyInsert`, `anntoolkit.KeyDelete`, `anntoolkit.KeyRight`, `anntoolkit.KeyLeft`, `anntoolkit.KeyDown`, `anntoolkit.KeyUp` or any char from 'A' to 'Z'
            down (bool): True if key is pressed. False if key is released.
            mods (int): Indicates additional modifing keys, such as Ctrl, Shift, Alt

        Example:
            >>> def on_keyboard(self, key, down, mods):
            >>>     if down:
            >>>         if key == getoolkit.KeyLeft:
            >>>             ...
            >>>         if key == getoolkit.KeyRight:
            >>>             ...
            >>>         if key == getoolkit.KeyUp:
            >>>             ...
            >>>         if key == getoolkit.KeyDown:
            >>>             ...
            >>>         if key == getoolkit.KeyDelete:
            >>>             ...
            >>>         if key == getoolkit.KeyBackspace:
            >>>             ...
            >>>         if key == 'R':
            >>>             ...
            >>>         if key == 'A':
            >>>             ...
            >>>         if key == 'Q':
            >>>             ...
        """
        pass

    def set_world_size(self, w, h, recenter=True):
        """ Sets the image to annotate

        .. note::
            Must be numpy ndarray of type numpy.uint8

        Should have 2 dims (grayscale) or 3 dims (colored) with the last dim of size 3 for RGB case or 4 for RGBA case.

        Example:
            >>> im = imageio.imread('test_image.jpg')
            >>> app.set_image(im)
        """
        # m = anntoolkit.generate_mipmaps(image)
        # self._ctx.set(anntoolkit.Image(m))
        self.world_width, self.world_height = w, h
        if recenter:
            self._ctx.set_world_size(w, h)
        else:
            self._ctx.set_without_recenter(w, h)

    def recenter(self):
        """ Resets zoom and recenters the image to fit in the window
        """

        self._ctx.recenter()

    def set_roi(self, roi, scale=0):
        """ Zooms and moves the image to view the requested roi
        """
        scale = 1.0 / scale
        x0, y0 = roi.left(), roi.top()
        x1, y1 = roi.left() + roi.width(), roi.top() + roi.height()
        self._ctx.set_roi(x0 * scale, y0 * scale, x1 * scale, y1 * scale)

    def text(self, s, x, y, color=None, color_bg=None, alignment=getoolkit.Alignment.Left):
        """Draw text in window space

        Arguments:
            s (str): Text
            x (int): x coordinate of the text in window space
            y (int): y coordinate of the text in window space
            color (tuple[int, int, int, int]): RGBA color of text. Default None, results in white
            color_bg (tuple[int, int, int, int]): RGBA color of background. Default None, results in black
            alignment (anntoolkit.Alignment): Alignment. Can be one of: `anntoolkit.Alignment.Left`,
                `anntoolkit.Alignment.Center`, `anntoolkit.Alignment.Right`. Default `anntoolkit.Alignment.Left`.
        """

        if color is None and color_bg is None:
            self._ctx.text(s, x, y, alignment)
        else:
            if color is None:
                raise ValueError
            if color_bg is None:
                color_bg = (0, 0, 0, 255)
            self._ctx.text(s, x, y, color, color_bg, alignment)

    def text_loc(self, s, lx, ly, color=None, color_bg=None, alignment=getoolkit.Alignment.Left):
        """Draw text in image space

        Arguments:
            s (str): Text
            x (int): x coordinate of the text in image space
            y (int): y coordinate of the text in image space
            color (tuple[int, int, int, int]): RGBA color of text
            color_bg (tuple[int, int, int, int]): RGBA color of background
            alignment (anntoolkit.Alignment): Alignment. Can be one of: `anntoolkit.Alignment.Left`,
                `anntoolkit.Alignment.Center`, `anntoolkit.Alignment.Right`. Default `anntoolkit.Alignment.Left`.
        """

        if color is None and color_bg is None:
            self._ctx.text_loc(s, lx, ly, alignment)
        else:
            if color is None:
                raise ValueError
            if color_bg is None:
                color_bg = (0, 0, 0, 255)
            self._ctx.text_loc(s, lx, ly, color, color_bg, alignment)

    def point(self, x, y, color, radius=5.0):
        """Draw point in image space

        Arguments:
            x (int): x coordinate of the text in image space
            y (int): y coordinate of the text in image space
            color (tuple[int, int, int, int]): RGBA color of text
            radius (float):radius of the point. Default 5.0.
        """
        self._ctx.point(x, y, color, radius)

    def window_2_world(self, x, y):
        """Convert window space to image space

        Arguments:
            x (float): x coordinate of the text in window space
            y (float): y coordinate of the text in window space

        Returns:
            tuple[float, float] - x, y coordinates in image space
        """
        return self._ctx.win_2_loc(x, y)

    def world_2_window(self, x, y):
        """Convert image space to window space

        Arguments:
            x (float): x coordinate of the text in image space
            y (float): y coordinate of the text in image space

        Returns:
            tuple[float, float] - x, y coordinates in window space
        """
        return self._ctx.loc_2_win(x, y)

    def transform(self, p):
        """Convert image space to window space

        Arguments:
            x (float): x coordinate of the text in image space
            y (float): y coordinate of the text in image space

        Returns:
            tuple[float, float] - x, y coordinates in window space
        """
        return self._ctx.loc_2_win(p)

    @property
    def scale(self):
        """Returns `scale` - change of length when transform from image space to window space

        If `scale` is 2, that means any line segment in window space will be twice longer than corresponding in image space

        Returns:
            float - scale
        """
        return self._ctx.get_scale()

    @property
    def encoder(self):
        return self._ctx.get_encoder()

    def box(self, box, color_stroke, color_fill):
        """Draw a box

        Arguments:
            box (list[tuple[float, float], tuple[float, float]]): coordinates of the box. List should be size of two
                contain x,y coordinats of two opposite corners of the box
            color_stroke (tuple[int, int, int, int]): color of the stroke
            color_fill (tuple[int, int, int, int]): color of the fill

        Returns:
            tuple[float, float] - x, y coordinates in window space
        """
        minx, miny = box[0]
        maxx, maxy = box[1]
        self._ctx.box(minx, miny, maxx, maxy, color_stroke, color_fill)

    @property
    def ctrl_v(self):
        return self._ctx.ctrl_v

    @property
    def ctrl_c(self):
        return self._ctx.ctrl_c

    @property
    def ctrl_x(self):
        return self._ctx.ctrl_x

    @property
    def width(self):
        """Width of the window
        """
        return self._ctx.width()

    @property
    def height(self):
        """Height of the window

        """
        return self._ctx.height()
