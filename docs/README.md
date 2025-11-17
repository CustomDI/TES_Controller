# TES Controller Documentation

This directory contains the complete Sphinx documentation for the TES Controller project.

## 📖 Documentation Files

### Sphinx Documentation (`.rst` files)
- **index.rst** - Main landing page
- **installation.rst** - Installation guide for macOS/Linux/Windows
- **getting-started.rst** - Comprehensive tutorial and usage patterns
- **examples.rst** - 10+ complete working examples
- **api.rst** - Auto-generated API reference

### Helper Documents (`.md` files)
- **README.md** - This file - documentation overview
- **DOCUMENTATION_SETUP.md** - Complete setup and deployment guide
- **QUICK_REFERENCE.md** - One-page API cheat sheet

### Configuration Files
- **conf.py** - Sphinx configuration
- **requirements.txt** - Documentation build dependencies
- **Makefile** - Build commands for Unix/macOS
- **make.bat** - Build commands for Windows

### Static Assets
- **_static/** - Custom CSS and images (committed to git)
- **_build/** - Generated documentation (gitignored)

## 🚀 Quick Start

### Build HTML Documentation Locally

```bash
cd docs
make html
```

Then open `_build/html/index.html` in your browser:

```bash
# macOS
open _build/html/index.html

# Linux
xdg-open _build/html/index.html

# Windows
start _build/html/index.html
```

### Clean Build

To clean and rebuild:

```bash
make clean
make html
```

## Documentation Structure

- `index.rst` - Main landing page
- `installation.rst` - Installation instructions
- `getting-started.rst` - Quick start guide and basic usage
- `examples.rst` - Complete working examples
- `api.rst` - Auto-generated API reference
- `conf.py` - Sphinx configuration

## Deploying to ReadTheDocs

### One-Time Setup

1. **Push your code to GitHub** (if not already done)

2. **Sign up for ReadTheDocs**:
   - Go to https://readthedocs.org/
   - Click "Sign up" and use your GitHub account
   - Authorize ReadTheDocs to access your repositories

3. **Import your project**:
   - Click "Import a Project"
   - Select your `TES_Controller` repository
   - Click "Next"
   - ReadTheDocs will auto-detect the `.readthedocs.yaml` config
   - Click "Build"

4. **Configure (if needed)**:
   - Go to project settings
   - Ensure Python version is set to 3.9+
   - The `.readthedocs.yaml` file handles the rest

### Automatic Updates

Once configured, ReadTheDocs automatically rebuilds your docs on every git push!

Your documentation will be available at:
```
https://tes-controller.readthedocs.io/
```

## Local Development Workflow

### Watch for Changes (recommended)

Install `sphinx-autobuild`:

```bash
pip install sphinx-autobuild
```

Then run:

```bash
sphinx-autobuild . _build/html
```

This starts a local server at http://127.0.0.1:8000 that auto-rebuilds on file changes.

### Manual Build

```bash
make html
```

### Build Other Formats

```bash
make latexpdf  # PDF via LaTeX
make epub      # ePub format
make help      # See all options
```

## Updating Documentation

### Adding a New Page

1. Create a new `.rst` file (e.g., `advanced-usage.rst`)
2. Add it to the `toctree` in `index.rst`:

```rst
.. toctree::
   :maxdepth: 2
   :caption: Contents:

   installation
   getting-started
   examples
   advanced-usage   # <-- Add your new page here
   api
```

3. Rebuild: `make html`

### Updating API Documentation

The API documentation is auto-generated from docstrings. Just update your Python code's docstrings and rebuild:

```bash
make clean
make html
```

### Adding Images

1. Save images to `_static/` directory
2. Reference in `.rst` files:

```rst
.. image:: _static/diagram.png
   :width: 600px
   :alt: System diagram
```

## Tips

- **Use RST syntax checker**: Install `rst-lint` for syntax checking
- **Preview locally**: Always build locally before pushing
- **Check warnings**: Fix Sphinx warnings - they indicate problems
- **Keep examples updated**: Update `examples.rst` when API changes
- **Version your docs**: Tag releases and ReadTheDocs can host multiple versions

## Troubleshooting

### Module not found errors

Make sure the path to your Python package is correct in `conf.py`:

```python
sys.path.insert(0, os.path.abspath('../python'))
```

### Build fails on ReadTheDocs

- Check the build log at https://readthedocs.org/projects/tes-controller/builds/
- Ensure all dependencies are in `docs/requirements.txt`
- Verify `.readthedocs.yaml` is correct

### Autodoc not working

Ensure your modules are importable:

```bash
cd docs
python -c "import tes_controller; print(tes_controller.__file__)"
```

## Resources

- [Sphinx Documentation](https://www.sphinx-doc.org/)
- [ReadTheDocs Guide](https://docs.readthedocs.io/)
- [reStructuredText Primer](https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html)
- [ReadTheDocs Theme](https://sphinx-rtd-theme.readthedocs.io/)
