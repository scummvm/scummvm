
local lang
local lang_en

local game_type = config_get_game_type()
local lang_selected = config_get_language()

function lang_init(lang_type)
   if lang_type ~= "intro" then
      lang_type = "game"
   end

   lang_en = nuvie_load(string.lower(game_type).."/lang/en/"..lang_type..".lua")

   if lang_en == nil then
      lang_en = {}
   else
      lang_en = lang_en()
   end

   if lang_selected ~= "en" then
       lang = nuvie_load(string.lower(game_type).."/lang/"..lang_selected.."/"..lang_type..".lua")
       if lang == nil then
         lang = lang_en
       else
         lang = lang()
       end
   else
      lang = lang_en
   end
end

function i18n(code)
   local str = lang[code]
   if str == nil then
      str = lang_en[code]
   end
   if str == nil then
      str = code
   end
   return str;
end

function i18nf(code, ...)
   return string.format(i18n(code), ...)
end

function printnl(code)
   print("\n"..i18n(code))
end

function printl(code)
   print(i18n(code))
end

function printfl(code, ...)
   print(i18nf(code, ...))
end
