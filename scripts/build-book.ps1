param(
  [string]$SourcePath = "src/book/chapter-01.md",
  [string]$TemplatePath = "src/templates/page.html",
  [string]$StylesPath = "src/styles/book.css",
  [string]$OutputPath = "docs/index.html"
)

python scripts/build-book.py $SourcePath $TemplatePath $StylesPath $OutputPath
