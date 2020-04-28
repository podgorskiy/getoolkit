import anntoolkit
import imageio


ctx = anntoolkit.Context()

ctx.init(600, 600, "Hello")

im = imageio.imread('test.jpg')

m = anntoolkit.generate_mipmaps(im)

image = anntoolkit.Image(m)

ctx.set(image)


def mouse(down, x, y, lx, ly):
    print(down, x, y, lx, ly)


ctx.set_mouse_button_callback(mouse)

while not ctx.should_close():
    with ctx:
        pass
