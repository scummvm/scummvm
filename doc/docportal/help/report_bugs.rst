
=======================
Report a bug
=======================

To report a bug, go to the ScummVM `Issue Tracker <https://bugs.scummvm.org/>`_ and log in with your GitHub account. 

Please make sure the bug is reproducible, and still occurs in the latest git/Daily build version. Also check the `compatibility list <https://www.scummvm.org/compatibility/>`_ for that game, to ensure the issue is not already known. Please do not report bugs for games that are not listed as completeable on the `Supported Games <https://wiki.scummvm.org/index.php?title=Category:Supported_Games>`_ wiki page, or on the compatibility list. We already know those games have bugs!

Please include the following information in the bug report:

- ScummVM version (test the latest git/Daily build)
- Bug details, including instructions for how to reproduce the bug. If possible, include log files, screenshots, and any other relevant information. 
- Game language
- Game version (for example, talkie or floppy)
- Platform and Compiler (for example, Win32, Linux or FreeBSD)
- An attached saved game, if possible. 
- If this bug only occurred recently, include the last version without the bug, and the first version with the bug. That way we can fix it quicker by looking at the changes made.

Finally, please report each issue separately; do not file multiple issues on the same ticket. It is difficult to track the status of each individual bug when they aren't on their own tickets. 


.. _logfile: 

The ScummVM log file
=====================

To help you report a bug, you can find error messages in the ScummVM log file. The location of this file varies depending on your operating system. 


.. tabbed:: macOS

	.. panels::
		:column: col-lg-12 mb-2

		``~/Library/Logs/scummvm.log``

.. tabbed:: Linux

	.. panels::
		:column: col-lg-12 mb-2

		We use the XDG Base Directory Specification, so by default the file will be ``~/.cache/scummvm/logs/scummvm.log`` but its location might vary depending on the value of the ``XDG_CACHE_HOME`` environment variable. 

.. tabbed:: Windows

	.. panels::
		:column: col-lg-12 mb-2

	        Window Vista/7/8/10
		^^^^^^^^^^^^^^^^^^^^^^
		
                ``%APPDATA%\ScummVM\scummvm.log``

            
