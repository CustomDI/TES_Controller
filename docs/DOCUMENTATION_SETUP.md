# Documentation Setup Guide ✅

## Contents

- `docs/conf.py` – Sphinx configuration (Read the Docs theme, type hints)
- `docs/index.rst` – Landing page and table of contents
- `docs/installation.rst` – Platform-specific install notes
- `docs/getting-started.rst` – Core usage walkthrough
- `docs/usage.rst` – Quick recipes and configuration snippets
- `docs/examples.rst` – End-to-end scripts
- `docs/api.rst` – Auto-generated API reference
- `docs/Makefile` / `docs/make.bat` – Convenience build targets
- `docs/requirements.txt` – Documentation dependencies
- `docs/.nojekyll` – Allows nested assets for GitHub Pages
- `docs/README.md` – Build and deploy instructions

## Build Locally

```bash
pip install -r docs/requirements.txt
make -C docs html
open docs/_build/html/index.html  # macOS
```

For live reload while editing:

```bash
pip install sphinx-autobuild
sphinx-autobuild docs docs/_build/html
```

## Publish Options

### GitHub Pages

1. Enable Pages for the repository using the `docs/` folder as the source.
2. Copy the freshly built HTML artefacts into `docs/` (or automate via GitHub Actions).
3. Commit and push – GitHub Pages redeploys automatically.

### Read the Docs (optional)

1. Sign in at <https://readthedocs.org/> using GitHub.
2. Import the repository; the builder detects `docs/conf.py` automatically.
3. Configure Python 3.11 (or your preferred version) and trigger a build.

## Updating the Docs

- Update Python docstrings → rebuild to refresh `api.rst` output.
- Add a new page → create `docs/new-page.rst`, link it from `index.rst`, rebuild.
- Ship new examples → edit `docs/examples.rst` and ensure snippets still run.

## Troubleshooting

| Issue | Fix |
|-------|-----|
| `ModuleNotFoundError: tes_controller` | Confirm `sys.path.insert` call in `conf.py` and run from repo root |
| Sphinx build warnings | Run `make -C docs clean html` and inspect the warning list |
| GitHub Pages missing CSS/JS | Ensure `.nojekyll` exists in `docs/` and assets were copied |

---

Your documentation stack is ready – build, review, and publish with confidence! 🎉
