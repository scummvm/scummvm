-- This is a custom writer for pandoc.  It produces output suitable
-- for use in a StrongHelp manual. Not all functionalilty is currently
-- implemented.
--
-- Invoke with: pandoc -t stronghelp.lua
--
-- Note:  you need not have lua installed on your system to use this
-- custom writer.  However, if you do have lua installed, you can
-- use it to test changes to the script.  'lua stronghelp.lua' will
-- produce informative error messages if your code contains
-- syntax errors.

-- Character escaping
local function escape(s, in_attribute)
  return s:gsub("[<>$\\]",
    function(x)
      if x == '<' then
        return '\\<'
      elseif x == '>' then
        return '\\>'
      elseif x == '$' then
        return '\\$'
      elseif x == '\\' then
        return '\\\\'
      else
        return x
      end
    end)
end

local function convertPath(s)
  local path = s:gsub("[./]",
    function(x)
      if x == '.' then
        return '/'
      elseif x == '/' then
        return '.'
      else
        return x
      end
    end)
  path = string.gsub(path, "/md", "")
  return path;
end

-- Helper function to convert an attributes table into
-- a string that can be put into HTML tags.
local function attributes(attr)
  local attr_table = {}
  for x,y in pairs(attr) do
    if y and y ~= "" then
      table.insert(attr_table, ' ' .. x .. '="' .. escape(y,true) .. '"')
    end
  end
  return table.concat(attr_table)
end

-- Helper function to look up an attribute by name from
--  an attributes table.
-- TODO: Is this necessary?
local function getattribute(attr, name)
  local attr_table = {}
  for x,y in pairs(attr) do
    if x == name then
      do return escape(y,true) end
    end
  end
  return ""
end

-- Table to store footnotes, so they can be included at the end.
local notes = {}

-- Blocksep is used to separate block elements.
function Blocksep()
  return "\n\n"
end

-- This function is called once for the whole document. Parameters:
-- body is a string, metadata is a table, variables is a table.
-- This gives you a fragment.  You could use the metadata table to
-- fill variables in a custom lua template.  Or, pass `--template=...`
-- to pandoc, and pandoc will add do the template processing as
-- usual.
function Doc(body, metadata, variables)
  local buffer = {}
  local function add(s)
    table.insert(buffer, s)
  end
  add(body)
  if #notes > 0 then
    for _,note in pairs(notes) do
      add(note)
    end
  end
  return "#Indent +4\n" .. table.concat(buffer, "") .. "\n#Indent\n"
end

-- The functions that follow render corresponding pandoc elements.
-- s is always a string, attr is always a table of attributes, and
-- items is always an array of strings (the items in a list).
-- Comments indicate the types of other variables.

function Str(s)
  return escape(s)
end

function Space()
  return " "
end

function SoftBreak()
  return " "
end

function LineBreak()
  return "\n"
end

function Emph(s)
  return "{/}" .. s .. "{/}"
end

function Strong(s)
  return "{*}" .. s .. "{*}"
end

function Subscript(s)
  -- TODO
  return s
end

function Superscript(s)
  -- TODO
  return s
end

function SmallCaps(s)
  -- TODO
  return s
end

function Strikeout(s)
  -- TODO
  return s
end

function Link(s, src, tit, attr)
  -- escape(tit,true)
  if string.find(src, "://") or string.find(src, "mailto:") then
    return "<" .. s .. "=>#URL " .. escape(src,true) .. ">"
  elseif string.sub(src, 1, 1) == '#' then
    return "<" .. s .. "=>#TAG " .. escape(string.sub(src,2),true) .. ">"
  else
    return "<" .. s .. "=>" .. convertPath(escape(src,true)) .. ">"
  end
end

function Image(s, src, tit, attr)
  -- TODO
  return s
end

function Code(s, attr)
  return "{FCode}" .. escape(s) .. "{F}"
end

function InlineMath(s)
  return "\\(" .. escape(s) .. "\\)"
end

function DisplayMath(s)
  return "\\[" .. escape(s) .. "\\]"
end

function SingleQuoted(s)
  return "'" .. s .. "'"
end

function DoubleQuoted(s)
  return '"' .. s .. '"'
end


function Note(s)
  local num = #notes + 1
  -- add a list item with the note to the note table.
  table.insert(notes, '#Tag fn' .. num .. '\n' .. num .. ")\t" .. s .. '\n')
  -- return the footnote reference, linked to the note.
  return '<[' .. num .. ']=>#TAG fn' .. num .. '>'
end

function Span(s, attr)
  -- TODO
  return s
end

function RawInline(format, str)
  -- TODO
  return ''
end

function Cite(s, cs)
  -- TODO
  return s
end

function Plain(s)
  return s
end

function Para(s)
  return s
end

-- lev is an integer, the header level.
function Header(lev, s, attr)
  return  "#Tag " .. getattribute(attr, "id") .. "\n" .. "#fH" .. lev .. "\n" .. s .. "\n#f\n"
end

function BlockQuote(s)
  return "#Indent +6\n" .. s .. "\n#Indent\n"
end

function HorizontalRule()
  return "#Line\n"
end

function LineBlock(ls)
  -- TODO
  return table.concat(ls, '\n')
end

function CodeBlock(s, attr)
  -- TODO: attr.class and string.match(' ' .. attr.class .. ' ',' dot ')
  return "#fCode\n" .. escape(s) .. "\n#f"
end

function BulletList(items)
  local buffer = {}
  for _, item in pairs(items) do
    table.insert(buffer, " •\t" .. string.gsub(string.gsub(item, "\n", "\n\t"), "\n\t#", "\n#") .. "\n")
  end
  return "#Indent +4\n" .. table.concat(buffer, "") .. "#Indent\n"
end

function OrderedList(items)
  local buffer = {}
  for _, item in pairs(items) do
    table.insert(buffer,  _ .. ")\t"  .. string.gsub(string.gsub(item, "\n", "\n\t"), "\n\t#", "\n#") .. "\n")
  end
  return "#Indent +4\n" .. table.concat(buffer, "") .. "#Indent\n"
end

function DefinitionList(items)
  local buffer = {}
  for _,item in pairs(items) do
    local k, v = next(item)
    table.insert(buffer, k .. "\t" .. table.concat(v,"\n\t") .. "\n")
  end
  return "#Indent +4\n" .. table.concat(buffer, "") .. "#Indent\n"
end

-- Convert pandoc alignment to something StrongHelp can use.
-- align is AlignLeft, AlignRight, AlignCenter, or AlignDefault.
function stronghelp_align(align)
  if align == 'AlignLeft' then
    return 'Left'
  elseif align == 'AlignRight' then
    return 'Right'
  elseif align == 'AlignCenter' then
    return 'Centre'
  else
    return 'Left'
  end
end

function CaptionedImage(src, tit, caption, attr)
   -- TODO
   return tit .. '\n' .. caption .. '\n'
end

-- Caption is a string, aligns is an array of strings,
-- widths is an array of floats, headers is an array of
-- strings, rows is an array of arrays of strings.
function Table(caption, aligns, widths, headers, rows)
  local buffer = {}
  local function add(s)
    table.insert(buffer, s)
  end
  add("#Tab\n")
  if caption ~= "" then
    add(caption .. "\n")
  end
  local header_row = {}
  local empty_header = true
  for i, h in pairs(headers) do
    table.insert(header_row,h .. '\t')
    empty_header = empty_header and h == ""
  end
  if empty_header then
    head = ""
  else
    local cells = {}
    for _,h in pairs(header_row) do
      table.insert(cells, h)
    end
    add('{*}' .. table.concat(cells,'\t') .. '{*}\n')
  end
  for _, row in pairs(rows) do
    local cells = {}
    for i,c in pairs(row) do
      table.insert(cells, c)
    end
    add(table.concat(cells,'\t') .. '\n')
  end
  add("#Tab\n")
  return table.concat(buffer,'')
end

function RawBlock(format, str)
  -- TODO
  return ''
end

function Div(s, attr)
  -- TODO
  return s
end

-- The following code will produce runtime warnings when you haven't defined
-- all of the functions you need for the custom writer, so it's useful
-- to include when you're working on a writer.
local meta = {}
meta.__index =
  function(_, key)
    io.stderr:write(string.format("WARNING: Undefined function '%s'\n",key))
    return function() return "" end
  end
setmetatable(_G, meta)

