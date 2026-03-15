-- mermaid-pdf.lua
-- Pandoc Lua filter: converts ```mermaid code blocks into PNG images
-- for PDF output by shelling out to mmdc (mermaid-cli).
--
-- Requires mmdc on PATH:  npm install -g @mermaid-js/mermaid-cli
--
-- Optional environment variable:
--   MERMAID_PUPPETEER_CONFIG  path to a puppeteer JSON config
--                             (used in CI to pass --no-sandbox)

local count = 0
local tmp_prefix = os.tmpname()
local puppeteer_config = os.getenv("MERMAID_PUPPETEER_CONFIG") or ""

function CodeBlock(el)
  if el.classes[1] == "mermaid" then
    count = count + 1
    local mmd_file = tmp_prefix .. "_" .. count .. ".mmd"
    local png_file = tmp_prefix .. "_" .. count .. ".png"

    -- Write mermaid source to a temp file.
    local f = io.open(mmd_file, "w")
    if not f then
      io.stderr:write("mermaid-pdf: could not write temp file " .. mmd_file .. "\n")
      return el
    end
    f:write(el.text)
    f:close()

    -- Build the mmdc command.
    local cmd = "mmdc -i " .. mmd_file .. " -o " .. png_file .. " -b white"
    if puppeteer_config ~= "" then
      cmd = cmd .. " -p " .. puppeteer_config
    end
    cmd = cmd .. " 2>/dev/null"

    local ok = os.execute(cmd)
    if ok then
      -- Return an image element centred in the PDF.
      local img = pandoc.Image({pandoc.Str("Diagram")}, png_file)
      img.attributes["width"] = "100%"
      return pandoc.Para({img})
    end

    -- mmdc not available or failed – keep the code block so the build
    -- doesn't break, but warn.
    io.stderr:write("mermaid-pdf: mmdc failed for diagram " .. count
                    .. " (is @mermaid-js/mermaid-cli installed?)\n")
    return el
  end
end
