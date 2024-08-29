#!/bin/env python3

import os, sys, shutil
import pathlib


def lipo_walker(arm64_basedir, x64_basedir, outputdir):
    for item in list(pathlib.Path(arm64_basedir).rglob("*")):
        fullpath = str(item)
        x64_fullpath = fullpath.replace(arm64_basedir, x64_basedir).replace("arm64", "x64")
        outputfile = fullpath.replace(arm64_basedir, outputdir).replace("arm64", "univeral")
        outdir = os.path.dirname(outputfile)
        if not os.path.exists(outdir):
            os.makedirs(outdir)
        
        if not os.path.exists(outputfile):
            if not os.path.islink(fullpath) and os.system("lipo -info %s > /dev/null 2>&1" % fullpath) == 0:
                if os.system("lipo -output %s -create %s %s" % (outputfile, fullpath, x64_fullpath)) == 0:
                    print ("Successfully created %s" % outputfile)
            else:
                if os.path.islink(fullpath):
                    linkto = os.readlink(fullpath)
                    
                    if linkto.startswith(arm64_basedir):
                        linkto = linkto.replace(arm64_basedir, outputdir)
                    elif linkto.startswith(x64_basedir):
                        linkto = linkto.replace(x64_basedir, outputdir)
                    
                    os.symlink(linkto, outputfile)
                
                elif not os.path.isdir(fullpath):
                    shutil.copy(fullpath, outputfile)
        
if __name__ == "__main__":
    if len(sys.argv) != 4:
        print (f"Usage: {sys.argv[0]} <arm64dir> <x64dir> <outputdir>")
        sys.exit(1)
        
    arm64_basedir = sys.argv[1]
    x64_basedir = sys.argv[2]
    outputdir = sys.argv[3]

    lipo_walker(arm64_basedir, x64_basedir, outputdir)
