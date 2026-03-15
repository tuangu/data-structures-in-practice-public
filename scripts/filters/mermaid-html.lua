-- mermaid-html.lua
-- Pandoc Lua filter: converts ```mermaid code blocks into
-- <div class="mermaid"> elements so mermaid.js can render them.

function CodeBlock(el)
  if el.classes[1] == "mermaid" then
    return pandoc.RawBlock("html", '<div class="mermaid">\n' .. el.text .. '\n</div>')
  end
end
