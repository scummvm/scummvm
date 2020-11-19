
=======================
Report a bug
=======================

To report a bug, go to the ScummVM `Issue Tracker <https://bugs.scummvm.org/>`_ and log in with your GitHub account. 


.. _logfile: 

The ScummVM log file
=====================

To help you report a bug, you can find error messages in the ScummVM log file. The location of this file varies depending on your operating system. 


.. tabbed:: macOS

	.. panels::
		:column: col-lg-12 mb-2

		``~/Library/Logs/scummvm.log``, where ``~`` is your Home directory.

.. tabbed:: Linux

	.. panels::
		:column: col-lg-12 mb-2

		We use the XDG Base Directory Specification, so by default the file will be ``~/.cache/scummvm/logs/scummvm.log`` but its location might vary depending on the value of the ``XDG_CACHE_HOME`` environment variable. 

.. tabbed:: Windows

	.. panels::
		:column: col-lg-12 mb-2

	        Window Vista/7/8/10
		^^^^^^^^^^^^^^^^^^^^^^
		
                ``\Users\username\AppData\Roaming\ScummVM\scummvm.log``

                .. tip::

			The AppData folder is hidden. The steps to view hidden files and folders depends on the Windows version. 

			For more information, see the `Windows support page <https://support.microsoft.com/en-us/help/14201/windows-show-hidden-files>`_.
	