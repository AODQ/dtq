FROM gitpod/workspace-full-vnc
                    
USER gitpod

# Install custom tools, runtime, etc. using apt-get
# For example, the command below would install "bastet" - a command line tetris clone:
#
# RUN sudo apt-get -q update && #     sudo apt-get install -yq bastet && #     sudo rm -rf /var/lib/apt/lists/*
#
# More information: https://www.gitgitpodpod.io/docs/config-docker/

RUN sudo apt-get -q update && sudo apt-get install -yq libvulkan1 mesa-vulkan-drivers libvulkan-dev vulkan-utils libglfw3 libglfw3-dev ninja-build
