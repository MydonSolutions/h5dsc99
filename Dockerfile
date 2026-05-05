FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get --fix-missing update -y && apt-get install -y \
    git build-essential cmake zlib1g-dev \
    linux-tools-generic \
    pkg-config \
    python3-pip \
&& python3 -m pip install meson ninja

WORKDIR /work

RUN git clone --branch hdf5_1.14.6 --depth 1 https://github.com/HDFGroup/hdf5/ \
&& mkdir /work/hdf5_build \
&& cd /work/hdf5_build \
&& cmake -S /work/hdf5 -B /work/hdf5_build/ \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_INSTALL_PREFIX=/usr/local \
 -DHDF5_BUILD_CPP_LIB=ON \
 -DHDF5_BUILD_TOOLS=ON \
 -DBUILD_TESTING=OFF \
&& cmake --build /work/hdf5_build/ \
&& cmake --install /work/hdf5_build/

COPY . /work/h5dsc99

RUN cd /work/h5dsc99 \
&& meson setup /work/h5dsc99_build \
&& cd /work/h5dsc99_build \
&& ninja install