# syntax=docker/dockerfile:1
FROM debian:bookworm-slim AS base

################################################################################
FROM base AS build
RUN apt-get update && apt-get install -y g++ make
ADD . /opt/ft_irc
RUN make -C /opt/ft_irc && chmod +x /opt/ft_irc/ircserv

################################################################################
FROM base AS final
RUN apt-get update && apt-get install -y netcat-openbsd
ARG UID=10001
RUN adduser \
    --disabled-password \
    --gecos "" \
    --home "/nonexistent" \
    --shell "/sbin/nologin" \
    --no-create-home \
    --uid "${UID}" \
    appuser
USER appuser
COPY --from=build /opt/ft_irc/ircserv /bin/ircserv
EXPOSE 4242
ENTRYPOINT [ "/bin/ircserv", "4242" ]
