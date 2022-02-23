======================================================
Understanding the search box
======================================================

This guide expands on the information contained in the :ref:`the search box <search_box>` section of the Launcher interface description.


Main functionality
===========================================

- A filter is applied as you type, so there is no need to press Enter or click anything to perform the search.
- All searches are case insensitive.
- To reset the filter and get the full list of games, click on the cross icon next to the text input field.
- Whitespace breaks the search input into separate tokens or search patterns.


Search patterns
===========================================


Simple patterns
___________________________________________

The simplest way to search is to use the substring search functionality. For example, type ``m`` to get all the games containing an 'M' or 'm' in description, and type ``monkey`` to get games like "The Curse of Monkey Island (Demo/Windows)", "Infinite Monkeys" or "Three Monkeys, One Cage". Ensure the search pattern does not contain ``:``/``=``/``~`` characters and it does not start with the ``!`` character.


Searching configuration key values
___________________________________________


Advanced patterns
**************************

To search for game properties listed in the :doc:`configuration_file` there are 3 available patterns:

.. csv-table::
  	:header-rows: 1
	:class: config

		Type,Description,Example,Example result
		"<key>=<value>", check exact value of ``<key>``, ``gameid=reversion2``, games with exact gameid "reversion2"
		"<key>:<value>", search substring at ``<key>``, ``description::``, games with ":" substring at description
		"<key>~<value>", match wildcard against ``<key>``, ``path~D:\\games\\*``, games located at Windows folder D:\\Games\\


.. note::

	Escape the ``\`` character used in Windows paths by using ``\\`` when using wildcards.


Available configuration keys
*******************************************

You can use any :ref:`configuration key <configuration_keys>` as a ``<key>`` part of the search pattern.

Here are some more examples:

- ``show_fps=true`` - games with "Show FPS" option set to true
- ``extra~?*`` - games with non-empty extra part of the description
- ``guioptions:noLang`` - games without displayed language
- ``keymap_engine-default_LCLK:MOUSE_RIGHT`` - games with right mouse button remapped to left mouse button

Abbrevating configuration keys
*******************************************

To abbreviate the 6 most common configuration keys, just type any prefix instead of full strings for those keys:

.. csv-table::
  	:header-rows: 1
	:class: config

		Full key,Description,Examples
		``description``, game description as displayed on the game list,"

	- ``d:monkey``
	- ``desc:monkey``
	- ``descr:monkey``
	- ``description:monkey``"
		``engineid``, internal ID of the game engine,"

	- ``e:ags``
	- ``engine:monkey``
	- ``engineid:monkey``"
		``gameid``, internal ID of the game,"

	- ``g:monkey``
	- ``game:monkey``
	- ``gameid:monkey``"
		``language``, internal ID of the language (usually 2 letter code like "de"/"en"/"fr"/"jp","

	- ``l=en``
	- ``lang=en``
	- ``language=en``"
		``path``, Filesystem path for the game,"

	- ``p~D:*``
	- ``path~D:*``"
		``platform``, internal ID of the platform,"

	- ``pl~windows``
	- ``platform~windows``"

.. note::

	The ``platform`` key can't be abbrevated to ``p``, since ``p`` is already used for ``path``.


Inverting the search pattern
___________________________________________

To invert the search result, prefix the search pattern with the`!` character. For example: 

- ``!GOG`` - games that don't contain "GOG" substring in description
- ``!lang=ru`` - games not in Russian languange
- ``!p:demo`` - games that don't have "demo" substring in game path
- ``!engine~sword#`` - games not made with "sword1" and "sword2" engines (but "sword25" is fine)


How do the search patterns work together?
===========================================

If you have provided several search patterns, only games that match all of them are displayed.

The matches are independent and not ordered, which means that if you search for ``Open Quest``, you get all the games with words "Open" and "Quest" in description. The results would contain games like "Open Quest (Windows/English)" and "Police Quest IV: Open Season (DOS/Demo)".

Here are some more examples of complex requests:

- ``engine=ags path:steamapps !extra:Steam`` - AGS games at your /SteamApps/ folder, but not marked as Steam game at "extra"
- ``e=wintermute l=`` - Wintermute games with empty "language" property
- ``pl:dos lang=he desc~a*`` - Hebrew games for DOS with description starting with letter "A"
