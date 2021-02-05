def import_extension(loader, name, file, modulename):
    import imp
    import os
    import os.path
    root = os.path.dirname(file)

    mf = None
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
        # keep open tmp file inside module
        if mf: module.__mf = mf;
    finally:
        if spec[0] is not None:
            spec[0].close()

    module.__file__ = os.path.dirname(file)
    return module


def file_from_archive(loader, name, file, filename, closeOnExec=True):
    import os
    import os.path
    root = os.path.dirname(file)

    if hasattr(loader, "archive"):
        archive = os.path.join(loader.prefix, loader.archive)
        rp = os.path.relpath(root, archive)
        if rp != ".":
            # some subdir
            zip_filename = os.path.join(rp, filename)
        else:
            # root itself
            zip_filename = filename

        import zipfile
        z = zipfile.ZipFile(archive)

        from melown import utility
        flags = 0
        if closeOnExec:
            flags |= utility.MemoryFileFlag.closeOnExec
        mf = utility.memoryFile(name, flags)

        buf = z.open(zip_filename).read();
        os.write(mf.fileno(), buf)
        return mf.path(), mf

    return os.path.join(root, filename), None
