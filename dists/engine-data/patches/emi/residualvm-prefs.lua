-- ResidualVM - A 3D game interpreter
--
-- ResidualVM is the legal property of its developers, whose names
-- are too numerous to list here. Please refer to the COPYRIGHT
-- file distributed with this source distribution.
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; either version 2
-- of the License, or (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.


--Setter and getter values for cfg
function emi_cfg_settable_event(table, index, value)
	PrintDebug("Set cfg value " .. index .. " to " .. value)
	SetResidualVMPreference(tostring(index), value)
end

function emi_cfg_gettable_event(table, index)
	PrintDebug("Get cfg value " .. index)
	value = GetResidualVMPreference(tostring(index))

	--use default value if not found in config
	if value == nil then
		value = system_prefs.defaults_cfg[index]
	end
	return value
end

--Overwrite stock loadprefs function
system_prefs.loadprefs = function(table, filename)
	--avoid installing tag methods repeatedly when starting a new game
	if system_prefs.defaults_cfg ~= nil then
		return
	end

	PrintDebug("Function system_prefs.loadprefs modified for ResidualVM loaded")

	--save old cfg table as default values
	system_prefs.defaults_cfg = system_prefs.cfg

	--set a new table for cfg and its tag methods to hook in the registry
	system_prefs.cfg = {}
	emi_cfg_tag = newtag()
	settagmethod(emi_cfg_tag, "gettable", emi_cfg_gettable_event)
	settagmethod(emi_cfg_tag, "settable", emi_cfg_settable_event)
	settag(system_prefs.cfg, emi_cfg_tag)
end

--delete saveprefs function
system_prefs.saveprefs = function(table, filename)
end
