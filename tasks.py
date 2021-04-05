import os
from os.path import join as j
from invoke import Collection, Config, task

DOCKER = "docker run --rm -v $(pwd):/home/developer grifcj/gnurm"

@task
def clean(ctx):
    ctx.run(f"{DOCKER} make clean")

@task
def build(ctx):
    ctx.run(f"{DOCKER} make build")

@task
def buildbash(ctx):
    ctx.run(command=f"docker run -it --rm -v $(pwd):/home/developer grifcj/gnurm bash",
            pty=True)

# run monodevelop and debug peripheral model with these binaries
@task
def monobash(ctx):
    path_to_renode = os.path.expanduser(j("~", "src", "renode"))
    path_to_embed = os.path.expanduser(j("~", "src", "stm32-adc"))
    ctx.run(
            command=(
                f"docker run -it --rm"
                f" -v {path_to_renode}:/home/developer/renode"
                f" -v {path_to_embed}:/home/developer/embed"
                f" -w /home/developer/renode"
                f" grifcj/monodevelop bash"),
            pty=True)

# run monodevelop and debug peripheral model with these binaries
@task
def runadc(ctx):
    path_to_renode = os.path.expanduser(j("~", "src", "renode"))
    path_to_embed = os.path.expanduser(j("~", "src", "stm32-adc"))
    home = '/home/developer'
    ctx.run(
            command=(
                f"docker run -it --rm"
                f" -v {path_to_renode}:{home}/renode"
                f" -v {path_to_embed}:{home}/embed"
                f" -w /home/developer/renode"
                f" -e DISPLAY=host.docker.internal:0"
                f" grifcj/monodevelop"
                f" mono output/bin/Debug/Renode.exe"
                f" -e 'path add @{home}/embed'"
                f" -e 'include @{home}/embed/test.resc'"
                ),
            pty=True)

@task
def monodevelop(ctx, shell=False):
    path_to_renode = os.path.expanduser(j("~", "src", "renode"))
    path_to_embed = os.path.expanduser(j("~", "src", "stm32-adc"))
    ctx.run((
        f"docker run -d"
        f" -v {path_to_renode}:/home/developer/renode:cached"
        f" -v {path_to_embed}:/home/developer/embed:cached"
        f" -w /home/developer/renode"
        f" -e DISPLAY=host.docker.internal:0"
        f" grifcj/monodevelop monodevelop Renode.sln"))

@task
def runtests(ctx):
    path_to_renode = os.path.expanduser(j("~", "src", "renode"))
    path_to_embed = os.path.expanduser(j("~", "src", "stm32-adc"))
    ctx.run(
        f"docker run"
        f" -v {path_to_renode}:/home/developer/renode"
        f" -v {path_to_embed}:/home/developer/embed"
        f" -w /home/developer/renode"
        f" -e DISPLAY=host.docker.internal:0"
        f" grifcj/renode-dev"
        " ./test.sh --show-log"
        " --robot-framework-remote-server-full-directory /home/developer/renode/output/bin/Debug"
        " /home/developer/embed/adc.robot")

# run renode robot tests using custom monodevelop

# Add all tasks to the namespace
# ns = Collection(clean, build, debugmodel, runtests)
# # Configure every task to act as a shell command
# #   (will print colors, allow interactive CLI)
# # Add our extra configuration file for the project
# config = Config(defaults={"run": {"pty": True}})
# ns.configure(config)
