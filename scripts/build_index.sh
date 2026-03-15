#!/usr/bin/env bash
# Generate dist/index.html — the landing page for the GitHub Pages site.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DIST_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)/dist"

mkdir -p "${DIST_DIR}"

cat > "${DIST_DIR}/index.html" <<'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Data Structures in Practice</title>
  <style>
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      background: #0d1117;
      color: #e6edf3;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      padding: 2rem;
    }

    .card {
      background: #161b22;
      border: 1px solid #30363d;
      border-radius: 12px;
      max-width: 560px;
      width: 100%;
      padding: 2.5rem 2rem;
      text-align: center;
    }

    h1 {
      font-size: 1.6rem;
      font-weight: 700;
      margin-bottom: 0.4rem;
      color: #f0f6fc;
    }

    .subtitle {
      font-size: 0.95rem;
      color: #8b949e;
      margin-bottom: 0.3rem;
    }

    .meta {
      font-size: 0.85rem;
      color: #6e7681;
      margin-bottom: 2rem;
    }

    .buttons {
      display: flex;
      gap: 1rem;
      justify-content: center;
      flex-wrap: wrap;
    }

    .btn {
      display: inline-flex;
      align-items: center;
      gap: 0.4rem;
      padding: 0.65rem 1.4rem;
      border-radius: 6px;
      font-size: 0.95rem;
      font-weight: 600;
      text-decoration: none;
      transition: opacity 0.15s;
    }
    .btn:hover { opacity: 0.85; }

    .btn-en {
      background: #238636;
      color: #ffffff;
    }

    .btn-zh {
      background: #1f6feb;
      color: #ffffff;
    }

    footer {
      margin-top: 2rem;
      font-size: 0.8rem;
      color: #6e7681;
    }
    footer a { color: #58a6ff; text-decoration: none; }
    footer a:hover { text-decoration: underline; }
  </style>
</head>
<body>
  <div class="card">
    <h1>Data Structures in Practice</h1>
    <p class="subtitle">A Hardware-Aware Approach for System Software Engineers</p>
    <p class="meta">Author: Danny Jiang &nbsp;·&nbsp; License: CC BY 4.0</p>

    <div class="buttons">
      <a class="btn btn-en" href="en.html">
        📖 Read in English
      </a>
      <a class="btn btn-zh" href="zh-TW.html">
        📖 繁體中文閱讀
      </a>
    </div>
  </div>

  <footer>
    <p>
      Source available on
      <a href="https://github.com/tuangu/data-structures-in-practice-public" target="_blank" rel="noopener">
        GitHub
      </a>
    </p>
  </footer>
</body>
</html>
EOF

echo "Generated ${DIST_DIR}/index.html"
