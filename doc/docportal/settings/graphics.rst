===============
Graphics
===============

Use the Graphics tab to change how games look when they are played. 

**To change settings globally:** 

	From the Launcher, select **Options**. The **Graphics** tab is the first tab shown. 

**To change settings for a specific game:** 

	Highlight the game in the games list, select **Edit Game** then select the **Graphics** tab. 

For a comprehensive look at how to use these settings, check out our :doc:`../advanced_topics/understand_graphics` guide. 

.. figure:: ../images/settings/graphics.png

    The Graphics tab in the global settings

All settings can also be changed in the :doc:`../advanced_topics/configuration_file`. The configuration key is listed in italics after each setting description. 

,,,,,,,

.. _gfxmode:

Graphics mode
	Changes the resolution of the game. For example, a 2x graphics mode will take a 320x200 resolution game and scale it up to 640x400. 

	*gfx_mode* 

		
.. _render:

Render mode
	Changes how the game is rendered.

	*render_mode* 
			
.. _stretchmode:

Stretch mode
	Changes the way the game is displayed in relation to the window or screen size.

	*stretch_mode* 

.. _ratio:

Aspect ratio correction
	If ticked, corrects the aspect ratio so that games appear the same as they would on original 320x200 resolution displays. 

	*aspect_ratio* 

.. _fullscreen:

Fullscreen mode
	Switches between playing games in a window, or playing them in fullscreen mode. Switch between the two by using :kbd:`Alt+F5` while in a game.

	*fullscreen* 

.. _filtering:

Filter graphics
	If ticked, uses bilinear interpolation instead of nearest neighbor resampling for the aspect ratio correction and stretch mode. It does not affect the graphics mode. 

	*filtering* 


