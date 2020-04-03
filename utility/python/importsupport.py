def import_extension(loader, name, file, modulename):
    import imp
    import os
    import os.path
    root = os.path.dirname(file)

    if hasattr(loader, "archive"):
        archive = os.path.join(loader.prefix, loader.archive)
        zip_filename = os.path.join(os.path.relpath(root, archive)
                                    , modulename)

        import zipfile
        z = zipfile.ZipFile(archive)

        from melown import utility
        mf = utility.memoryFile(name, utility.MemoryFileFlag.closeOnExec)

        buf = z.open(zip_filename).read();
        os.write(mf.fileno(), buf)
        spec = (None, mf.path(), (".so", "rb", imp.C_EXTENSION))
    else:
        spec = (None, os.path.join(root, modulename)
                    , (".so", "rb", imp.C_EXTENSION))

    try:
        module = imp.load_module(name, *spec)
    finally:
        if spec[0] is not None:
            spec[0].close()

    module.__file__ = os.path.dirname(file)
    return module
