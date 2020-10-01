
============================================
Graphics
============================================

ScummVM has several ways to adjust the graphics of games, and these settings can be found in the Graphics tabs of both the :doc:`global <../using_scummvm/global_settings>` and :doc:`game-specific <../using_scummvm/game_settings>` settings. They can also be changed directly in the :doc:`configuration file <../advanced_options/configuration_file>`, or passed as an option on the :doc:`command line <../advanced_options/command_line>`.   

The graphics mode, aspect ratio, and stretch mode settings work together to change the resolution and display of each game. 

Render modes are only applicable to some older games that were designed to be played on multiple systems, and lets us choose which system's graphics to replicate. 



Graphics mode
---------------

The first setting applied to the image is the graphics mode. 

The original game graphics are upscaled using different graphical filters, which are specialized algorithms used to ensure that low resolution pixel-art still looks good when it is displayed at a higher resolution.

If the game originally ran at a resolution of 320x200 - which is typical for most SCUMM games - then using a graphics mode with a scale factor of 2x will yield 640x400 graphics. A 3x scale factor will yield 960x600.


Comparison of graphics modes
******************************

.. figure:: ../images/graphics/graphics_mode/1x.png

    **1x**: No filtering, no scaling. Fastest.
   
.. figure:: /images/graphics/graphics_mode/2x.png

   **2x**: No filtering, factor 2x. Default for non 640x480 games.

.. figure:: ../images/graphics/graphics_mode/3x.png
   
   **3x**: No filtering, factor 3x.

.. figure:: ../images/graphics/graphics_mode/2xsai.png

    **2xSaI**: SaI filter, factor 2x.

.. figure:: ../images/graphics/graphics_mode/super2xsai.png

    **Super2xSaI**: Enhanced 2xSAI filtering, factor 2x.

.. figure:: ../images/graphics/graphics_mode/supereagle.png

    **SuperEagle**: Less blurry than 2xSAI, but slower. Factor 2x.

.. figure:: ../images/graphics/graphics_mode/advmame2x.png

    **AdvMAME2x**: Doesn't rely on blurring like 2xSAI, fast. Factor 2x.

.. figure:: ../images/graphics/graphics_mode/advmame3x.png

    **AdvMAME3x**: Doesn't rely on blurring like 2xSAI, fast. Factor 3x.

.. figure:: ../images/graphics/graphics_mode/hq2x.png

    **HQ2x**: Very nice high quality filter, but slow. Factor 2x.

.. figure:: ../images/graphics/graphics_mode/hq3x.png

    **HQ3x**: Very nice high quality filter, but slow. Factor 3x.

.. figure:: ../images/graphics/graphics_mode/tv2x.png

    **TV2x**: Interlace filter, tries to emulate a TV. Factor 2x.

.. figure:: ../images/graphics/graphics_mode/dotmatrix.png

    **DotMatrix**: Dot matrix effect. Factor 2x.



.. note::

    Not all engines support all, or even any, of the graphics modes. 
    
    There is always a speed penalty when using any form of anti-aliasing/linear filtering.



Aspect Ratio
-------------
The next setting applied to the image is the aspect ratio correction.

Older games were designed to be run at 320x200 pixels, but on systems where each pixel was rectangular instead of square. This means that on modern systems these games look wider and flatter than they are supposed to. Aspect ratio correction duplicates lines of pixels to correct this. 

.. figure:: ../images/graphics/aspect_ratio/no_aspect_ratio.png

    No aspect ratio correction applied.

.. figure:: ../images/graphics/aspect_ratio/aspect_ratio.png

    Aspect ratio correction applied. 

Stretch mode
----------------

The last setting applied to the image is the stretch mode. 

There are five stretch modes:

- Center - centers the image in the window. 
- Pixel-perfect scaling - keeps the image at its resolution.
- Fit to window - fits the image to the window, but maintains the aspect ratio; does not stretch it to fill the window.
- Stretch - stretches the image to fill the window
-  Fit to window (4:3) - fits the image to the window, at a 4:3 aspect ratio.

Render mode
-------------

For most games this setting will have no effect. For some of the older games that could be played on different systems and graphics cards, this control allows us to decide which system we want ScummVM to reproduce. 

Below are some of the common render modes, to illustrate how the render mode setting works. 

.. figure:: ../images/graphics/render_mode/default.png
   
    Maniac Mansion with <default> render mode

.. figure:: ../images/graphics/render_mode/herc_green.png

    Maniac Mansion with Hercules Green render mode

.. figure:: ../images/graphics/render_mode/herc_amber.png

    Maniac Mansion with Hercules Amber render mode

.. figure:: ../images/graphics/render_mode/cga.png

    Maniac Mansion with CGA (4 color) render mode

.. figure:: ../images/graphics/render_mode/ega.png

    Maniac Mansion with EGA (16 color) render mode

.. figure:: ../images/graphics/render_mode/amiga.png

    Maniac Mansion with Amiga (32 color) render mode


