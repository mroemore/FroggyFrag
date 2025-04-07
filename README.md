![Happy little low-polygon frog, jumping in the air](https://github.com/mroemore/FroggyFrag/blob/main/bin/resources/Froggy.png)

# FroggyFrag
An OpenGL fragment shader preview tool, for assisting in shader development.

## Build Instructions
1. Install GCC.
2. Download the repository ``git clone https://github.com/mroemore/FroggyFrag/``.
3. Enter the project directory ``cd FroggyFrag``.
4. Run ``make release``.
5. Copy the contents of ``bin/`` to a location of your choice.
6. Run ``froggy-frag``.
## Usage Instructions
### Keybindings
- **[R]** Reload currently loaded shader.
- **[= or :arrow_right:]** Load next shader.
- **[- or :arrow_left:]** Load Previous Shader.
- **[F5]** Rescan shader directory.
- **[Q]** Show/hide message console.
- **[A]** Toggle automatic reloading of shaders.
### Config Settings
Edit the ``config.json`` file and restart the application to change these settings.
- **contentW, contentH** : changes the render resolution of the background image, irrespective of the image resolution.
- **screenW, screenH** : change the default shader and window resolution. resizing the application window after the program starts effectively overrides this.
- **shaderFileExtension** : default is ``.glsl`` but you can change this if you've used a different naming convention for your fragment shaders.
- **systemFontPath** : path to the default system font.
- **shaderFolder** : path to the folder which contains your shaders.
- **backgroundImagePath** : path to the default background image.
- **screenshotsFolder** : default save location for screenshots.
- **imagesFolder** : default location for image resources which shaders can be rendered over.
- **autoReload** : set this to ``false`` if you want to manually reload shaders.
- **reloadCheckInterval** : a value in seconds after which the application will check for changes in the currently selected shader file.
- **maintainContentAspectRatio** : set this to ``true`` if you would like the aspect ratio of your shader to remain the same after resizing the window. **[NOT YET IMPLEMENTED]**
- **copyOnDrag** : set this to ``true`` if you would like the application to copy images or shaders to the 'resources' directory when they are dragged onto the application window. **[NOT YET IMPLEMENTED]**

### Upcoming Features
- Drag and drop loading of images and shaders.
- Keybindings for cycling through background images.
- Aspect ratio settings.
- Config files for customization of shader variables.
