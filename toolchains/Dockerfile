FROM debian:bullseye

# Obtain the user and the group ID from the command line.
ARG USER_ID
ARG GROUP_ID

# Install the package required to build GCC.
RUN apt-get update && apt-get install -y git gcc g++ make bison flex texinfo wget bzip2 file rsync

# Setup a user with the information provided by the caller, so that the files
# generated won't belong to root.
RUN addgroup --gid $GROUP_ID user
RUN adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID user
USER user

# Prepare the workspace.
WORKDIR /build
ADD build-gcc-ia16.sh /build

# Trigger the build and copy the products.
CMD /build/build-gcc-ia16.sh && rsync -a build-ia16/prefix/* /out

