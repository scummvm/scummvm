=========================
Autostart
=========================

Use the autostart feature to automatically start or add games. The available features and the way they work depend on the system on which ScummVM is running.


.. tabbed:: General

    .. panels::
        :column: col-lg-12

        On most systems, such as Windows and Linux, ScummVM can detect and start a game located in the same folder as the ScummVM executable, as long as it is a game supported by ScummVM. It is the equivalent of using the ``--path`` and ``--auto-detect`` :doc:`command line options <../advanced_topics/command_line>`. This also works on macOS when ScummVM is built as a command line application and not as a .app bundle.

        There are two ways to use the autostart feature:

        - Rename the executable file to ``scummvm-auto.xxx``, where ``.xxx`` is the file type extension (if applicable).
        - Create an empty file named ``scummvm-autorun`` in the same folder as both the executable and the chosen game. Either leave the file empty, or use it to specify any further command line options. Specify one option per line.

.. tabbed:: macOS

    .. panels::
        :column: col-lg-12

        The following information are only correct when ScummVM is built as a .app bundle. When ScummVM is built as a command line application it behaves as on Windows and Linux.

        ScummVM can automatically detect and run a supported game included in the ScummVM.app bundle. The game files simply need to be in the ``ScummVM.app/Contents/Resources/game/`` folder.

        ScummVM can also automatically detect and add to the launcher supported games included in the ScummVM.app bundle. The games simply need to be in the ``ScummVM.app/Contents/Resources/games/`` folder, or subfolders in that folder (you can for example use one subfolder per game).

        A ``scummvm-autorun`` file can also be used to specify command line options to use, with one option per line. The file should be in the  ``ScummVM.app/Contents/Resources/`` folder.

        In addition if a ``scummvm.ini`` file is present in the ``ScummVM.app/Contents/Resources/`` folder, it will be used as initial config file if no ``ScummVM Preferences`` file exists yet. It is the equivalent of using the ``--initial-cfg`` :doc:`command line option <../advanced_topics/command_line>`.

        Finally it can be noted that the bundle name specified in the ``ScummVM.app/Contents/Info.plist`` file is used for the default config file name. For example by default, when the bundle name is ``ScummVM``, the default config file where settings are saved is ``~/Library/Preferences/ScummVM Preferences``. But if you change the bundle name to ``MyGame``, the default config file used is ``~/Library/Preferences/MyGame Preferences``

        Thus to create a game bundle for a specific game you can:

        1. Copy the ScummVM.app bundle for example to MyGame.app
        2. Create a MyGame.app/Contents/Resources/game/ folder and copy your game data files to that folder.
        3. Edit the bundle name and bundle display name in the MyGame.app/Contents/Info.plist file.
        4. Create a new icons file in MyGame.app/Contents/Resources/ and edit the icon file in the MyGame.app/Contents/Info.plist file.
        5. Create a MyGame.app/Contents/Resources/scummvm.ini file with the default settings for your game bundle (for example to set fullscreen to true).

        Note that modifying a signed bundle will invalidate the signature. So the bundle needs to be signed again after for example adding a ``game`` folder inside the ScummVM.app bundle.

.. tabbed:: iOS

    .. panels::
        :column: col-lg-12

        ScummVM can automatically detect and run a supported game included in the ScummVM.app bundle. The game files simply need to be in the ``ScummVM.app/game/`` folder.

        ScummVM can also automatically detect and add to the launcher supported games included in the ScummVM.app bundle. The games simply need to be in the ``ScummVM.app/games/`` folder, or subfolders in that folder (you can for example use one subfolder per game).

        A ``scummvm-autorun`` file can also be used to specify command line options to use, with one option per line. The file should be in the  ``ScummVM.app/`` folder.

        In addition if a ``scummvm.ini`` file is present in the ``ScummVM.app/`` folder, it will be used as initial config file if no config file exists yet. It is the equivalent of using the ``--initial-cfg`` :doc:`command line option <../advanced_topics/command_line>`.

        Note that modifying a signed bundle will invalidate the signature. So the bundle needs to be signed again after for example adding a ``game`` folder inside the ScummVM.app bundle.



Example of a scummvm-autorun file
===================================

.. code::

    --fullscreen
    --shader=crt/crt-lottes.glslp
    --path=./game/
    --auto-detect
