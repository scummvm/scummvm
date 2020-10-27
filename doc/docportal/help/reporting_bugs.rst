
=======================
Reporting a bug
=======================

To report a bug, go to the ScummVM `Issue Tracker <https://bugs.scummvm.org/>`_ and log in with your GitHub account. 

Please make sure the bug is reproducible, and still occurs in the latest git/Daily build version. Also check the `compatibility list <https://www.scummvm.org/compatibility/>`_ for that game, to ensure the issue is not already known.

Please do not report bugs for games that are not listed as being completeable on the `Supported Games <https://wiki.scummvm.org/index.php?title=Category:Supported_Games>`_ wiki page, or on the compatibility list. We know those games have bugs.

Please include the following information in the bug report:

- ScummVM version (PLEASE test the latest git/Daily build)
- Bug details, including instructions for how to reproduce the bug
- Language of the game 
- Version of the game (talkie, floppy, and so on)
- Platform and Compiler (Win32, Linux, FreeBSD, and so on)
- Attach a saved game if possible 
- If this bug only occurred recently, please note the last version without the bug, and the first version including the bug. That way we can fix it quicker by looking at the changes made.

Finally, please report each issue separately; do not file multiple issues on the same ticket. It gets difficult to track the status of each individual bug when they aren't on their own tickets. 

.. _logfile: 

The ScummVM log file
=====================

You can find error messages in the ScummVM log file. The location of this file varies depending on your operating system. 

.. tabs::

    .. tab:: Linux

            We use the XDG Base Directory Specification, so by default the file will be ``~/.cache/scummvm/logs/scummvm.log`` but its location may vary depending on the value of the ``XDG_CACHE_HOME`` environment variable. 


    .. tab:: Windows

            ``%appdata%\ScummVM\Logs\scummvm.log``

    
	.. tab:: macOS
		``~/Library/Logs/scummvm.log``, where ``~`` is your Home directory. 
    
	