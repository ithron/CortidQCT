FROM nvidia/cuda:10.2-devel

# Base packages
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && \
  apt-get install -y \
  ca-certificates \
  lsb-release \
  libasound2 \
  libatk1.0-0 \
  libc6 \
  libcairo2 \
  libcap2 \
  libcomerr2 \
  libcups2 \
  libdbus-1-3 \
  libfontconfig1 \
  libgconf-2-4 \
  libgcrypt20 \
  libgdk-pixbuf2.0-0 \
  libgssapi-krb5-2 \
  libgstreamer-plugins-base1.0-0 \
  libgstreamer1.0-0 \
  libgtk2.0-0 \
  libk5crypto3 \
  libkrb5-3 \
  libnspr4 \
  libnspr4-dbg \
  libnss3 \
  libpam0g \
  libpango-1.0-0 \
  libpangocairo-1.0-0 \
  libpangoft2-1.0-0 \
  libselinux1 \
  libsm6 \
  libsndfile1 \
  libudev1 \
  libx11-6 \
  libx11-xcb1 \
  libxcb1 \
  libxcomposite1 \
  libxcursor1 \
  libxdamage1 \
  libxext6 \
  libxfixes3 \
  libxft2 \
  libxi6 \
  libxmu6 \
  libxrandr2 \
  libxrender1 \
  libxslt1.1 \
  libxss1 \
  libxt6 \
  libxtst6 \
  libxxf86vm1 \
  procps \
  xkb-data \
  xvfb \
  x11vnc \
  xvfb \
  sudo \
  zlib1g \
  curl \
  cmake \
  unzip

RUN curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | bash && \
  apt-get install -y git-lfs && \
  git lfs install

# MATLAB runtime
ENV MCR_URL "https://ssd.mathworks.com/supportfiles/downloads/R2018b/deployment_files/R2018b/installers/glnxa64/MCR_R2018b_glnxa64_installer.zip"
RUN curl ${MCR_URL} > MCR.zip && \
  unzip MCR.zip -d MCR-install && \
  cd MCR-install && \
  ./install -mode silent -agreeToLicense yes && \
  cd .. && rm -rf MCR-install MCR.zip

RUN git clone https://github.com/ithron/CortidQCT.git /usr/local/src/CortidQCT && \
  cd /usr/local/src/CortidQCT && \
  git checkout docker && \
  git submodule update --init
  
RUN cd /tmp && mkdir CortidQCT-build && cd CortidQCT-build && \
  cmake -DCMAKE_BUILD_TYPE=Release -DWITH_OPENCL=OFF -DMatlab_ROOT_DIR=/usr/local/MATLAB/MATLAB_Runtime/v95 /usr/local/src/CortidQCT && \
  make -kj16 && \
  make test && \
  make install && \
  cd && rm -rf /tmp/CortidQCT-build

