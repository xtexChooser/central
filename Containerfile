FROM docker.io/library/alpine
ARG TARGETDIR
RUN apk add libgcc
COPY target/${TARGETDIR}/release/wezen-broker /usr/local/bin
COPY target/${TARGETDIR}/release/wezen-bridge /usr/local/bin
COPY target/${TARGETDIR}/release/wezen-exit /usr/local/bin
COPY target/${TARGETDIR}/release/wezen-client /usr/local/bin
COPY target/${TARGETDIR}/release/wezen-client-gui /usr/local/bin
