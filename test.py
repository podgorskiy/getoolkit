import getoolkit
import imageio
import os
import pickle
import numpy as np
import random
import bimpy
import random

LIBRARY_PATH = 'images'
SAVE_PATH = 'save.pth'


class Node(object):
    def __init__(self, name='', children=None):
        self.name = name
        self.children = children if children is not None else []
        self.id = random.randint(0, 1000000)
        self.width = 10.
        self.height = 10.
        self.pos_x = 0.
        self.pos_y = 0.


class App(getoolkit.App):
    def __init__(self):
        super(App, self).__init__(title='Test')

        self.path = LIBRARY_PATH
        self.paths = []
        for dirName, subdirList, fileList in os.walk(self.path):
            self.paths += [os.path.relpath(os.path.join(dirName, x), self.path) for x in fileList if x.endswith('.jpg') or x.endswith('.jpeg') or x.endswith('.png')]

        self.paths.sort()

        self.moving = None
        self.nearest = None
        self.cursor_pos_window = (0, 0)
        self.cursor_pos_world = (0, 0)
        self.cursor_pos_world_grid_snapped = (0, 0)

        self.set_world_size(1000, 1000)
        self.grid_snap_step = 5
        self.root_nodes = []
        self.selected_node = None
        if os.path.exists(SAVE_PATH):
            with open(SAVE_PATH, 'rb') as f:
                self.root_nodes = pickle.load(f)
        self.copied_node = None

    def new_node(self):
        if self.selected_node is not None:
            self.selected_node.children.append(Node('New node'))
        else:
            self.root_nodes.append(Node('New node'))

    def grid_snap(self, pos):
        x, y = pos
        x = int(round(x / self.grid_snap_step)) * self.grid_snap_step
        y = int(round(y / self.grid_snap_step)) * self.grid_snap_step
        return x, y

    def on_update(self):
        y = 30
        self.text('Window size: %dx%d' % (self.width, self.height), 10, y)
        y += 30
        self.text('World size: %dx%d' % (self.world_width, self.world_height), 10, y)
        y += 30
        self.text('Scale %f' % self.scale, 10, y)
        y += 30
        self.text('%f x %f' % self.cursor_pos_world, 10, y)
        y += 30

        self.point(*self.cursor_pos_world, (230, 10, 10, 240), radius=2.)

        if bimpy.button("New Node"):
            self.new_node()

        def pnodes(l):
            remove_n = None

            for n in l:
                node_flags = bimpy.OpenOnArrow | bimpy.OpenOnDoubleClick | bimpy.DefaultOpen
                if n == self.selected_node:
                    node_flags |= bimpy.Selected

                if len(n.children) == 0:
                    node_flags |= bimpy.Leaf | bimpy.NoTreePushOnOpen

                node_open = bimpy.tree_node_ex(n.id, node_flags, n.name)
                if bimpy.is_item_clicked():
                    if self.selected_node == n:
                         self.selected_node = None
                    else:
                        self.selected_node = n

                if self.selected_node is not None and self.selected_node == n and self.ctrl_x:
                    self.copied_node = n
                    self.selected_node = None
                    remove_n = n

                if node_open and len(n.children) != 0:
                    pnodes(n.children)
                    bimpy.tree_pop()

            if remove_n is not None:
                l.remove(remove_n)

        if self.copied_node is not None and self.selected_node is not None and self.ctrl_v:
            self.selected_node.children.append(self.copied_node)
            self.copied_node = None

        pnodes(self.root_nodes)

        has_selection = self.selected_node is not None

        nstr = bimpy.String(self.selected_node.name if has_selection else '')
        bimpy.input_text('Name', nstr, 256)

        nwidth = bimpy.Float(self.selected_node.width if has_selection else 0.)
        nheigth = bimpy.Float(self.selected_node.height if has_selection else 0.)
        bimpy.input_float2('size', nwidth, nheigth)

        nposx = bimpy.Float(self.selected_node.pos_x if has_selection else 0.)
        nposy = bimpy.Float(self.selected_node.pos_y if has_selection else 0.)
        bimpy.input_float2('position', nposx, nposy)

        if has_selection:
            self.selected_node.name = nstr.value
            self.selected_node.width = nwidth.value
            self.selected_node.height = nheigth.value
            self.selected_node.pos_x = nposx.value
            self.selected_node.pos_y = nposy.value

        bimpy.text("ID: %s" % (str(self.selected_node.id) if has_selection else ''))

        def dnodes(l, p_pox=0, p_poy=0):
            for n in l:
                minp = getoolkit.vec2(n.pos_x + p_pox, n.pos_y + p_poy)
                minp = self.transform(minp)
                maxp = getoolkit.vec2(n.pos_x + n.width + p_pox, n.pos_y + n.height + p_poy)
                maxp = self.transform(maxp)
                self.encoder.rect(minp, maxp, (200, 100, 150, 200))

                if len(n.children) != 0:
                    dnodes(n.children, n.pos_x + p_pox, n.pos_y + p_poy)

        dnodes(self.root_nodes)


        # if k in self.annotation:
        #     self.text("Points count %d" % len(self.annotation[k]), 10, 50)
        #     for i, p in enumerate(self.annotation[k]):
        #         r = 8 if i == self.nearest else 5
        #         if i == self.moving:
        #             self.point(*p, (0, 255, 0, 250), radius=r)
        #         else:
        #             self.point(*p, (255, 0, 0, 250), radius=r)
        #
        #     n = 2
        #     boxes = [self.annotation[k][i:i + n] for i in range(0, len(self.annotation[k]), n)]
        #     for box in boxes:
        #         if len(box) == 2:
        #             self.text_loc("I'm label", *box[0], (0, 10, 0, 250), (150, 255, 150, 150))
        #             self.box(box, (0, 255, 0, 250), (100, 255, 100, 50))

    def get_nearset(self, points_pool, lx, ly, threshold=6):
        points = np.asarray(points_pool)
        if len(points) > 0:
            point = np.asarray([[lx, ly]])
            d = points - point
            d = np.linalg.norm(d, axis=1)
            i = np.argmin(d)
            distance_to_nearset_point = d[i]
            distance_in_window_space = self.scale * distance_to_nearset_point
            if distance_in_window_space < threshold:
                return i
        return None

    def on_mouse_button(self, down, x, y, lx, ly):
        pass
        # k = self.paths[self.iter]
        # if down and self.nearest is not None:
        #     self.moving = self.nearest
        #
        # if not down:
        #     if k not in self.annotation:
        #         self.annotation[k] = []
        #     if self.moving is not None:
        #         self.annotation[k][self.moving] = (lx, ly)
        #         self.moving = None
        #     else:
        #         self.annotation[k].append((lx, ly))
        #     with open(SAVE_PATH, 'wb') as f:
        #         pickle.dump(self.annotation, f)

    def on_mouse_position(self, x, y, lx, ly):
        self.cursor_pos_window = (x, y)
        self.cursor_pos_world = self.grid_snap((lx, ly))
        # self.nearest = self.get_nearset(lx, ly)
        # if self.moving is not None:
        #     k = self.paths[self.iter]
        #     self.annotation[k][self.moving] = (lx, ly)

    def on_keyboard(self, key, down, mods):
        if down:
            if key == getoolkit.SpecialKeys.KeyLeft:
                pass
            if key == getoolkit.SpecialKeys.KeyRight:
                pass
            # if key == getoolkit.SpecialKeys.KeyUp:
            #     self.load_next_not_annotated()
            # if key == getoolkit.SpecialKeys.KeyDown:
            #     self.load_prev_not_annotated()
            # if key == getoolkit.SpecialKeys.KeyDelete:
            #     k = self.paths[self.iter]
            #     if k in self.annotation:
            #         del self.annotation[k]
            #     with open(SAVE_PATH, 'wb') as f:
            #         pickle.dump(self.annotation, f)
            if key == getoolkit.SpecialKeys.KeyDelete:
                with open(SAVE_PATH, 'wb') as f:
                    pickle.dump(self.root_nodes, f)



app = App()
app.run()
