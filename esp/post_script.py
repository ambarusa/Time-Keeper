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

    # check if the files to be merged exist - NOT USED 
    # os.chdir(destpath)
    # if os.path.exists("firmware.bin") and os.path.exists("littlefs.bin"):
    #     command = ['--chip', 'esp8266', 'merge_bin', '-o', 'combined.bin', '0x0', 'firmware.bin', '0x300000', 'littlefs.bin']
    #     print('Using esptool.py %s' % ' '.join(command))
    #     esptool.main(command)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", post_program_action)
env.AddPostAction("$BUILD_DIR/littlefs.bin", post_program_action)