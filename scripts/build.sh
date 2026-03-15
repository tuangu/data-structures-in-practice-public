#!/usr/bin/env bash
# Build PDF and/or HTML outputs for both English and Chinese versions of the book.
# Usage:
#   ./scripts/build.sh [en|zh-TW|all]            (defaults to all, builds PDF + HTML)
#   ./scripts/build.sh [en|zh-TW|all] --html-only (builds HTML only, no LaTeX needed)
#
# Outputs written to dist/
#   dist/data-structures-in-practice-en.pdf
#   dist/data-structures-in-practice-en.html
#   dist/data-structures-in-practice-zh-TW.pdf
#   dist/data-structures-in-practice-zh-TW.html

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
DIST_DIR="${REPO_ROOT}/dist"

TARGET="${1:-all}"
HTML_ONLY=false
if [[ "${2:-}" == "--html-only" ]]; then
  HTML_ONLY=true
fi

mkdir -p "${DIST_DIR}"

# Ordered list of files (relative to a manuscript root) to feed to pandoc.
ORDERED_FILES=(
  "front_matter/00_cover.md"
  "front_matter/01_copyright.md"
  "front_matter/02_preface.md"
  "front_matter/03_table_of_contents.md"
  "chapters/chapter01.md"
  "chapters/chapter02.md"
  "chapters/chapter03.md"
  "chapters/chapter04.md"
  "chapters/chapter05.md"
  "chapters/chapter06.md"
  "chapters/chapter07.md"
  "chapters/chapter08.md"
  "chapters/chapter09.md"
  "chapters/chapter10.md"
  "chapters/chapter11.md"
  "chapters/chapter12.md"
  "chapters/chapter13.md"
  "chapters/chapter14.md"
  "chapters/chapter15.md"
  "chapters/chapter16.md"
  "chapters/chapter17.md"
  "chapters/chapter18.md"
  "chapters/chapter19.md"
  "chapters/chapter20.md"
  "appendices/appendix_a.md"
  "appendices/appendix_b.md"
  "appendices/appendix_c.md"
  "appendices/appendix_d.md"
  "appendices/appendix_e.md"
  "appendices/appendix_f.md"
  "back_matter/about_author.md"
  "back_matter/bibliography.md"
)

build_html() {
  local manuscript_dir="$1"
  local output_file="$2"
  local lang="$3"

  echo "Building HTML: ${output_file}"

  local input_files=()
  for f in "${ORDERED_FILES[@]}"; do
    local path="${manuscript_dir}/${f}"
    if [[ -f "${path}" ]]; then
      input_files+=("${path}")
    fi
  done

  pandoc "${input_files[@]}" \
    --from markdown \
    --to html5 \
    --standalone \
    --toc \
    --toc-depth=2 \
    --metadata lang="${lang}" \
    --css "https://cdn.jsdelivr.net/npm/github-markdown-css/github-markdown.min.css" \
    --output "${output_file}"

  echo "  -> ${output_file}"
}

build_pdf() {
  local manuscript_dir="$1"
  local output_file="$2"
  local lang="$3"

  echo "Building PDF: ${output_file}"

  # Choose a CJK-compatible font when building the Chinese version.
  local extra_args=()
  if [[ "${lang}" == "zh-TW" ]]; then
    extra_args+=(
      "--variable" "CJKmainfont=Noto Sans CJK TC"
      "--variable" "CJKoptions=Scale=1.0"
    )
  fi

  local input_files=()
  for f in "${ORDERED_FILES[@]}"; do
    local path="${manuscript_dir}/${f}"
    if [[ -f "${path}" ]]; then
      input_files+=("${path}")
    fi
  done

  pandoc "${input_files[@]}" \
    --from markdown \
    --to pdf \
    --pdf-engine=xelatex \
    --toc \
    --toc-depth=2 \
    --metadata lang="${lang}" \
    --variable geometry:margin=1in \
    --variable fontsize=11pt \
    "${extra_args[@]}" \
    --output "${output_file}"

  echo "  -> ${output_file}"
}

build_en() {
  local manuscript_dir="${REPO_ROOT}/manuscript"
  if [[ "${HTML_ONLY}" == false ]]; then
    build_pdf  "${manuscript_dir}" "${DIST_DIR}/data-structures-in-practice-en.pdf"  "en"
  fi
  build_html "${manuscript_dir}" "${DIST_DIR}/data-structures-in-practice-en.html" "en"
}

build_zh_TW() {
  local manuscript_dir="${REPO_ROOT}/manuscript-zh-TW"
  if [[ "${HTML_ONLY}" == false ]]; then
    build_pdf  "${manuscript_dir}" "${DIST_DIR}/data-structures-in-practice-zh-TW.pdf"  "zh-TW"
  fi
  build_html "${manuscript_dir}" "${DIST_DIR}/data-structures-in-practice-zh-TW.html" "zh-TW"
}

case "${TARGET}" in
  en)    build_en ;;
  zh-TW) build_zh_TW ;;
  all)   build_en; build_zh_TW ;;
  *)
    echo "Unknown target '${TARGET}'. Use: en | zh-TW | all"
    exit 1
    ;;
esac

echo "Done. Artifacts in ${DIST_DIR}/"
