from subprocess import run
import os
from os import makedirs, access
from os.path import exists, dirname, join, abspath, splitext, isfile
from shutil import copy, rmtree
from invoke import task
import requests

# from compile_unit import wasm_cmake, wasm_copy_upload

PROJ_ROOT = dirname(dirname(abspath(__file__)))

FUNC_DIR = join(PROJ_ROOT, "Function")
FUNC_BUILD_DIR = join(FUNC_DIR, "build")


def func_cmake(src_dir, build_dir, target, clean=False, debug=False):
    cmake_build_type = "Debug" if debug else "Release"

    if exists(build_dir) and clean:
        rmtree(build_dir)

    makedirs(build_dir, exist_ok=True)

    build_cmd = [
        "cmake",
        "-GNinja",
        "-DCMAKE_BUILD_TYPE={}".format(cmake_build_type),
        src_dir,
    ]

    build_cmd = " ".join(build_cmd)
    print(build_cmd)

    res = run(build_cmd, shell=True, cwd=build_dir)
    if res.returncode != 0:
        raise RuntimeError("Failed on cmake for {}".format(target))

    cmd = "ninja -vv" if debug else "ninja"
    cmd = "{} {}".format(cmd, target) if target else cmd
    res = run(cmd, shell=True, cwd=build_dir)

    if res.returncode != 0:
        raise RuntimeError("failed on make for {}".format(target))


@task(name="compile")
def compile(ctx, func, clean=False, debug=False):
    """
    Compile a function
    """
    # Build the function (gets written to the build dir)
    func_cmake(FUNC_DIR, FUNC_BUILD_DIR, func, clean, debug)


# /register/type/user/functionName/concurrency/core/mem
@task(default=True)
def register(ctx, func, concurrency, core=0, mem=0):
    """
    Register function
    """
    func_file = join(FUNC_BUILD_DIR, "lib{}.so".format(func))
    if not exists(func_file):
        print("{} is not exist".format(func_file))
        return
    if not isfile(func_file):
        print("{} is not file".format(func_file))
        return
    if not access(func_file, os.R_OK):
        print("{} is nor readable".format(func_file))
        return
    if func == "hello1":
        core = 0.5
        mem = 128
    url = "http://localhost:8080/register/native/kingdo/{}/{}/{}/{}".format(func, concurrency, core, mem)
    response = requests.put(url, data=open(func_file, "rb"))
    print("Response {}: {}".format(response.status_code, response.text))


@task()
def invoke(ctx, func):
    url = "http://localhost:8080/invoke/{}".format(func)
    data = {
        "name": "Kingdo"
    }
    response = requests.post(url, json=data)
    print("Response {}:\n{}".format(response.status_code, response.text))


@task()
def hey(ctx, func, n, c):
    url = "http://localhost:8080/invoke/kingdo/{}".format(func)
    cmd = "hey -n {} -c {} -m POST -t 0 -d ".format(n, c) + "\"{\"name\": \"Kingdo\"}\" " + url
    print(cmd)
    res = run(cmd, shell=True)


@task()
def getRFT(ctx):
    url = "http://localhost:8080/rft/info"
    response = requests.get(url)
    print("Response Code:{}".format(response.status_code))
    print(response.text)
