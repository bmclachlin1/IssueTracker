# Base cpsc image, based on Ubuntu
FROM everettblakley/cpsc-uleth:software-eng

# Copy local files over to container
COPY . /usr/local/src/jalapenos

# Set the working directory to the source directory
WORKDIR /usr/local/src/jalapenos

# Expost port 8080 to be able to communicate with host
EXPOSE 8080

# Create a symbolic link for g++9.1
RUN ln -s /usr/bin/g++ /usr/bin/g++9.1







