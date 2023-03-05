=========================
Autostart
=========================

Use the autostart feature to automatically start the game located in the same folder as the ScummVM executable, as long as it is a game supported by ScummVM. It is the equivalent of using the ``--path`` and ``--auto-detect`` :doc:`command line options <../advanced_topics/command_line>`.

There are two ways to use the autostart feature:

- Rename the executable file to ``scummvm-auto.xxx``, where ``.xxx`` is the file type extension (if applicable). On macOS the executable to rename is ``ScummVM.app/Contents/MacOS/scummvm`` and not the ``ScummVM.app`` application.
- Create an empty file named ``scummvm-autorun`` in the same folder as both the executable and the chosen game. Either leave the file empty, or use it to specify any further command line options. Specify one option per line.

Example of a scummvm-autorun file
===================================

.. code::

    --fullscreen
    --shader=crt/crt-lottes.glslp
    --path=./game/
    --auto-detect
