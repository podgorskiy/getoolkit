# Hello world
Subclass from `anntoolkit.App`
In init method load some test image.

``` python
import anntoolkit

class App(anntoolkit.App):
    def __init__(self):
        super(App, self).__init__(title='Test')        
        im = imageio.imread('test_image.jpg')
        self.set_image(im)

```
Run app:

``` python
app = App()
app.run()
```

Result:

![](https://user-images.githubusercontent.com/3229783/90511347-2c2c0f00-e111-11ea-91eb-a918f2f55288.png)
![](https://user-images.githubusercontent.com/3229783/90512523-21727980-e113-11ea-87b1-f79d76761f7a.gif)
