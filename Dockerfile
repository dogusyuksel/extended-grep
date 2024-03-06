FROM ubuntu:22.04
ARG DEBIAN_FRONTEND=noninteractive

USER root

RUN apt-get update && \
    apt-get -y upgrade && \
    apt-get -y --no-install-recommends install \
        build-essential \
        clang-format \
        valgrind \
        gdb \
        ruby \
        git \
        git-core \
        git-lfs \
        python3-dbg \
        python3-dev \
        python3-pip \
        vim \
        language-pack-en-base \
        splint && \
    apt-get -y clean

RUN gem install ceedling

RUN git config --global --add safe.directory /workspace

RUN ln -s /usr/bin/python3 /usr/bin/python
RUN pip install matplotlib
RUN pip install pytest
RUN python3 -m pip install flake8

RUN cd /
RUN git clone git://git.openembedded.org/bitbake
ENV PATH="${PATH}:/bitbake/bin"
ENV PYTHONPATH="${PYTHONPATH}:/bitbake/lib"
RUN pip install -r bitbake/toaster-requirements.txt

CMD ["/bin/bash"]

WORKDIR /workspace/
