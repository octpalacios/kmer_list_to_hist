# Copyright (C) 2022 Roberto Rossini <roberros@uio.no>
#
# SPDX-License-Identifier: MIT


FROM ubuntu:20.04 as builder

RUN apt-get update \
&&  apt-get install -y cmake g++

COPY . /tmp/src

RUN mkdir /tmp/build \
&&  cd /tmp/build \
&&  cmake /tmp/src \
&&  cmake --build /tmp/build -j $(nproc)

FROM ubuntu:20.04 as base

COPY --from=builder /tmp/build/kmer_list_to_hist /usr/local/bin/kmer_list_to_hist
COPY --from=builder /tmp/src/LICENSE /usr/local/share/licenses/kmer_list_to_hist/

WORKDIR /data
ENTRYPOINT ["/usr/local/bin/kmer_list_to_hist"]

RUN kmer_list_to_hist --help

# https://github.com/opencontainers/image-spec/blob/main/annotations.md#pre-defined-annotation-keys
LABEL org.opencontainers.image.authors='Roberto Rossini <roberros@uio.no>'
LABEL org.opencontainers.image.url='https://github.com/octpalacios/kmer_list_to_hist'
LABEL org.opencontainers.image.documentation='https://github.com/octpalacios/kmer_list_to_hist'
LABEL org.opencontainers.image.source='https://github.com/octpalacios/kmer_list_to_hist'
LABEL org.opencontainers.image.licenses='MIT'
LABEL org.opencontainers.image.title='kmer_list_to_hist'
LABEL org.opencontainers.image.base.name='ubuntu'
