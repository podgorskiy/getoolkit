import anntoolkit
import imageio
import os
import pickle


class App(anntoolkit.App):
    def __init__(self):
        super(App, self).__init__(title='Test')

        self.path = 'images'
        self.paths = list(os.listdir(self.path))
        self.paths = [x for x in self.paths if x.endswith('.jpg') or x.endswith('.jpeg') or x.endswith('.png')]
        self.paths.sort()
        self.iter = -1
        self.load_next()
        self.annotation = {}
        if os.path.exists('save.pth'):
            with open('save.pth', 'rb') as f:
                self.annotation = pickle.load(f)

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

    def on_update(self):
        k = self.paths[self.iter]
        self.text(k, 10, 10)
        if k in self.annotation:
            for p in self.annotation[k]:
                self.point(*p, (255, 0, 0, 250))

    def on_mouse_button(self, down, x, y, lx, ly):
        if not down:
            k = self.paths[self.iter]
            if k not in self.annotation:
                self.annotation[k] = []
            self.annotation[k].append((lx, ly))
            with open('save.pth', 'wb') as f:
                pickle.dump(self.annotation, f)

    def on_keyboard(self, key, down, mods):
        if down:
            if key == anntoolkit.KeyLeft:
                self.load_prev()
            if key == anntoolkit.KeyRight:
                self.load_next()
            if key == anntoolkit.KeyDelete:
                k = self.paths[self.iter]
                if k in self.annotation:
                    del self.annotation[k]
            if key == anntoolkit.KeyBackspace:
                k = self.paths[self.iter]
                if k in self.annotation and len(self.annotation[k]) > 0:
                    self.annotation[k] = self.annotation[k][:-1]


app = App()

app.run()
