import anntoolkit

ctx = anntoolkit.Context()

ctx.init(600, 600, "Hello")

while not ctx.should_close():
    with ctx:
        pass
