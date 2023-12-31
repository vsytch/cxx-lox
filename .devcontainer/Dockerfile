ARG UBUNTU_VERSION
FROM mcr.microsoft.com/devcontainers/base:ubuntu-${UBUNTU_VERSION}

USER root

# Prepare installation environment.
RUN export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get -y install \
        lsb-release \
        software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test && \
    apt-get update

# Install build environment.
RUN export DEBIAN_FRONTEND=noninteractive && \
    apt-get -y install \
        build-essential \
        wget \
        gnupg \
        pkg-config \
        ninja-build \
        autoconf \
        autoconf-archive

# Debugging tools.
RUN export DEBIAN_FRONTEND=noninteractive && \
    apt-get -y install \
        cppcheck \
        gdb \
        valgrind \
        iwyu

# TAB completion.
RUN export DEBIAN_FRONTEND=noninteractive && \
    apt-get -y install \
        bash-completion

# Install CMake.
ARG CMAKE_VERSION
ADD https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh /tmp/
RUN chmod +x /tmp/cmake-${CMAKE_VERSION}-linux-x86_64.sh && \
    /tmp/cmake-${CMAKE_VERSION}-linux-x86_64.sh --prefix=/usr/ --skip-license

# Install libstdc++.
ARG GCC_VERSION
RUN export DEBIAN_FRONTEND=noninteractive && \
    apt-get -y install gcc-${GCC_VERSION} g++-${GCC_VERSION} && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-${GCC_VERSION} ${GCC_VERSION} && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-${GCC_VERSION} ${GCC_VERSION}

# Install clang.
ARG CLANG_VERSION
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
ADD https://apt.llvm.org/llvm.sh /tmp/
RUN chmod +x /tmp/llvm.sh && \
    export DEBIAN_FRONTEND=noninteractive && \
    /tmp/llvm.sh ${CLANG_VERSION} && \
    apt-get -y install clang && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${CLANG_VERSION} ${CLANG_VERSION} && \
    update-alternatives --set cc /usr/bin/clang && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-${CLANG_VERSION} ${CLANG_VERSION} && \
    update-alternatives --set c++ /usr/bin/clang++ && \
    apt-get -y install clang-tidy-${CLANG_VERSION} && \
    update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-${CLANG_VERSION} ${CLANG_VERSION} && \
    apt-get -y install clang-format-${CLANG_VERSION} && \
    update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-${CLANG_VERSION} ${CLANG_VERSION} && \
    update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-${CLANG_VERSION} ${CLANG_VERSION}

# Install mold.
ARG MOLD_VERSION
ARG MOLD_MINOR_VERSION
ARG MOLD_PATCH_VERSION
ADD https://github.com/rui314/mold/releases/download/v${MOLD_VERSION}/mold-${MOLD_VERSION}-x86_64-linux.tar.gz /tmp/
RUN tar -zxvf /tmp/mold-${MOLD_VERSION}-x86_64-linux.tar.gz -C /usr/ --strip-components=1 && \
    update-alternatives --install /usr/bin/ld ld /usr/bin/ld.mold 888

# Install python.
ARG PY_MAJOR_VERSION
ARG PY_MINOR_VERSION
RUN export DEBIAN_FRONTEND=noninteractive && \
    apt-get -y install --no-install-recommends \
        zlib1g-dev \
        libssl-dev
ADD https://www.python.org/ftp/python/${PY_MAJOR_VERSION}.${PY_MINOR_VERSION}/Python-${PY_MAJOR_VERSION}.${PY_MINOR_VERSION}.tgz /tmp/
RUN tar -zxvf /tmp/Python-${PY_MAJOR_VERSION}.${PY_MINOR_VERSION}.tgz -C /tmp/
WORKDIR /tmp/Python-${PY_MAJOR_VERSION}.${PY_MINOR_VERSION}/
RUN ./configure --prefix=/usr/ --enable-optimizations && \
    make -j && \
    make -j install
WORKDIR /tmp/

# Install vcpkg.
ARG VCPKG_VERSION
RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg-${VCPKG_VERSION}/ && \
    git -C /opt/vcpkg-${VCPKG_VERSION}/ reset --hard tags/${VCPKG_VERSION} && \
    /opt/vcpkg-${VCPKG_VERSION}/bootstrap-vcpkg.sh && \
    ln -s /opt/vcpkg-${VCPKG_VERSION}/vcpkg /usr/bin/vcpkg && \
    vcpkg integrate zsh && \
    vcpkg install --clean-after-build \
        boost \
        fmt \
        magic-enum \
        range-v3 && \
    chown -R vscode: /opt/vcpkg-${VCPKG_VERSION}/
ENV VCPKG_ROOT="/opt/vcpkg-${VCPKG_VERSION}"

# https://wg21.link/p2168
ADD https://raw.githubusercontent.com/lewissbaker/generator/main/include/__generator.hpp /usr/include/c++/${GCC_VERSION}/generator
RUN chown vscode: /usr/include/c++/${GCC_VERSION}/generator

# http://wg21.link/p2300
RUN git clone https://github.com/NVIDIA/stdexec.git /usr/src/stdexec && \
    git -C /usr/src/stdexec checkout ${STDEXEC_VERSION} && \
    cmake -S /usr/src/stdexec/ -B /tmp/stdexec/build/ && \
    cmake --build /tmp/stdexec/build/ --target install

COPY ./scripts/dump_cpp_env.sh /usr/bin/

# Cleanup install artifacts.
RUN rm -rf /tmp/*

LABEL org.opencontainers.image.source=https://github.com/vsytch/cxx-init
