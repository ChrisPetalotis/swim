FROM gabrielmoreno/omnetpp-vnc:5.4.1
LABEL Description="Docker image for SWIM"

USER root
RUN apt update && apt install -y --no-install-recommends libboost-all-dev git r-base r-cran-reshape r-cran-ggplot2 r-cran-rsqlite evince ristretto nano curl python3-dev 

RUN apt update -y && apt upgrade -y && \
    apt-get install -y wget build-essential checkinstall  libreadline-gplv2-dev  libncursesw5-dev  libssl-dev  libsqlite3-dev tk-dev libgdbm-dev libc6-dev libbz2-dev libffi-dev zlib1g-dev && \
    cd /usr/src && \
    wget https://www.python.org/ftp/python/3.10.9/Python-3.10.9.tgz && \
    tar xzf Python-3.10.9.tgz && \
    cd Python-3.10.9 && \
    ./configure --enable-optimizations && \
    make altinstall

RUN curl http://bootstrap.pypa.io/get-pip.py | python3.10

RUN apt-get -y install python3-pip

RUN apt-get install -y --no-install-recommends ca-certificates

RUN update-ca-certificates

ENV BASE_DIR=${HOME}/seams-swim


WORKDIR ${BASE_DIR}

RUN git clone https://github.com/ChrisPetalotis/swim.git


RUN python3.10 -m pip install --trusted-host pypi.org --trusted-host pypi.python.org --trusted-host files.pythonhosted.org -r swim/predictions/requirements.txt

RUN tar xzf swim/queueinglib.tgz

WORKDIR ${BASE_DIR}/queueinglib

RUN make -j$(nproc)

WORKDIR ${BASE_DIR}/swim

RUN make cleanall && make makefiles && make -j$(nproc)

WORKDIR ${BASE_DIR}/swim/examples/simple_am

RUN make -j$(nproc)

RUN R -e 'install.packages("https://github.com/wilkelab/cowplot/archive/0.6.3.tar.gz", repos=NULL)'

# configure OMNeT++ IDE
WORKDIR ${BASE_DIR}
RUN mkdir -p workspace; $(dirname `which omnetpp`)/../ide/omnetpp -nosplash -data workspace -application org.eclipse.cdt.managedbuilder.core.headlessbuild -importAll .

WORKDIR ${HOME}
RUN mkdir -p .local/share/applications/
COPY omnetpp.desktop .local/share/applications/


