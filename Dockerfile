FROM ubuntu:18.04

#########
# install wget
#########
RUN apt-get update >/dev/null && apt-get install -y wget

#########
# put apps in /apps
#########
WORKDIR /apps

#########
# install nifti2dicom
#########

ARG NIFTI2DICOM_LINUX_VERSION=v0.4.20210412

RUN wget -q https://github.com/ylugithub/nifti2dicom/releases/download/${NIFTI2DICOM_LINUX_VERSION}/nifti2dicom_ubuntu18.04 && \
    mv nifti2dicom_ubuntu18.04 nifti2dicom && \
    chmod a+x nifti2dicom

ENV PATH=/apps:$PATH

ENTRYPOINT ["/apps/nifti2dicom"]
CMD ["-h"]
