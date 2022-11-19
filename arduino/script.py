Import("env")
import os
import shutil

def post_program_action(source, target, env):

    targetfile = os.path.abspath(target[0].get_abspath())
    filename = os.path.basename(targetfile)
    startpath = os.path.dirname(targetfile)
    basedir = os.path.basename(startpath)
    destpath = os.path.normpath(os.path.join(startpath, '../../../../build', basedir))

    print("\nCopying " + filename + " file to the build directory...\n")
    print("Target file: " + targetfile)
    print("Destination directory: " + destpath)

    # create directories if they don't exist
    if not os.path.exists(destpath):
        os.makedirs(destpath)

    # copy the target file to the destination, if it exist
    if os.path.exists(targetfile):
        shutil.copy(targetfile, destpath)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", post_program_action)