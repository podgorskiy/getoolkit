import anntoolkit
import imageio
import os
import pickle
import numpy as np
import random

LIBRARY_PATH = 'images'
SAVE_PATH = 'save.pth'


class App(anntoolkit.App):
    def __init__(self):
        super(App, self).__init__(title='Test')

        self.path = LIBRARY_PATH
        self.paths = []
        for dirName, subdirList, fileList in os.walk(self.path):
            self.paths += [os.path.relpath(os.path.join(dirName, x), self.path) for x in fileList if x.endswith('.jpg') or x.endswith('.jpeg') or x.endswith('.png')]

        self.paths.sort()
        self.iter = -1
        self.annotation = {}
        if os.path.exists(SAVE_PATH):
            with open(SAVE_PATH, 'rb') as f:
                self.annotation = pickle.load(f)

        self.moving = None
        self.load_next()

        print("Data size: %d" % len(self.annotation.items()))

    def load_next(self):
        self.iter += 1
        self.iter = self.iter % len(self.paths)
        im = imageio.imread(os.path.join(self.path, self.paths[self.iter]))
        self.set_image(im)

    def load_prev(self):
        self.iter -= 1
        self.iter = (self.iter + len(self.paths)) % len(self.paths)
        im = imageio.imread(os.path.join(self.path, self.paths[self.iter]))
        self.set_image(im)


    def load_next_not_annotated(self):
        while True:
            self.iter += 1
            self.iter = self.iter % len(self.paths)
            k = self.paths[self.iter]
            if k not in self.annotation:
                break
            if self.iter == 0:
                break
        try:
            im = imageio.imread(os.path.join(self.path, self.paths[self.iter]))
            self.set_image(im)
        except ValueError:
            self.load_next_not_annotated()

    def load_prev_not_annotated(self):
        while True:
            self.iter -= 1
            self.iter = (self.iter + len(self.paths)) % len(self.paths)
            k = self.paths[self.iter]
            if k not in self.annotation:
                break
            if self.iter == 0:
                break
        try:
            im = imageio.imread(os.path.join(self.path, self.paths[self.iter]))
            self.set_image(im)
        except ValueError:
            self.load_prev_not_annotated()

    def on_update(self):
        k = self.paths[self.iter]
        self.text(k, 10, 10)
        if k in self.annotation:
            self.text("Points count %d" % len(self.annotation[k]), 10, 50)
            for i, p in enumerate(self.annotation[k]):
                if i == self.moving:
                    self.point(*p, (0, 255, 0, 250))
                else:
                    self.point(*p, (255, 0, 0, 250))

            n = 2
            boxes = [self.annotation[k][i:i + n] for i in range(0, len(self.annotation[k]), n)]
            for box in boxes:
                if len(box) == 2:
                    self.box(box, (0, 255, 0, 250), (100, 255, 100, 50))

    def on_mouse_button(self, down, x, y, lx, ly):
        k = self.paths[self.iter]
        if down:
            if k in self.annotation:
                points = np.asarray(self.annotation[k])
                if len(points) > 0:
                    point = np.asarray([[lx, ly]])
                    d = points - point
                    d = np.linalg.norm(d, axis=1)
                    i = np.argmin(d)
                    if d[i] < 6:
                        self.moving = i
                        print(i, d[i])

        if not down:
            if k not in self.annotation:
                self.annotation[k] = []
            if self.moving is not None:
                self.annotation[k][self.moving] = (lx, ly)
                self.moving = None
            else:
                self.annotation[k].append((lx, ly))
            with open(SAVE_PATH, 'wb') as f:
                pickle.dump(self.annotation, f)

    def on_mouse_position(self, x, y, lx, ly):
        if self.moving is not None:
            k = self.paths[self.iter]
            self.annotation[k][self.moving] = (lx, ly)

    def on_keyboard(self, key, down, mods):
        if down:
            if key == anntoolkit.KeyLeft:
                self.load_prev()
            if key == anntoolkit.KeyRight:
                self.load_next()
            if key == anntoolkit.KeyUp:
                self.load_next_not_annotated()
            if key == anntoolkit.KeyDown:
                self.load_prev_not_annotated()
            if key == anntoolkit.KeyDelete:
                k = self.paths[self.iter]
                if k in self.annotation:
                    del self.annotation[k]
                with open(SAVE_PATH, 'wb') as f:
                    pickle.dump(self.annotation, f)
            if key == anntoolkit.KeyBackspace:
                k = self.paths[self.iter]
                if k in self.annotation and len(self.annotation[k]) > 0:
                    self.annotation[k] = self.annotation[k][:-1]
                with open(SAVE_PATH, 'wb') as f:
                    pickle.dump(self.annotation, f)
            if key == 'R':
                self.iter = random.randrange(len(self.paths))
                self.load_next()

app = App()

app.run()
