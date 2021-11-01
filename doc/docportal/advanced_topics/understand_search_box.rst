======================================================
Understanding the search box
======================================================

This guide expands on the information contained on the :ref:`The search box <search_box>` section of the Launcher interface description.


Main functionality
===========================================

The filter is applied as you type, there is no need to press Enter or click anything to perform the search.

All searches are case insensitive.

To reset the filter and get the full list of games, you could click on a cross icon next to the text input field.


Search patterns
===========================================

Whitespace breaks the search input into separate tokens, search patterns.

Simple patterns
___________________________________________

If search pattern does not contain ``:``/``=``/``~`` characters and it does not start with ``!`` character, then it's used for a case insensitive substring search.

For example, if you type ``m`` you would get all the games containing an 'M' or 'm' in description.

If you type ``monkey``, you would get games like "The Curse of Monkey Island (Demo/Windows)", "Infinite Monkeys", "Three Monkeys, One Cage", etc.

This is what you need in most simple cases, but is not enough to search for more complex things.

Let's search the configuration key's values
___________________________________________

Game properties listed at :doc:`configuration_file` can be checked with the filter as well.

Advanced patterns
*******************************************

There are currently 3 types of patterns, that can be used for accessing configuration file:

.. csv-table::
  	:header-rows: 1
	:class: config

		Type,Description,Example,Example result
		"<key>=<value>", check exact value of ``<key>``, ``gameid=reversion2``, games with exact gameid "reversion2"
		"<key>:<value>", search substring at ``<key>``, ``description::``, games with ":" substring at description
		"<key>~<value>", match wildcard against ``<key>``, ``path~D:\\games\\*``, games located at Windows folder D:\\Games\\

Available configuration keys
*******************************************

.. note::

	The ``\`` character used at Windows paths should be escaped as ``\\`` when using wildcards.

You can use any :ref:`configuration_keys` as a ``<key>`` part of the search pattern.

Here are some more examples:

- ``show_fps=true`` - games with "Show FPS" option set to true
- ``extra~?*`` - games with non-empty extra part of the description
- ``guioptions:noLang`` - games without displayed language
- ``keymap_engine-default_LCLK:MOUSE_RIGHT`` - games with right mouse button remapped to left mouse button

Abbrivating configuration keys
*******************************************

Some common used configuration keys can be abbrivated, just type any prefix instead of full strings for those 6 keys:

.. csv-table::
  	:header-rows: 1
	:class: config

		Full key,Description,Examples
		``description``, game description as displayed at the game list,"

	- ``d:monkey``
	- ``desc:monkey``
	- ``descr:monkey``
	- ``description:monkey``"
		``engineid``, internal ID of the game engine,"

	- ``e:ags``
	- ``engine:monkey``
	- ``engineed:monkey``"
		``gameid``, internal ID of the game,"

	- ``g:monkey``
	- ``game:monkey``
	- ``gameid:monkey``"
		``language``, internal ID of the language (usually 2 letter code like "de"/"en"/"fr"/"jp"/etc),"

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

	The ``platform`` key can't be abbrivated to ``p``, since ``p`` is already used for ``path``.


Inverting any search pattern
___________________________________________

Prefixing search pattern with `!` character inverts the result:

- ``!GOG`` - games that don't contain "GOG" substring in description
- ``!lang=ru`` - games not in Russian languange
- ``!p:demo`` - games that don't have "demo" substring in game path
- ``!engine~sword#`` - games not made with "sword1" and "sword2" engines (but "sword25" is fine)


How do the search patterns work together?
===========================================

If you have provided several search patterns, only games that match all of them are displayed.

The matches are independent and not ordered, which means that when you are looking for ``Open Quest``, you would get all the games with words "Open" and "Quest" in description. The results would contain games like "Open Quest (Windows/English)" and "Police Quest IV: Open Season (DOS/Demo)".

Here are some more examples of complex requests:

- ``engine=ags path:steamapps !extra:Steam`` - AGS games at your /SteamApps/ folder, but not marked as Steam game at "extra"
- ``e=wintermute l=`` - Wintermute games with empty "language" property
- ``pl:dos lang=he desc~a*`` - Hebrew games for DOS with description starting with letter "A"
